/* Copyright 2022 QMK
 * Copyright 2024 Dimitris Mantzouranis <d3xter93@gmail.com>
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

#define RAW_IN_CAPACITY 1
#define RAW_OUT_CAPACITY 1

#ifndef EARLY_INIT_PERFORM_BOOTLOADER_JUMP
#    define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE
#endif

#ifndef WEAR_LEVELING_EFL_OMIT_LAST_SECTOR_COUNT
#    define WEAR_LEVELING_EFL_OMIT_LAST_SECTOR_COUNT 1
#endif

/* The SN32F26x has only 2 KB of RAM. */

/* Logical EEPROM is cached in RAM: 64 bytes hold the 37-byte eeconfig core block
 * with some room for keyboard/user data. */
#ifndef WEAR_LEVELING_LOGICAL_SIZE
#    define WEAR_LEVELING_LOGICAL_SIZE 64
#endif
#ifndef WEAR_LEVELING_BACKING_SIZE
#    define WEAR_LEVELING_BACKING_SIZE 512
#endif

/* USB IN endpoint queues: two reports deep instead of four. */
#ifndef USB_DEFAULT_BUFFER_CAPACITY
#    define USB_DEFAULT_BUFFER_CAPACITY 2
#endif

/* The hardware-ID serial number descriptor is built in RAM. */
#ifndef SERIAL_NUMBER_USE_HARDWARE_ID
#    define SERIAL_NUMBER_USE_HARDWARE_ID FALSE
#endif