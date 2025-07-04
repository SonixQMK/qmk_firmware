/*
Copyright 2020 Dimitris Mantzouranis
Copyright 2022 Philip Mourdjis
Copyright 2023 Fernando Birra

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
#include QMK_KEYBOARD_H

#define CAPS_LOCK_LED   28

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
enum layer_names {
    WINBASE,
    WINFN,
    MACBASE,
    MACFN,
    FNS,
};

enum custom_keycodes {
    KC_WIN_KEY = SAFE_RANGE,
    KC_MAC_KEY
};

#define KC_TASK LGUI(KC_TAB)        // Task viewer
#define KC_FLXP LGUI(KC_E)          // Windows file explorer
#define KC_MCTL KC_MISSION_CONTROL  // Mission Control
#define KC_LPAD KC_LAUNCHPAD        // Launchpad
#define KC_MAC KC_MAC_KEY
#define KC_WIN KC_WIN_KEY

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Win Base
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐
     * │Esc│ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │   Shift  │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┴───┴┬────┬────┤
     * │Ctrl│GUI │Alt │         Space          │Alt │APP │Ctr │ Fn │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘
     */
    [WINBASE] = LAYOUT_60_ansi(
        QK_GESC,    KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       KC_6,       KC_7,       KC_8,       KC_9,       KC_0,       KC_MINS,    KC_EQL,     KC_BSPC,    
        KC_TAB,     KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,       KC_LBRC,    KC_RBRC,    KC_BSLS,    
        KC_CAPS,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOT,                KC_ENT,     
        KC_LSFT,                KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,    KC_RSFT,                
        KC_LCTL,    KC_LGUI,    KC_LALT,                                        KC_SPC,                                         KC_RALT,    KC_APP,     KC_RCTL,    MO(WINFN)
    ),
    /* Win FN
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐
     * │`~ │BRU│BRD│TSK│FLX│VAD│VAI│PRV│PLY│NXT│MTE│VLD│VLU│ Backsp│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤
     * │     │   │   │   │   │   │Prt│Scr│Pau│   │   │VAD│VAI│ MOD │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤
     * │      │WIN│MAC│   │   │   │Ins│Hom│PgU│   │SPD│SPI│        │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤
     * │        │   │   │   │   │   │Del│End│PgD│HUI│ ↑ │   Shift  │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┴───┴┬────┬────┤
     * │BOOT│    │    │         Space          │  ← │  ↓ │  → │ Fn │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘
     */
    [WINFN] = LAYOUT_60_ansi(
        KC_GRV,     KC_BRIU,    KC_BRID,    KC_TASK,    KC_FLXP,    RGB_VAD,    RGB_VAI,    KC_MPRV,    KC_MPLY,    KC_MNXT,    KC_MUTE,    KC_VOLD,    KC_VOLU,    _______,
        _______,    _______,    _______,    _______,    _______,    _______,    KC_PSCR,    KC_SCRL,    KC_PAUS,    _______,    _______,    RGB_VAD,    RGB_VAI,    RGB_MOD,   
        _______,    _______,    KC_MAC,     _______,    _______,    _______,    KC_INS,     KC_HOME,    KC_PGUP,    _______,    RGB_SPD,    RGB_SPI,                _______,   
        _______,                _______,    _______,    _______,    _______,    _______,    KC_DEL,     KC_END,     KC_PGDN,    RGB_HUI,    KC_UP,      MO(FNS),   
        QK_BOOT,    _______,    _______,                                        _______,                                        KC_LEFT,    KC_DOWN,    KC_RIGHT,   _______
    ),
    /* Mac Base
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐
     * │Esc│ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │   Shift  │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┴───┴┬────┬────┤
     * │Ctrl│Alt │GUI │         Space          │GUI │Alt │Ctr │ Fn │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘
     */
    [MACBASE] = LAYOUT_60_ansi(
        QK_GESC,    KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       KC_6,       KC_7,       KC_8,       KC_9,       KC_0,       KC_MINS,    KC_EQL,     KC_BSPC,    
        KC_TAB,     KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,       KC_LBRC,    KC_RBRC,    KC_BSLS,    
        KC_CAPS,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOT,                KC_ENT,     
        KC_LSFT,                KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,    KC_RSFT,                
        KC_LCTL,    KC_LALT,    KC_LCMD,                                        KC_SPC,                                         KC_RCMD,    KC_RALT,    KC_RCTL,    MO(MACFN)
    ),
    /* Mac FN
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐
     * │`~ │BRU│BRD│MCT│LPD│VAD│VAI│PRV│PLY│NXT│MTE│VLD│VLU│ Backsp│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤
     * │     │   │   │   │   │   │Prt│Scr│Pau│   │   │VAD│VAI│ MOD │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤
     * │      │WIN│MAC│   │   │   │Ins│Hom│PgU│   │SPD│SPI│        │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤
     * │        │   │   │   │   │   │Del│End│PgD│HUI│ ↑ │   Shift  │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┴───┴┬────┬────┤
     * │BOOT│    │    │         Space          │  ← │  ↓ │  → │ Fn │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘
     */
    [MACFN] = LAYOUT_60_ansi(
        KC_GRV,     KC_BRIU,    KC_BRID,    KC_MCTL,    KC_LPAD,    RGB_VAD,    RGB_VAI,    KC_MPRV,    KC_MPLY,    KC_MNXT,    KC_MUTE,    KC_VOLD,    KC_VOLU,    _______,
        _______,    _______,    _______,    _______,    _______,    _______,    KC_PSCR,    KC_SCRL,    KC_PAUS,    _______,    _______,    RGB_VAD,    RGB_VAI,    RGB_MOD,   
        _______,    KC_WIN,     _______,    _______,    _______,    _______,    KC_INS,     KC_HOME,    KC_PGUP,    _______,    RGB_SPD,    RGB_SPI,                _______,   
        _______,                KC_MUTE,    KC_VOLD,    KC_VOLU,    KC_MPRV,    KC_MNXT,    KC_DEL,     KC_END,     KC_PGDN,    RGB_HUI,    KC_UP,      MO(FNS),   
        QK_BOOT,    _______,    _______,                                        _______,                                        KC_LEFT,    KC_DOWN,    KC_RIGHT,   _______
    ),
    /* Mac FN + Right Shift
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐
     * │   │F1 │F2 │F3 │F4 │F5 │F6 │F7 │F8 │F9 │F10│F11│F12│       │
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤
     * │     │   │   │   │   │   │   │   │   │   │   │   │   │     │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤
     * │      │   │   │   │   │   │   │   │   │   │   │   │        │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤
     * │        │   │   │   │   │   │   │   │   │   │   │          │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┴───┴┬────┬────┤
     * │    │    │    │                        │    │    │    │    │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘
     */
    [FNS] = LAYOUT_60_ansi(
        KC_TILD,    KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,      KC_F6,      KC_F7,      KC_F8,      KC_F9,      KC_F10,     KC_F11,     KC_F12,     _______,
        _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,
        _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______,
        _______,                _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,
        _______,    _______,    _______,                                        _______,                                        _______,    _______,    _______,    _______
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_WIN:
            if(record->event.pressed) {
                set_single_persistent_default_layer(WINBASE);
            }
            return false;
        case KC_MAC:
            if(record->event.pressed) {
                set_single_persistent_default_layer(MACBASE);
            }
            return false;
        case KC_MISSION_CONTROL:
            if (record->event.pressed) {
                host_consumer_send(0x29F);
            } else {
                host_consumer_send(0);
            }
            return false;  // Skip all further processing of this key
        case KC_LAUNCHPAD:
            if (record->event.pressed) {
                host_consumer_send(0x2A0);
            } else {
                host_consumer_send(0);
            }
            return false;  // Skip all further processing of this key
        default:
            return true;  // Process all other keycodes normally
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (host_keyboard_led_state().caps_lock) {
        // Set capslock key to orange (capslock is led number 30)
        rgb_matrix_set_color(CAPS_LOCK_LED, 255, 130, 15);
    }
    return false;
}