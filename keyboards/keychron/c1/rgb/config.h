/*
Copyright 2024 NetUserGet

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
#define SN32F2XX_RGB_MATRIX_ROW_PINS  { C0, C1, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, B13, D3, B15, B14 } /* This *is* supposed to be in info.json */

#define LED_MAC_PIN  B11 // Labeled Mac on KB since no Scroll Lock
#define LED_WIN_PIN  B12 // Labeled Windows on KB since no Numpad

// START LAYER DEFINITIONS
#define _WIN_BASE 0
#define _WIN_FN   1
#define _MAC_BASE 2
#define _MAC_FN   3
