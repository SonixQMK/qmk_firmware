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

#include <stdint.h>
#include <stdbool.h>

#if !defined(SN32F2XX_LED_OUTPUT_LUMINOSITY_R)
#    define SN32F2XX_LED_OUTPUT_LUMINOSITY_R 1
#endif
#if !defined(SN32F2XX_LED_OUTPUT_LUMINOSITY_G)
#    define SN32F2XX_LED_OUTPUT_LUMINOSITY_G 1
#endif
#if !defined(SN32F2XX_LED_OUTPUT_LUMINOSITY_B)
#    define SN32F2XX_LED_OUTPUT_LUMINOSITY_B 1
#endif

#if defined(RGB_MATRIX_SN32F2XX)
#    define SN32F2XX_LED_COUNT RGB_MATRIX_LED_COUNT
#endif

#if !defined(SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL)
#    define SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL SN32F2XX_PWM_OUTPUT_ACTIVE_LOW
#endif

#if !defined(SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL)
#    define SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH
#endif

#if !defined(SN32F2XX_PWM_CONTROL)
#    define SN32F2XX_PWM_CONTROL HARDWARE_PWM
#endif

#if !defined(SN32F2XX_PWM_DIRECTION)
#    define SN32F2XX_PWM_DIRECTION DIODE_DIRECTION
#endif

#if !defined(SN32F2XX_RGB_MATRIX_ROW_CHANNELS)
#    define SN32F2XX_RGB_MATRIX_ROW_CHANNELS 3
#endif

#if !defined(SN32F2XX_RGB_MATRIX_ROWS)
#    define SN32F2XX_RGB_MATRIX_ROWS MATRIX_ROWS
#endif

#if !defined(SN32F2XX_RGB_MATRIX_COLS)
#    define SN32F2XX_RGB_MATRIX_COLS MATRIX_COLS
#endif

#if !defined(SN32F2XX_RGB_MATRIX_ROW_PINS)
#    error Missing definition for SN32F2XX_RGB_MATRIX_ROW_PINS
#endif

#if !defined(SN32F2XX_RGB_MATRIX_COL_PINS)
#    define SHARED_MATRIX
#    define SN32F2XX_RGB_MATRIX_COL_PINS MATRIX_COL_PINS
#endif

#if !defined(SN32F2XX_RGB_MATRIX_ROWS_HW)
#    define SN32F2XX_RGB_MATRIX_ROWS_HW (SN32F2XX_RGB_MATRIX_ROWS * SN32F2XX_RGB_MATRIX_ROW_CHANNELS)
#endif

void sn32f2xx_init(void);
void sn32f2xx_flush(void);
void sn32f2xx_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void sn32f2xx_set_color_all(uint8_t r, uint8_t g, uint8_t b);
