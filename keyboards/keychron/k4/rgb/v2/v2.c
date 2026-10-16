// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "gpio.h"
#include "timer.h"
#include "iton_bt.h"
#include "connection.h"

#ifdef CONSOLE_ENABLE
#    include "print.h"
#    include "debug.h"
#endif

#define BT_PROFILE_RESET_HOLD_MS 3000
#define BT_PAIRING_REPEAT_MS     100
// Number-row LEDs used for profile pairing status: 1/2/3 are indices 20-22.
#define BT_LED_BASE_INDEX        20
#define BT_LED_CONNECTED_MS      2500
#define BT_BATT_QUERY_MS         60000
#define BT_BATT_SHOW_MS          5000
#define BT_BATT_FILL_STEP_MS     120
#define BT_BATT_FLASH_STEP_MS    120
#define BT_BATT_FLASH_COUNT      3
#define FN_INDICATOR_RED         0
#define FN_INDICATOR_GREEN       64
#define FN_INDICATOR_BLUE        128

// dip_switch_init() (and the first dip_switch_update_kb() calls it triggers)
// runs before bluetooth_init() in quantum/keyboard.c, so the initial mode
// must not talk to the BT module until after iton_bt_init() has run.
static bool     bt_ready         = false;
static bool     bt_mode          = false;
static uint8_t  bt_profile       = 0;
static uint16_t bt_held_keycode  = 0;
static uint16_t bt_held_timer    = 0;
static uint16_t bt_pairing_timer = 0;
static bool     bt_mode_sent     = false;
static uint16_t bt_mode_timer    = 0;
static uint8_t  bt_led_state     = 0; // 0 idle, 1 pairing flash, 2 connected hold
static uint16_t bt_led_timer     = 0;
static uint8_t  bt_batt_level    = 0;
static uint16_t bt_batt_timer    = 0;
static bool     bt_batt_due      = false;
static uint16_t bt_batt_show_timer = 0;
static bool     bt_batt_show       = false;
static bool     bt_batt_dbg_pending = false;
static bool     mac_mode           = false;
static uint16_t mac_fkeys          = 0;
static bool     mac_fn_rcmd        = false;
static bool     fn_indicator_cached = false;
static uint8_t  fn_indicator_map[(MATRIX_ROWS * MATRIX_COLS + 7) / 8];

static const uint16_t PROGMEM mac_fn_defaults[] = {
    KC_SLCT, KC_PAUS, KC_APP,   KC_F5,   RM_VALD, RM_VALU,
    KC_MPRV, KC_MPLY, KC_MNXT,  KC_MUTE, KC_VOLD, KC_VOLU,
};

// DIP index 0 = cable/BT selector (active = cable), index 1 = win/mac (active = mac).
static void k4v2_apply_dip_mode(bool cable_active) {
    bt_mode = !cable_active;
    if (bt_mode) {
        iton_bt_switch_profile(bt_profile);
        iton_bt_enable_sleep();
        bt_batt_due = true;
        connection_set_host_noeeprom(CONNECTION_HOST_BLUETOOTH);
    } else {
        iton_bt_mode_usb();
        connection_set_host_noeeprom(CONNECTION_HOST_USB);
    }
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] apply_dip_mode cable_active=%d bt_mode=%d host=%d\n", cable_active, bt_mode, connection_get_host());
#endif
}

static void k4v2_apply_os_mode(bool mac_active) {
    mac_mode = mac_active;
    if (mac_fkeys || mac_fn_rcmd) {
        clear_keyboard();
        mac_fkeys   = 0;
        mac_fn_rcmd = false;
    }
    layer_clear();
    default_layer_set((layer_state_t)1 << (mac_mode ? K4V2_LAYER_MAC_BASE : K4V2_LAYER_WIN_BASE));
}

bool dip_switch_update_kb(uint8_t index, bool active) {
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] dip_switch_update_kb index=%d active=%d bt_ready=%d\n", index, active, bt_ready);
#endif
    if (!dip_switch_update_user(index, active)) {
        return false;
    }

    if (index == 0 && bt_ready) {
        k4v2_apply_dip_mode(active);
    } else if (index == 1) {
        k4v2_apply_os_mode(active);
    }

    return true;
}

