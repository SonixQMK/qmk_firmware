/*

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

//define pins for MX25L4006e flash chip
#define EXTERNAL_FLASH_SPI_SLAVE_SELECT_PIN A5
#define SPI0_SCK_PIN A4
#define SPI0_MOSI_PIN A3
#define SPI0_MISO_PIN A2
#define SPI_DRIVER SPID0

//define pins for both is31f3737 led controllers
#define I2C1_SCL_PIN A6
#define I2C1_SDA_PIN A7
#define IS31FL3737_SDB_PIN A0
#define I2C1_OPMODE OPMODE_I2C
#define I2C1_CLOCK_SPEED 400000
#define I2C_DRIVER I2CD0

//chip address 1 is physically closer to the escape key
#define IS31FL3737_I2C_ADDRESS_1 IS31FL3737_I2C_ADDRESS_GND
#define IS31FL3737_I2C_ADDRESS_2 IS31FL3737_I2C_ADDRESS_SDA

#define DRIVER_1_LED_TOTAL 44
#define DRIVER_2_LED_TOTAL 43
#define RGB_MATRIX_LED_COUNT (DRIVER_1_LED_TOTAL + DRIVER_2_LED_TOTAL)