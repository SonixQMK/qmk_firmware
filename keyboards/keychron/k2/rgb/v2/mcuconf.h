/* Copyright 2024 mintyleaf
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

#ifdef BLUETOOTH_ITON_BT
#    define SN32_SPI_USE_SPI0          TRUE
#    define SN32_SPI_SPI0_IRQ_PRIORITY 0
#    define SN32_GPIOA_IRQ_PRIORITY    0
#endif

#include_next <mcuconf.h>