void keyboard_post_init_kb(void) {
    bt_ready = true;
#ifdef CONSOLE_ENABLE
    debug_enable = true;
    uprintf("[k4v2] post_init D5=%d D6=%d mac=%d\n", (int)readPin(D5), (int)readPin(D6), mac_mode);
#endif
    // Set the host immediately but defer the iton mode command; stock delays
    // it after boot rather than talking to the module right away.
    bt_mode = readPin(D5);
    connection_set_host_noeeprom(bt_mode ? CONNECTION_HOST_BLUETOOTH : CONNECTION_HOST_USB);
    bt_mode_timer = timer_read();
#ifdef CONSOLE_ENABLE
    uprintf("[k4v2] post_init bt_mode=%d host=%d (mode cmd deferred)\n", bt_mode, connection_get_host());
#endif
    keyboard_post_init_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    if (mac_mode) {
        if (keycode == MO(K4V2_LAYER_FN)) {
            if (record->event.pressed) {
                default_layer_set(0);
                layer_on(K4V2_LAYER_FN);
            } else {
                layer_off(K4V2_LAYER_FN);
                default_layer_set((layer_state_t)1 << K4V2_LAYER_MAC_BASE);
            }
            return false;
        }

        if (record->event.key.row == 0 && record->event.key.col >= 1 && record->event.key.col <= 12) {
            uint8_t  index    = record->event.key.col - 1;
            uint16_t expected = pgm_read_word(&mac_fn_defaults[index]);
            if (record->event.pressed && layer_state_is(K4V2_LAYER_FN) && keycode == expected) {
                mac_fkeys |= (uint16_t)1 << index;
                register_code(KC_F1 + index);
                return false;
            }
            if (!record->event.pressed && (mac_fkeys & ((uint16_t)1 << index))) {
                mac_fkeys &= ~(uint16_t)1 << index;
                unregister_code(KC_F1 + index);
                return false;
            }
        }

        if (record->event.key.row == 5 && record->event.key.col == 10 && keycode == KC_RALT) {
            if (record->event.pressed && layer_state_is(K4V2_LAYER_FN)) {
                mac_fn_rcmd = true;
                register_code(KC_RGUI);
                return false;
            }
            if (!record->event.pressed && mac_fn_rcmd) {
                mac_fn_rcmd = false;
                unregister_code(KC_RGUI);
                return false;
            }
        }
    }

    if (keycode == KC_BT_BAT) {
        if (record->event.pressed && bt_mode) {
            bt_batt_show_timer = timer_read();
            bt_batt_show       = true;
            iton_bt_query_battery_level();
        }
        return false;
    }

    if (!bt_mode) {
#ifdef CONSOLE_ENABLE
        switch (keycode) {
            case BT_PRF1:
            case BT_PRF2:
            case BT_PRF3:
                uprintf("[k4v2] BT_PRF %s ignored, bt_mode=%d\n", record->event.pressed ? "press" : "release", bt_mode);
                break;
        }
#endif
        return true;
    }

    switch (keycode) {
        case BT_PRF1:
        case BT_PRF2:
        case BT_PRF3:
            if (record->event.pressed) {
                bt_profile       = keycode - BT_PRF1;
                bt_held_keycode  = keycode;
                bt_held_timer    = timer_read();
                bt_pairing_timer = 0;
                iton_bt_switch_profile(bt_profile);
#ifdef CONSOLE_ENABLE
                uprintf("[k4v2] BT_PRF%d press -> switch_profile(%d)\n", bt_profile + 1, bt_profile);
#endif
            } else {
                bt_held_keycode = 0;
#ifdef CONSOLE_ENABLE
                uprintf("[k4v2] BT_PRF release\n");
#endif
            }
            return false;
    }

    return true;
}

