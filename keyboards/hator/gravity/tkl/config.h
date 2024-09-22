/*
Copyright 2024 AlexHtr
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

#define SN32F2XX_RGB_MATRIX_ROW_PINS { C3,C1,C0, C6,C5,C4, C9,C8,C7, C12,C11,C10, B13,C14,C13, B14,B15,D3, A2,A4,A3 }

#define SN32F2XX_RGB_MATRIX_COL_PINS {A8, A9, A10, A11, A12, A13, A14, A15, B0, B1, B2, B3, B4, B5, B6, B7, B8}

#define RGB_MATRIX_LED_COUNT 104
#define MATRIX_UNSELECT_DRIVE_HIGH
//#define SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH
#define SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH
//#define USB_MAX_POWER_CONSUMPTION 500
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 240