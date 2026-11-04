/* Copyright 2026 QMK
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

#include <stdint.h>
#include <stdbool.h>

#if !defined(VS11K09A_RGB_MATRIX_ROW_CHANNELS)
#    define VS11K09A_RGB_MATRIX_ROW_CHANNELS 3
#endif

#if !defined(VS11K09A_RGB_MATRIX_ROWS)
#    define VS11K09A_RGB_MATRIX_ROWS MATRIX_ROWS
#endif

#if !defined(VS11K09A_RGB_MATRIX_COLS)
#    define VS11K09A_RGB_MATRIX_COLS MATRIX_COLS
#endif

#if !defined(VS11K09A_RGB_MATRIX_ROWS_HW)
#    define VS11K09A_RGB_MATRIX_ROWS_HW (VS11K09A_RGB_MATRIX_ROWS * VS11K09A_RGB_MATRIX_ROW_CHANNELS)
#endif

#if !defined(VS11K09A_RGB_MATRIX_ROW_PINS)
#    if defined(LED_MATRIX_ROW_PINS)
#        define VS11K09A_RGB_MATRIX_ROW_PINS LED_MATRIX_ROW_PINS
#    else
#        error "Missing definition for VS11K09A_RGB_MATRIX_ROW_PINS"
#    endif
#endif

#if !defined(VS11K09A_RGB_MATRIX_COL_PINS)
#    define SHARED_MATRIX
#    define VS11K09A_RGB_MATRIX_COL_PINS MATRIX_COL_PINS
#endif

void vs11k09a_init(void);
void vs11k09a_flush(void);
void vs11k09a_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void vs11k09a_set_color_all(uint8_t r, uint8_t g, uint8_t b);
