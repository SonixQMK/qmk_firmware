/*
Copyright 2022 Dimitris Mantzouranis <d3xter93@gmail.com>
Copyright 2022 Pablo Ramirez
Copyright 2023 Santanu Paik (https://github.com/ITNerd69)

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
#include "keymap.h"

#define KC_MSCR LSFT(LGUI(KC_3))
#define KC_MSNP LSFT(LGUI(KC_4))
#define KC_FIND LALT(LGUI(KC_SPC))


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  /*
    *****************************************************************************************************************
    * ESC  * F1   * F2   * F3   * F4   * F5   * F6   * F7   * F8   * F9   * F10  * F11  * F12  * PSCR * DEL  * RMOD *
    *****************************************************************************************************************
    * ~    * 1    * 2    * 3    * 4    * 5    * 6    * 7    * 8    * 9    * 0    * *    * =    * BSPC        * PGUP *
    *****************************************************************************************************************
    * TAB     * Q    * W    * E    * R    * T    * Y    * U    * I    * O    * P    * [    * ]    * \        * PGDN *
    *****************************************************************************************************************
    * CAPS      * A    * S    * D    * F    * G    * H    * J    * K    * L    * ;    * '    * ENT           * HOME *
    *****************************************************************************************************************
    * LSFT         * Z    * X    * C    * V    * B    * N    * M    * ,    * .    * /    * RSFT       * UP   * END  *
    *****************************************************************************************************************
    * LCTL  * LGUI  * LALT  * SPC                                         * RALT * FN   * RCTL * LEFT * DOWN * RGHT *
    *****************************************************************************************************************
  */
  [WIN_BASE] = LAYOUT_ansi(
  /*  0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15       */
      KC_ESC,     KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,      KC_F6,      KC_F7,      KC_F8,      KC_F9,      KC_F10,     KC_F11,     KC_F12,     KC_PSCR,    KC_DEL,     RM_NEXT  ,
      KC_GRV,     KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       KC_6,       KC_7,       KC_8,       KC_9,       KC_0,       KC_MINS,    KC_EQL,     KC_BSPC,                KC_PGUP  ,
      KC_TAB,     KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,       KC_LBRC,    KC_RBRC,    KC_BSLS,                KC_PGDN  ,
      KC_CAPS,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOT,                KC_ENT,                 KC_HOME  ,
      KC_LSFT,                KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,                KC_RSFT,    KC_UP,      KC_END   ,
      KC_LCTL,    KC_LGUI,    KC_LALT,                                        KC_SPC,                                         KC_RALT,    MO(WIN_FN), KC_RCTL,    KC_LEFT,    KC_DOWN,    KC_RGHT
  ),

  /*
    *****************************************************************************************************************
    * BKBT * BRID * BRIU * TASK * FLXP * RVAD * RVAI * MPRV * MPLY * MNXT * MUTE * VOLD * VOLU *      *  INS * RTOG *
    *****************************************************************************************************************
    *BPAIR*BPF0 *BPF1 *BPF2 *      *      *      *      *      *      *      *BRST *      *             *      *
    *****************************************************************************************************************
    *         *      *      *      *      *      *      *      *      *      *      *      *      *          *      *
    *****************************************************************************************************************
    *           *      *      *      *      *      *      *      *      *      *      *      *               *      *
    *****************************************************************************************************************
    *              *      *      *      *      *      *      *      *      *      *      *            * RSAI *BBATT*
    *****************************************************************************************************************
    *       *       *       *                                             *      *      *      * RHUD * RSAD * RHUI *
    *****************************************************************************************************************
  */
  [WIN_FN] = LAYOUT_ansi(
  /*  0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15       */
      QK_BOOT,    KC_BRID,    KC_BRIU,    KC_TASK,    KC_FLXP,    RM_VALD,    RM_VALU,    KC_MPRV,    KC_MPLY,    KC_MNXT,    KC_MUTE,    KC_VOLD,    KC_VOLU,    _______,    KC_INS,     RM_TOGG  ,
      KC_BTPAIR,  KC_BTPROF0, KC_BTPROF1, KC_BTPROF2, _______,    _______,    _______,    _______,    _______,    _______,    _______,    KC_BTRST,    _______,    _______,                _______  ,
      _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______  ,
      _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______,                _______  ,
      _______,                _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______,    RM_SATU,    KC_BTBATT ,
      _______,    _______,    _______,                                        _______,                                        _______,    _______,    _______,    RM_HUED,    RM_SATD,    RM_HUEU
  ),

    /*
    *****************************************************************************************************************
    * ESC  * F1   * F2   * F3   * F4   * F5   * F6   * F7   * F8   * F9   * F10  * F11  * F12  * PSCR * DEL  * RMOD *
    *****************************************************************************************************************
    * ~    * 1    * 2    * 3    * 4    * 5    * 6    * 7    * 8    * 9    * 0    * *    * =    * BSPC        * PGUP *
    *****************************************************************************************************************
    * TAB     * Q    * W    * E    * R    * T    * Y    * U    * I    * O    * P    * [    * ]    * \        * PGDN *
    *****************************************************************************************************************
    * CAPS      * A    * S    * D    * F    * G    * H    * J    * K    * L    * ;    * '    * ENT           * HOME *
    *****************************************************************************************************************
    * LSFT         * Z    * X    * C    * V    * B    * N    * M    * ,    * .    * /    * RSFT       * UP   * END  *
    *****************************************************************************************************************
    * LCTL  * LALT  * LGUI  * SPC                                         * LGUI * FN   * RCTL * LEFT * DOWN * RGHT *
    *****************************************************************************************************************
  */
  [MAC_BASE] = LAYOUT_ansi(
  /*  0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15       */
      KC_ESC,     KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,      KC_F6,      KC_F7,      KC_F8,      KC_F9,      KC_F10,     KC_F11,     KC_F12,     KC_MSCR,    KC_DEL,     RM_NEXT  ,
      KC_GRV,     KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       KC_6,       KC_7,       KC_8,       KC_9,       KC_0,       KC_MINS,    KC_EQL,     KC_BSPC,                KC_PGUP  ,
      KC_TAB,     KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,       KC_LBRC,    KC_RBRC,    KC_BSLS,                KC_PGDN  ,
      KC_CAPS,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOT,                KC_ENT,                 KC_HOME  ,
      KC_LSFT,                KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM,    KC_DOT,     KC_SLSH,                KC_RSFT,    KC_UP,      KC_END   ,
      KC_LCTL,    KC_LALT,    KC_LGUI,                                        KC_SPC,                                         KC_RGUI,    MO(MAC_FN), KC_RCTL,    KC_LEFT,    KC_DOWN,    KC_RGHT
  ),

  /*
    *****************************************************************************************************************
    * BKBT * BRID * BRIU * MCTL * LPAD * RVAD * RVAI * MPRV * MPLY * MNXT * MUTE * VOLD * VOLU * MSNP *  INS * RTOG *
    *****************************************************************************************************************
    *BPAIR*BPF0 *BPF1 *BPF2 *      *      *      *      *      *      *      *BRST *      *             *      *
    *****************************************************************************************************************
    *         *      *      *      *      *      *      *      *      *      *      *      *      *          *      *
    *****************************************************************************************************************
    *           *      *      *      *      *      *      *      *      *      *      *      *               *FIND *
    *****************************************************************************************************************
    *              *      *      *      *      *      *      *      *      *      *      *            * RSAI *BBATT*
    *****************************************************************************************************************
    *       *       *       *                                             *      *      *      * RHUD * RSAD * RHUI *
    *****************************************************************************************************************
  */
  [MAC_FN] = LAYOUT_ansi(
  /*  0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15       */
      QK_BOOT,    KC_BRID,    KC_BRIU,    KC_MCTL,    KC_LPAD,    RM_VALD,    RM_VALU,    KC_MPRV,    KC_MPLY,    KC_MNXT,    KC_MUTE,    KC_VOLD,    KC_VOLU,    KC_MSNP,    KC_INS,     RM_TOGG  ,
      KC_BTPAIR,  KC_BTPROF0, KC_BTPROF1, KC_BTPROF2, _______,    _______,    _______,    _______,    _______,    _______,    _______,    KC_BTRST,    _______,    _______,                _______  ,
      _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______  ,
      _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______,                KC_FIND  ,
      _______,                _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,    _______,                _______,    RM_SATU,    KC_BTBATT ,
      _______,    _______,    _______,                                        _______,                                        _______,    _______,    _______,    RM_HUED,    RM_SATD,    RM_HUEU
  )
};
