/*
 * Copyright 2025 ExoticPubis
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
#define DYNAMIC_KEYMAP_LAYER_COUNT 3

#define MATRIX_UNSELECT_DRIVE_HIGH

#define RGB_MATRIX_LED_COUNT 104
#define SN32F2XX_RGB_MATRIX_ROW_PINS { C0, C1, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, B6, B7, B8, B9, B10, B11 }

#define RGB_MATRIX_TYPING_HEATMAP_DECREASE_DELAY_MS 50

#define SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH
#define SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL SN32F2XX_RGB_OUTPUT_ACTIVE_LOW

/* Color correction
 * Need to manually adjust the output of the green and red channel...
 * If not set, the colors are wrong depending of the output from each color channel.
 * While looking at the PCB, I found out why... There is only a single 62 ohms resistor
 * for the R, G, and B channel for each key. Because of their different voltage drop, they
 * they wont draw power equally, resulting in the output being wrong if they are all set to 100%.
 */

#define SN32F2XX_LED_OUTPUT_LUMINOSITY_R 0.25
#define SN32F2XX_LED_OUTPUT_LUMINOSITY_G 0.6
