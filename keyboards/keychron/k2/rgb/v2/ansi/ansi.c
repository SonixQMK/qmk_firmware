/*
Copyright 2024 mintyleaf

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "quantum.h"
#include "keymap.h"
#include "gpio.h"
#include "wait.h"

#define BT_PROFILE_LED 16
#define BT_BATTERY_LED 49

#ifdef BLUETOOTH_ENABLE
#    include "iton_bt.h"
#    include "outputselect.h"

static uint32_t last_update_time = 0;

static bool     ev_connecting    = false;
static bool     ev_pairing       = false;
static uint16_t ev_disconnected  = 0;
static uint16_t ev_connected     = 0;
static uint16_t ev_battery_level = 0;

static uint8_t  battery_level = 0;
static uint8_t  bt_profile    = 0;

static bool dip_switch_bt_en = false;

void iton_bt_connection_successful() {
    set_output(OUTPUT_BLUETOOTH);
    ev_connected  = 2500;
    ev_pairing    = false;
    ev_connecting = false;
}

void iton_bt_entered_pairing() {
    ev_pairing    = true;
    ev_connected  = 0;
    ev_connecting = false;
}

void iton_bt_enters_connection_state() {
    ev_connecting = true;
    ev_connected  = 0;
    ev_pairing    = false;
}

void iton_bt_disconnected() {
    set_output(OUTPUT_USB);
    ev_disconnected = 2500;
    ev_connected    = 0;
    ev_pairing      = false;
    ev_connecting   = false;
}

void iton_bt_battery_level(uint8_t level) {
    battery_level    = level;
    ev_battery_level = 2500;
}

static void set_bt_profile(uint8_t profile) {
    iton_bt_switch_profile(profile);
    bt_profile = profile;
}

#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // TODO: remove
    // uprintf("K=%u\n", keycode);
#ifdef BLUETOOTH_ENABLE
    if (dip_switch_bt_en && record->event.pressed) {
        switch (keycode) {
            case KC_BTPROF0:
                set_bt_profile(0);
                return false;
            case KC_BTPROF1:
                set_bt_profile(1);
                return false;
            case KC_BTPROF2:
                set_bt_profile(2);
                return false;
            case KC_BTPAIR:
                iton_bt_enter_pairing();
                return false;
            case KC_BTRST:
                iton_bt_reset_pairing();
                return false;
            case KC_BTBATT:
                ev_battery_level = 10000;
                iton_bt_query_battery_level();
                return false;
        }
    }
#endif
//TODO: remove
    // switch (keycode) {
    //     case KC_MISSION_CONTROL:
    //         if (record->event.pressed) {
    //             host_consumer_send(0x29F);
    //         } else {
    //             host_consumer_send(0);
    //         }
    //         return false;
    //     case KC_LAUNCHPAD:
    //         if (record->event.pressed) {
    //             host_consumer_send(0x2A0);
    //         } else {
    //             host_consumer_send(0);
    //         }
    //         return false;
    //     default:
    //         break;
    // }
    return process_record_user(keycode, record);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer != WIN_BASE && layer != MAC_BASE) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                uint8_t index = g_led_config.matrix_co[row][col];

                if (index >= led_min && index < led_max && index != NO_LED && keymap_key_to_keycode(layer, (keypos_t){col, row}) > KC_TRNS) {
                    rgb_matrix_set_color(index, RGB_WHITE);
                }
            }
        }
    }

#ifdef BLUETOOTH_ENABLE
    if (!dip_switch_bt_en) {
        return true;
    }

    uint32_t current_time = timer_read();
    uint32_t elapsed;
    if (current_time >= last_update_time) {
        elapsed = current_time - last_update_time;
    } else {
        elapsed = 1;
    }
    last_update_time = current_time;

    if (ev_connected > 0) {
        uint8_t profile_index = BT_PROFILE_LED + bt_profile;
        if ((current_time / 250) % 2 == 0) {
            rgb_matrix_set_color(profile_index, RGB_GREEN);
        }
        if (elapsed >= ev_connected) {
            ev_connected = 0;
        } else {
            ev_connected -= elapsed;
        }
    }

    if (ev_connecting) {
        uint8_t profile_index = BT_PROFILE_LED + bt_profile;
        if ((current_time / 125) % 2 == 0) {
            rgb_matrix_set_color(profile_index, RGB_YELLOW);
        }
    }

    if (ev_pairing) {
        uint8_t profile_index = BT_PROFILE_LED + bt_profile;
        if ((current_time / 62) % 2 == 0) {
            rgb_matrix_set_color(profile_index, RGB_BLUE);
        }
    }

    if (ev_disconnected > 0) {
        uint8_t profile_index = BT_PROFILE_LED + bt_profile;
        if ((current_time / 250) % 2 == 0) {
            rgb_matrix_set_color(profile_index, RGB_RED);
        }
        if (elapsed >= ev_disconnected) {
            ev_disconnected = 0;
        } else {
            ev_disconnected -= elapsed;
        }
    }

    if (ev_battery_level > 0) {
        if (battery_level == 4) {
            rgb_matrix_set_color(BT_BATTERY_LED, RGB_GREEN);
        } else if (battery_level == 3) {
            rgb_matrix_set_color(BT_BATTERY_LED, RGB_YELLOW);
        } else if (battery_level == 2) {
            rgb_matrix_set_color(BT_BATTERY_LED, RGB_ORANGE);
        } else if (battery_level == 1) {
            rgb_matrix_set_color(BT_BATTERY_LED, RGB_RED);
        } else {
            rgb_matrix_set_color(BT_BATTERY_LED, RGB_WHITE);
        }

        if (elapsed >= ev_battery_level) {
            ev_battery_level = 0;
            battery_level    = 0;
        } else {
            ev_battery_level -= elapsed;
        }
    }

#endif
    return true;
}

bool dip_switch_update_user(uint8_t index, bool active) {
    dprintf("DIP #%u: %u\n", index, active);
    switch (index) {
        case DIP_WIN_MAC:
            if (active) {
                layer_move(MAC_BASE);
            } else {
                layer_move(WIN_BASE);
            }
            return false;
#ifdef BLUETOOTH_ENABLE
        case DIP_BT_EN:
            dip_switch_bt_en = !active;
            if (active) {
                set_output(OUTPUT_USB);
            } else {
                set_output(OUTPUT_NONE);
            }
            return false;
#endif
    }
    return true;
}

void keyboard_post_init_user(void) {
    debug_enable = true;

    uprintf("K2: boot OK\n");
#ifdef BLUETOOTH_ENABLE
    uprintf("K2: BT=%u\n", dip_switch_bt_en);
#else
    uprintf("K2: BT=0\n");
#endif
}
