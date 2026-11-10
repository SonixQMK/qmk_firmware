// Copyright 2026 mgrenonville
// Indicator/profile handling originally based on the Keychron K4 V2 Bluetooth
// integration by reubenjs (SonixQMK PR #486).
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Keychron K3 v2 board code: optical matrix settle, Bluetooth (ITON module via
// k3_bt.c, behaviour modelled on the stock V1.10 firmware), indicators.

#include "quantum.h"
#include "connection.h"
#include "k3_bt.h"

#ifdef CONSOLE_ENABLE
#    include "print.h"
#    include "debug.h"
#endif

#define BT_PAIR_HOLD_MS      3000
#define BT_PAIR_REPEAT_MS    100
#define BT_PAIR_SENDS        3
#define BT_LED_CONNECTED_MS  2500
#define BT_BATT_SHOW_MS      5000
#define BT_BATT_FILL_STEP_MS 120

// Cable/BT selector: DIP index 0 on D5, high = Bluetooth.
#define K3V2_CONN_SWITCH_PIN D5

// Optical switches need time to settle after a row is selected: with the
// default (a few cycles) the first columns read mid-transition, which shows up
// as heavy chatter on columns 0-2 when running from the battery (BT mode).
// Runs inside the shared-matrix PWM ISR, so busy-wait (~8 cycles/iteration).
#ifndef K3V2_MATRIX_SELECT_DELAY_US
#    define K3V2_MATRIX_SELECT_DELAY_US 20
#endif
void matrix_output_select_delay(void) {
    for (volatile uint32_t i = 0; i < (K3V2_MATRIX_SELECT_DELAY_US * (CPU_CLOCK / 8000000UL)); i++) {
    }
}

static bool     bt_ready        = false;
static bool     bt_mode         = false;
static uint8_t  bt_profile      = 0;
static uint16_t bt_held_keycode = 0;
static uint16_t bt_held_timer   = 0;
static uint8_t  bt_pair_sends   = 0;
static uint16_t bt_pair_timer   = 0;
static uint8_t  bt_led_state    = 0; // 0 idle, 1 pairing flash, 2 connected hold
static uint16_t bt_led_timer    = 0;
static uint8_t  bt_batt_level   = 0;
static bool     bt_batt_show    = false;
static uint16_t bt_batt_timer   = 0;

// ---- Idle sleep (BT mode), timings from the stock firmware -------------------
// Stock counts 100 ms ticks: 400 (40 s) while not connected, 6000 (10 min)
// while connected, reset by any key, paused while pairing. The module can
// also request sleep (B6 5A 08). Here "sleep" turns the LEDs off (the main
// power draw); the matrix keeps scanning so any key wakes the board.
#ifndef K3V2_IDLE_CONNECTED_MS
#    define K3V2_IDLE_CONNECTED_MS (10UL * 60UL * 1000UL)
#endif
#ifndef K3V2_IDLE_DISCONNECTED_MS
#    define K3V2_IDLE_DISCONNECTED_MS (40UL * 1000UL)
#endif
#define K3V2_CAPS_LED_PIN D4

static bool k3_sleeping      = false;
static bool k3_sleep_request = false;

static void k3_sleep_enter(void) {
    k3_sleeping      = true;
    k3_sleep_request = false;
    rgb_matrix_set_suspend_state(true);
    gpio_write_pin_low(K3V2_CAPS_LED_PIN);
#ifdef CONSOLE_ENABLE
    uprintf("[k3v2] t=%lu SLEEP (conn=%d)\n", (unsigned long)timer_read32(), k3bt_connected);
#endif
}

static void k3_sleep_exit(void) {
    k3_sleeping = false;
    rgb_matrix_set_suspend_state(false);
    led_update_kb(host_keyboard_led_state());
#ifdef CONSOLE_ENABLE
    uprintf("[k3v2] t=%lu WAKE\n", (unsigned long)timer_read32());
#endif
}

bool led_update_kb(led_t led_state) {
    if (k3_sleeping) {
        gpio_write_pin_low(K3V2_CAPS_LED_PIN);
        return false;
    }
    bool res = led_update_user(led_state);
    if (res) led_update_ports(led_state); // drives the caps lock LED (D4)
    return res;
}

static void k3_sleep_task(bool pairing) {
    if (k3_sleeping) {
        if (!bt_mode || last_input_activity_elapsed() < 50) k3_sleep_exit();
        return;
    }
    if (!bt_mode || pairing) return;
    uint32_t limit = k3bt_connected ? K3V2_IDLE_CONNECTED_MS : K3V2_IDLE_DISCONNECTED_MS;
    if (k3_sleep_request || last_input_activity_elapsed() > limit) k3_sleep_enter();
}

static inline uint8_t number_row_led(uint8_t n) {
    return g_led_config.matrix_co[1][1 + n];
}

static void k3v2_apply_conn_mode(bool cable_active) {
    bt_mode = !cable_active;
    if (bt_mode) {
        connection_set_host_noeeprom(CONNECTION_HOST_BLUETOOTH);
        k3bt_control(K3BT_GROUP_BT, K3BT_SWITCH_PROFILE + bt_profile);
    } else {
        k3bt_control(K3BT_GROUP_USB, K3BT_USB_MODE);
        connection_set_host_noeeprom(CONNECTION_HOST_USB);
    }
#ifdef CONSOLE_ENABLE
    uprintf("[k3v2] conn mode cable=%d bt_mode=%d\n", cable_active, bt_mode);
#endif
}

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0 && bt_ready) {
        k3v2_apply_conn_mode(active);
    }
    return true;
}

