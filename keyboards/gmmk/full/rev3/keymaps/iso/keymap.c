/*
 * Copyright 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _FN,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* The GMMK uses a 7x16 matrix. Some keys are not wired in the matrix at their physical location */

    [_BASE] = { {    KC_ESC,     KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,      KC_F6,      KC_F7,      KC_F8,      KC_F9,      KC_F10,         KC_F11,        KC_F12,     _______,    _______,        _______           },
                {    KC_GRV,     KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       KC_6,       KC_7,       KC_8,       KC_9,       KC_0,           KC_MINS,       KC_EQL,     KC_NUM,     KC_KP_SLASH,    KC_KP_ASTERISK    },
                {    KC_TAB,     KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,           KC_LBRC,       KC_RBRC,    KC_KP_7,    KC_KP_8,        KC_KP_9           },
                {    KC_CAPS,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,        KC_QUOT,       KC_BSLS,    KC_KP_4,    KC_KP_5,        KC_KP_6           },
                {    KC_LSFT,    KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,        KC_RSFT,       KC_ENT,     KC_KP_1,    KC_KP_2,        KC_KP_3           },
                {    KC_LCTL,    KC_LGUI,    KC_LALT,    KC_SPC,     KC_RALT,    MO(_FN),    KC_APP,     KC_RCTL,    KC_LEFT,    KC_DOWN,    KC_UP,          KC_RGHT,       KC_BSPC,    KC_KP_0,    KC_KP_DOT,      KC_KP_ENTER       },
                {    KC_NUBS,    KC_PSCR,    KC_SCRL,    KC_PAUS,    KC_INS,     KC_HOME,    KC_PGUP,    KC_DEL,     KC_END,     KC_PGDN,    KC_KP_MINUS,    KC_KP_PLUS,    _______,    _______,    _______,        _______           }
              },                                                                                                                                                                                                    
    [_FN] =   { {    QK_BOOT,    KC_MYCM,    KC_WSCH,    KC_CALC,    KC_MSEL,    KC_MPRV,    KC_MNXT,    KC_MPLY,    KC_MSTP,    KC_MUTE,    KC_VOLD,        KC_VOLU,       KC_TRNS,    _______,    _______,        _______,          },
                {    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,       KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,          },
                {    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,       KC_TRNS,    RM_SPDD,    _______,        KC_TRNS,          },
                {    KC_TRNS,    KC_TRNS,    RM_TOGG,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,       KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,          },
                {    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,       KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,          },
                {    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    RM_HUED,    RM_VALD,    RM_VALU,        RM_HUEU,       KC_TRNS,    KC_TRNS,    KC_TRNS,        KC_TRNS,          },
                {    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    RGB_M_B,    RGB_M_R,    RGB_M_SW,   RM_NEXT,    RM_PREV,    KC_TRNS,        KC_TRNS,       _______,    _______,    _______,        _______           }
              }
};
