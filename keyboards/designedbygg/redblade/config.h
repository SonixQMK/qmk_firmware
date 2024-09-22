/* Copyright 2024 Dimitris Mantzouranis <d3xter93@gmail.com>
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

#pragma once

/* Board and GPIO setup */
#ifndef RGB_MATRIX_ENABLE
#define WAIT_US_TIMER GPTD2
#endif
#define MATRIX_UNSELECT_DRIVE_HIGH
#define MATRIX_IO_DELAY 1
#define GPIO_INPUT_PIN_DELAY 0

/* Debug options */
#define DEBUG_MATRIX_SCAN_RATE

/* Win Lock LED Indicator */
#define LED_WIN_LOCK_PIN B12

/* RGB LED Config */
#define SN32F2XX_RGB_MATRIX_ROW_PINS \
    { C0, C1, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, B6, B7, B8, B9, B10, B11 }
#define RGB_MATRIX_LED_COUNT (109)