void housekeeping_task_kb(void) {
    // Deferred boot-time mode command (see keyboard_post_init_kb).
    if (!bt_mode_sent && timer_elapsed(bt_mode_timer) >= 1000) {
        bt_mode_sent = true;
        if (bt_mode) {
            iton_bt_switch_profile(bt_profile);
            iton_bt_enable_sleep();
            bt_batt_due = true;
        } else {
            iton_bt_mode_usb();
        }
#ifdef CONSOLE_ENABLE
        uprintf("[k4v2] deferred mode sent bt_mode=%d\n", bt_mode);
#endif
    }

    // Stock re-queues enter_pairing every tick while a profile key is held
    // past its hold timer.
    if (bt_mode && bt_held_keycode && timer_elapsed(bt_held_timer) >= BT_PROFILE_RESET_HOLD_MS && timer_elapsed(bt_pairing_timer) >= BT_PAIRING_REPEAT_MS) {
        bt_pairing_timer = timer_read();
        bt_led_state     = 1;
        bt_led_timer     = timer_read();
        iton_bt_enter_pairing();
#ifdef CONSOLE_ENABLE
        uprintf("[k4v2] enter_pairing sent\n");
#endif
    }

    // Battery level: query shortly after every (re)connect and periodically
    // while connected. Sends are thread-context, so callbacks only flag it.
    if (bt_mode && (bt_batt_due || (iton_bt_is_connected && timer_elapsed(bt_batt_timer) >= BT_BATT_QUERY_MS))) {
        bt_batt_due   = false;
        bt_batt_timer = timer_read();
        iton_bt_query_battery_level();
    }

#ifdef CONSOLE_ENABLE
    if (bt_batt_dbg_pending) {
        bt_batt_dbg_pending = false;
        uprintf("[k4v2] batt level 0x%02X\n", bt_batt_level);
    }
#endif

#ifdef CONSOLE_ENABLE
    static uint16_t dbg_last = 0, dbg_seq = 0, dbg_send = 0;
    uint16_t sends = 0, done_tx = 0, done_rx = 0, timeouts = 0, tx_aborted = 0, rx_seq = 0;
    uint16_t int_hi = 0, int_lo = 0, sent_hi = 0, sent_lo = 0;
    iton_bt_debug_stats(&sends, &done_tx, &done_rx, &timeouts, &tx_aborted, &rx_seq, &int_hi, &int_lo, &sent_hi, &sent_lo);
    if (dbg_seq == 0 && rx_seq > 8) {
        dbg_seq = rx_seq - 8;
    }
    while (dbg_seq < rx_seq) {
        uint8_t f[6];
        dbg_seq++;
        uint8_t len = iton_bt_debug_rx_frame(dbg_seq, f);
        if (len) {
            uprintf("[k4v2] RX #%u len=%u: %02X %02X %02X %02X %02X %02X\n", dbg_seq, len, f[0], f[1], f[2], f[3], f[4], f[5]);
        }
    }
    if (dbg_send == 0 && sends > 8) {
        dbg_send = sends - 8;
    }
    while (dbg_send < sends) {
        uint8_t r[12];
        dbg_send++;
        uint8_t idx = iton_bt_debug_reply(dbg_send, r);
        if (idx) {
            uprintf("[k4v2] reply #%u idx=%u: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", dbg_send, idx, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11]);
        }
    }
    if (timer_elapsed(dbg_last) >= 1000) {
        dbg_last = timer_read();
        uprintf("[k4v2] D5=%d D6=%d bt_mode=%d mac=%d host=%d bt_conn=%d batt=0x%02X sends=%u done_tx=%u done_rx=%u timeouts=%u aborted=%u rx_seq=%u int=%d hi/lo=%u/%u sent hi/lo=%u/%u\n", (int)readPin(D5), (int)readPin(D6), bt_mode, mac_mode, connection_get_host(), iton_bt_is_connected, bt_batt_level, sends, done_tx, done_rx, timeouts, tx_aborted, rx_seq, (int)readPin(A1), int_hi, int_lo, sent_hi, sent_lo);
    }
#endif
}

/**
 * Module notifications. These run inside the INT-line pal callback (ISR
 * context), so they may only set flags — anything that sends a frame must
 * be deferred to housekeeping_task_kb.
 */
void iton_bt_entered_pairing(void) {
    bt_led_state = 1;
    bt_led_timer = timer_read();
}

void iton_bt_enters_connection_state(void) {
    // Keep flashing while the module tries the stored host.
}

void iton_bt_connection_successful(void) {
    bt_led_state = 2;
    bt_led_timer = timer_read();
    bt_batt_due  = true;
}

void iton_bt_disconnected(void) {
    bt_led_state = 0;
}

