/*
Copyright 2020 Adam Honse <calcprogrammer1@gmail.com>
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

// Backlight configuration
#define BACKLIGHT_LEVELS 8

#define LED_MATRIX_ROWS 7
#define LED_MATRIX_ROW_CHANNELS 3
#define LED_MATRIX_ROWS_HW (LED_MATRIX_ROWS * LED_MATRIX_ROW_CHANNELS)

#define LED_MATRIX_ROW_PINS MATRIX_ROW_PINS
#define LED_MATRIX_COLS 16

#define LED_MATRIX_COL_PINS MATRIX_COL_PINS

#ifndef VIA_ENABLE
#ifdef QMK_KEYMAP_H
    #include QMK_KEYMAP_H
#endif
#endif

#ifdef KEYMAP_ISO
    #define DRIVER_LED_TOTAL 105
#else
    #define DRIVER_LED_TOTAL 104
#endif

/* indicator pins */
#define LED_PIN_ON_STATE    0
#define LED_NUM_LOCK_PIN    C6
#define LED_CAPS_LOCK_PIN   C7
#define LED_SCROLL_LOCK_PIN C8