void keyboard_post_init_kb(void) {
    bt_ready = true;
    // Like stock: select the host from the slider, send nothing to the module.
    bt_mode = gpio_read_pin(K3V2_CONN_SWITCH_PIN);
    connection_set_host_noeeprom(bt_mode ? CONNECTION_HOST_BLUETOOTH : CONNECTION_HOST_USB);
#ifdef CONSOLE_ENABLE
    debug_enable = true;
    uprintf("[k3v2] post_init bt_mode=%d\n", bt_mode);
#endif
    keyboard_post_init_user();
}

void k3bt_on_notification(uint8_t group, uint8_t code) {
#ifdef CONSOLE_ENABLE
    uprintf("[k3v2] t=%lu NOTIF %02X %02X\n", (unsigned long)timer_read32(), group, code);
#endif
    if (group == K3BT_NOTIF_BT) {
        switch (code) {
            case K3BT_BT_PAIRING:
                bt_led_state  = 1;
                bt_led_timer  = timer_read();
                bt_pair_sends = 0;
                break;
            case K3BT_BT_CONNECTED:
                bt_led_state  = 2;
                bt_led_timer  = timer_read();
                bt_pair_sends = 0;
                break;
            case K3BT_BT_DISCONNECTED:
                bt_led_state = 0;
                break;
        }
    } else if (group == K3BT_NOTIF_BATTERY) {
        if (code == K3BT_BATT_ABOVE_70 || code == K3BT_BATT_30_70 || code == K3BT_BATT_BELOW_30) bt_batt_level = code;
        if (code == K3BT_BATT_LOW || code == K3BT_BATT_SHUTDOWN) bt_batt_level = K3BT_BATT_BELOW_30;
        if (code == K3BT_BATT_SLEEP_REQ) k3_sleep_request = true; // module asks the MCU to sleep
    }
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }


    if (keycode == KC_BT_BAT) {
        if (record->event.pressed && bt_mode) {
            bt_batt_show  = true;
            bt_batt_timer = timer_read();
            k3bt_control(K3BT_GROUP_BT, K3BT_QUERY_BATTERY);
        }
        return false;
    }

    switch (keycode) {
        case BT_PRF1:
        case BT_PRF2:
        case BT_PRF3:
            if (!bt_mode) return false;
            if (record->event.pressed) {
                bt_profile      = keycode - BT_PRF1;
                bt_held_keycode = keycode;
                bt_held_timer   = timer_read();
                bt_pair_sends   = 0;
                k3bt_control(K3BT_GROUP_BT, K3BT_SWITCH_PROFILE + bt_profile);
            } else {
                bt_held_keycode = 0;
            }
            return false;
    }
    return true;
}

void housekeeping_task_kb(void) {
    // Hold a profile key: like stock, send enter_pairing a few times.
    if (bt_mode && bt_held_keycode && timer_elapsed(bt_held_timer) >= BT_PAIR_HOLD_MS) {
        bt_held_keycode = 0;
        bt_pair_sends   = BT_PAIR_SENDS;
        bt_pair_timer   = timer_read() - BT_PAIR_REPEAT_MS;
    }
    if (bt_pair_sends && timer_elapsed(bt_pair_timer) >= BT_PAIR_REPEAT_MS) {
        bt_pair_timer = timer_read();
        bt_pair_sends--;
        k3bt_control(K3BT_GROUP_BT, K3BT_ENTER_PAIRING);
    }

    // Pairing in progress: profile key held, enter_pairing pending, or module pairing.
    k3_sleep_task(bt_held_keycode || bt_pair_sends || bt_led_state == 1);

}

// Pairing: blank the matrix and flash the profile key blue (only after the
// module confirms), hold it solid on connect. Fn+B: battery gauge on 1..0.
bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_user()) {
        return false;
    }
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
            rgb_matrix_set_color(number_row_led(bt_profile), 0, 0, 255);
        }
        return true;
    }
    if (bt_batt_show) {
        uint16_t elapsed = timer_elapsed(bt_batt_timer);
        if (elapsed < BT_BATT_SHOW_MS) {
            uint8_t leds = 10, r = 0, g = 0, b = 0;
            switch (bt_batt_level) {
                case K3BT_BATT_BELOW_30: leds = 3; r = 180; break;
                case K3BT_BATT_30_70:    leds = 7; r = 180; g = 72; break;
                case K3BT_BATT_ABOVE_70: g = 180; break;
                default:                 b = 80; break; // unknown
            }
            uint8_t lit = elapsed / BT_BATT_FILL_STEP_MS + 1;
            rgb_matrix_set_color_all(1, 1, 1);
            for (uint8_t i = 0; i < 10; i++) {
                bool on = i < leds && i < lit;
                rgb_matrix_set_color(number_row_led(i), on ? r : 0, on ? g : 0, on ? b : 0);
            }
            return true;
        }
        bt_batt_show = false;
    }
    if (bt_batt_level == K3BT_BATT_BELOW_30 && (timer_read() % 4000) < 600) {
        rgb_matrix_set_color(number_row_led(bt_profile), 255, 0, 0);
    }
    return true;
}
