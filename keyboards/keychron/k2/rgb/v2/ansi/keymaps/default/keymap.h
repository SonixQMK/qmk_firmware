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

#pragma once

#include "quantum.h"

enum BT_keycodes {
    KC_BTPROF0 = SAFE_RANGE,
    KC_BTPROF1,
    KC_BTPROF2,
    KC_BTPAIR,
    KC_BTRST,
    KC_BTBATT,
};

enum layer_names {
    WIN_BASE = 0,
    WIN_FN   = 1,
    MAC_BASE = 2,
    MAC_FN   = 3,
};

#define KC_TASK LGUI(KC_TAB)
#define KC_FLXP LGUI(KC_E)
#define KC_MCTL KC_MISSION_CONTROL
#define KC_LPAD KC_LAUNCHPAD