void iton_bt_battery_level(uint8_t level) {
    bt_batt_level = level;
    // This runs in the EXTI ISR; defer printing to housekeeping since
    // sendchar blocks on the USB console queue.
    bt_batt_dbg_pending = true;
}

void iton_bt_battery_voltage_low(void) {
    bt_batt_level = batt_below_30;
}

void iton_bt_battery_low_power_shutdown(void) {
    bt_batt_level = batt_below_30;
}

void iton_bt_battery_exit_low_battery_mode(void) {
    bt_batt_due = true; // refresh the level at the next housekeeping pass
}

static void k4v2_fn_indicators(void) {
    if (!layer_state_is(K4V2_LAYER_FN)) {
        fn_indicator_cached = false;
        return;
    }
    if (!fn_indicator_cached) {
        for (uint8_t i = 0; i < sizeof(fn_indicator_map); i++) {
            fn_indicator_map[i] = 0;
        }
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                uint16_t keycode = keymap_key_to_keycode(K4V2_LAYER_FN, MAKE_KEYPOS(row, col));
                if (keycode != KC_NO && keycode != KC_TRNS) {
                    uint8_t index = row * MATRIX_COLS + col;
                    fn_indicator_map[index / 8] |= (uint8_t)1 << (index % 8);
                }
            }
        }
        fn_indicator_cached = true;
    }
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t index = row * MATRIX_COLS + col;
            if (!(fn_indicator_map[index / 8] & ((uint8_t)1 << (index % 8)))) continue;
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) {
                rgb_matrix_set_color(led, FN_INDICATOR_RED, FN_INDICATOR_GREEN, FN_INDICATOR_BLUE);
            }
        }
    }
}

// While pairing, blank the matrix and flash the active profile's key blue.
// On connect it holds solid briefly, then control returns to the animation.
bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }
    k4v2_fn_indicators();
    if (!bt_mode) {
        return true;
    }
    if (bt_led_state) {
        if (bt_led_state == 2 && timer_elapsed(bt_led_timer) >= BT_LED_CONNECTED_MS) {
            bt_led_state = 0;
            return true;
        }
        rgb_matrix_set_color_all(0, 0, 0);
        if (bt_led_state == 2 || (timer_elapsed(bt_led_timer) % 500) < 250) {
            rgb_matrix_set_color(BT_LED_BASE_INDEX + bt_profile, 0, 0, 255);
        }
        return true;
    }
    if (bt_batt_show) {
        uint16_t elapsed = timer_elapsed(bt_batt_show_timer);
        if (elapsed < BT_BATT_SHOW_MS) {
            uint8_t leds = 10, flash_start = 0, red = 0, green = 0, blue = 0;
            switch (bt_batt_level) {
                case batt_below_30:
                    leds = 3;
                    red  = 180;
                    break;
                case batt_between_30_70:
                    leds        = 7;
                    flash_start = 3;
                    red         = 180;
                    green       = 72;
                    break;
                case batt_above_70:
                    flash_start = 6;
                    green       = 180;
                    break;
                default:
                    blue = 80;
                    break;
            }
            uint8_t  lit_leds       = elapsed / BT_BATT_FILL_STEP_MS + 1;
            uint16_t flash_start_ms = leds * BT_BATT_FILL_STEP_MS;
            bool     flash_off      = elapsed >= flash_start_ms && elapsed < flash_start_ms + 2 * BT_BATT_FLASH_COUNT * BT_BATT_FLASH_STEP_MS && ((elapsed - flash_start_ms) / BT_BATT_FLASH_STEP_MS) % 2 == 0;
            rgb_matrix_set_color_all(1, 1, 1);
            for (uint8_t i = 0; i < 10; i++) {
                bool lit = i < leds && i < lit_leds && !(flash_off && i >= flash_start);
                rgb_matrix_set_color(BT_LED_BASE_INDEX + i, lit ? red : 0, lit ? green : 0, lit ? blue : 0);
            }
            return true;
        }
        bt_batt_show = false;
    }
    // Persistent reminder while the battery is low: the profile key blinks
    // red for ~0.6s every 4s without disturbing the rest of the animation.
    if (bt_batt_level == batt_below_30 && (timer_read() % 4000) < 600) {
        rgb_matrix_set_color(BT_LED_BASE_INDEX + bt_profile, 255, 0, 0);
    }
    return true;
}
