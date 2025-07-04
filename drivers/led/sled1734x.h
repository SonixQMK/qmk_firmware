/* Copyright 2023 Dimitris Mantzouranis <d3xter93@gmail.com>
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
#include "progmem.h"
#include "util.h"

/* Command Registers Declarations */
#define SLED1734X_REG_COMMAND 0xFD
#define SLED1734X_COMMAND_FRAME_1 0x00
#define SLED1734X_COMMAND_FRAME_2 0x01
#define SLED1734X_COMMAND_FUNCTION 0x0B
#define SLED1734X_COMMAND_CURRENT_TUNE 0x0C
#define SLED1734X_COMMAND_VAF_TUNE 0x0D

/* Function Registers Declarations */
#define SLED1734X_FUNCTION_REG_CONFIGURATION 0x00     // Configuration register
#define SLED1734X_FUNCTION_REG_PICTUREDISPLAY 0x01    // Picture Display register
#define SLED1734X_FUNCTION_REG_DISPLAYOPTION 0x05     // Display Option register
#define SLED1734X_FUNCTION_REG_AUDIOSYNC 0x06         // Audio Sync register
#define SLED1734X_FUNCTION_REG_BREATHCONTROL1 0x08    // Breath Control 1 register
#define SLED1734X_FUNCTION_REG_BREATHCONTROL2 0x09    // Breath Control 2 register
#define SLED1734X_FUNCTION_REG_SOFTWARE_SHUTDOWN 0x0A // Shutdown register
#define SLED1734X_FUNCTION_REG_AUDIOGAIN_CONTROL 0x0B // AGC Control register
#define SLED1734X_FUNCTION_REG_STAGGERED_DELAY 0x0D   // Staggered Delay register
#define SLED1734X_FUNCTION_REG_SLEW_RATE_CONTROL 0x0E // Slew Rate Control register
#define SLED1734X_FUNCTION_REG_CURRENT_CONTROL 0x0F   // Current Control register
#define SLED1734X_FUNCTION_REG_OPEN_SHORT_1 0x10      // Open Short Test 1 register
#define SLED1734X_FUNCTION_REG_OPEN_SHORT_2 0x11      // Open Short Test 2 register
#define SLED1734X_FUNCTION_REG_VAF_1 0x14             // VAF 1 register
#define SLED1734X_FUNCTION_REG_VAF_2 0x15             // VAF 2 register
#define SLED1734X_FUNCTION_REG_THERMAL_DETECTION 0x17 // Thermal Detection register
#define SLED1734X_FUNCTION_REG_TP4VAF_1 0x18          // TP4Vaf Register 1
#define SLED1734X_FUNCTION_REG_TP4VAF_2 0x19          // TP4Vaf Register 2
#define SLED1734X_FUNCTION_REG_TP4VAF_3 0x1A          // TP4Vaf Register 3
#define SLED1734X_FUNCTION_REG_ID 0x1B                // Chip ID Register

/* Function Registers Defaults */
// Sync Mode. 00 - High Impedance, 01 Master, 10 Slave
#ifndef SLED1734X_SYNC_MODE
#    define SLED1734X_SYNC_MODE 0x00
#endif
// Matrix Type. 00 - Type-1, 01 Type-2, 10 Type-3, 11 Type-4
#ifndef SLED1734X_MATRIX_TYPE
#    define SLED1734X_MATRIX_TYPE 0x10
#endif
// Blink Frame and Enable. Default to Disabled
#ifndef SLED1734X_BLINK_FRAME
#    define SLED1734X_BLINK_FRAME 0x00
#endif
// Audio Sync Enable. Default to Disabled
#ifndef SLED1734X_AUDIOSYNC_ENABLE
#    define SLED1734X_AUDIOSYNC_ENABLE 0x00
#endif
// Fade Out and Fade In Time setting. Default to 0
#ifndef SLED1734X_FADE_TIME
#    define SLED1734X_FADE_TIME 0x00
#endif
// Breathe and Continuous Breathe Enable. Default to Disabled
#ifndef SLED1734X_BREATHE_ENABLE
#    define SLED1734X_BREATHE_ENABLE 0x00
#endif
// Audio Gain Control. Mode, Gain and Enable setting. Default to Disabled
#ifndef SLED1734X_AUDIOGAIN_MODE
#    define SLED1734X_AUDIOGAIN_MODE 0x00
#endif
// Staggered Delay Timing setting. Default to 0
#ifndef SLED1734X_STAGGERED_DELAY_TIMING
#    define SLED1734X_STAGGERED_DELAY_TIMING 0x00
#endif
// Slew Rate Control Enable. Default to Enabled
#ifndef SLED1734X_SLEW_RATE_CONTROL_ENABLE
#    define SLED1734X_SLEW_RATE_CONTROL_ENABLE 0x01
#endif
// Current Control Enable. Default to Disabled
#ifndef SLED1734X_CURRENT_CONTROL_ENABLE
#    define SLED1734X_CURRENT_CONTROL_ENABLE 0xB0
#endif
// Open Short Detection Mode. Default to Disabled
#ifndef SLED1734X_OPEN_SHORT_1_MODE
#    define SLED1734X_OPEN_SHORT_1_MODE 0x00
#endif
// Open Short Detection Interrupt. Default to Disabled
#ifndef SLED1734X_OPEN_SHORT_2_MODE
#    define SLED1734X_OPEN_SHORT_2_MODE 0x00
#endif
// VAF fine tune setting. Default to 0x44
#ifndef SLED1734X_VAF_1_TUNE
#    define SLED1734X_VAF_1_TUNE 0x44
#endif
// VAF control setting. Default to 0x04
#ifndef SLED1734X_VAF_2_TUNE
#    define SLED1734X_VAF_2_TUNE 0x04
#endif

/* SW Shutdown Declarations */
// 0 - SW Shutdown Mode, 1 Normal Mode
#define SLED1734X_SOFTWARE_SHUTDOWN_SSD_SHUTDOWN 0x00
#define SLED1734X_SOFTWARE_SHUTDOWN_SSD_NORMAL 0x01

/* I2C Address Declarations */
#define SLED1734X_I2C_ADDRESS_GND 0x74
#define SLED1734X_I2C_ADDRESS_SCL 0x75
#define SLED1734X_I2C_ADDRESS_SDA 0x76
#define SLED1734X_I2C_ADDRESS_VDDIO 0x77

#if defined(RGB_MATRIX_SLED1734X)
#    define SLED1734X_LED_COUNT RGB_MATRIX_LED_COUNT
#endif

#if defined(SLED1734X_I2C_ADDRESS_4)
#    define SLED1734X_DRIVER_COUNT 4
#elif defined(SLED1734X_I2C_ADDRESS_3)
#    define SLED1734X_DRIVER_COUNT 3
#elif defined(SLED1734X_I2C_ADDRESS_2)
#    define SLED1734X_DRIVER_COUNT 2
#elif defined(SLED1734X_I2C_ADDRESS_1)
#    define SLED1734X_DRIVER_COUNT 1
#endif

typedef struct sled1734x_led_t {
    uint8_t driver : 2;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} PACKED sled1734x_led_t;

extern const sled1734x_led_t PROGMEM g_sled1734x_leds[SLED1734X_LED_COUNT];

void sled1734x_init_drivers(void);
void sled1734x_init(uint8_t index);
void sled1734x_write_register(uint8_t index, uint8_t reg, uint8_t data);
void sled1734x_select_page(uint8_t index, uint8_t page);

void sled1734x_set_color(int index, uint8_t red, uint8_t green, uint8_t blue);
void sled1734x_set_color_all(uint8_t red, uint8_t green, uint8_t blue);

void sled1734x_set_led_control_register(uint8_t index, bool red, bool green, bool blue);

void sled1734x_flush(void);

void sled1734x_sw_return_normal(uint8_t index);
void sled1734x_sw_shutdown(uint8_t index);

// This should not be called from an interrupt
// (eg. from a timer interrupt).
// Call this while idle (in between matrix scans).
// If the buffer is dirty, it will update the driver with the buffer.
void sled1734x_update_pwm_buffers(uint8_t index);
void sled1734x_update_led_control_registers(uint8_t index);

#define SLED1734X_OFFSET 0x20
#define SLED1734X_FRAME_OFFSET 0x80

/* Adjust the register locations by an offset to fit in a byte */

#define CA1_A (0x20 - SLED1734X_OFFSET)
#define CA1_B (0x21 - SLED1734X_OFFSET)
#define CA1_C (0x22 - SLED1734X_OFFSET)
#define CA1_D (0x23 - SLED1734X_OFFSET)
#define CA1_E (0x24 - SLED1734X_OFFSET)
#define CA1_F (0x25 - SLED1734X_OFFSET)
#define CA1_G (0x26 - SLED1734X_OFFSET)
#define CA1_H (0x27 - SLED1734X_OFFSET)
#define CA1_I (0x28 - SLED1734X_OFFSET)
#define CA1_J (0x29 - SLED1734X_OFFSET)
#define CA1_K (0x2A - SLED1734X_OFFSET)
#define CA1_L (0x2B - SLED1734X_OFFSET)
#define CA1_M (0x2C - SLED1734X_OFFSET)
#define CA1_N (0x2D - SLED1734X_OFFSET)
#define CA1_O (0x2E - SLED1734X_OFFSET)
#define CA1_P (0x2F - SLED1734X_OFFSET)

#define CA2_A (0x30 - SLED1734X_OFFSET)
#define CA2_B (0x31 - SLED1734X_OFFSET)
#define CA2_C (0x32 - SLED1734X_OFFSET)
#define CA2_D (0x33 - SLED1734X_OFFSET)
#define CA2_E (0x34 - SLED1734X_OFFSET)
#define CA2_F (0x35 - SLED1734X_OFFSET)
#define CA2_G (0x36 - SLED1734X_OFFSET)
#define CA2_H (0x37 - SLED1734X_OFFSET)
#define CA2_I (0x38 - SLED1734X_OFFSET)
#define CA2_J (0x39 - SLED1734X_OFFSET)
#define CA2_K (0x3A - SLED1734X_OFFSET)
#define CA2_L (0x3B - SLED1734X_OFFSET)
#define CA2_M (0x3C - SLED1734X_OFFSET)
#define CA2_N (0x3D - SLED1734X_OFFSET)
#define CA2_O (0x3E - SLED1734X_OFFSET)
#define CA2_P (0x3F - SLED1734X_OFFSET)

#define CA3_A (0x40 - SLED1734X_OFFSET)
#define CA3_B (0x41 - SLED1734X_OFFSET)
#define CA3_C (0x42 - SLED1734X_OFFSET)
#define CA3_D (0x43 - SLED1734X_OFFSET)
#define CA3_E (0x44 - SLED1734X_OFFSET)
#define CA3_F (0x45 - SLED1734X_OFFSET)
#define CA3_G (0x46 - SLED1734X_OFFSET)
#define CA3_H (0x47 - SLED1734X_OFFSET)
#define CA3_I (0x48 - SLED1734X_OFFSET)
#define CA3_J (0x49 - SLED1734X_OFFSET)
#define CA3_K (0x4A - SLED1734X_OFFSET)
#define CA3_L (0x4B - SLED1734X_OFFSET)
#define CA3_M (0x4C - SLED1734X_OFFSET)
#define CA3_N (0x4D - SLED1734X_OFFSET)
#define CA3_O (0x4E - SLED1734X_OFFSET)
#define CA3_P (0x4F - SLED1734X_OFFSET)

#define CA4_A (0x50 - SLED1734X_OFFSET)
#define CA4_B (0x51 - SLED1734X_OFFSET)
#define CA4_C (0x52 - SLED1734X_OFFSET)
#define CA4_D (0x53 - SLED1734X_OFFSET)
#define CA4_E (0x54 - SLED1734X_OFFSET)
#define CA4_F (0x55 - SLED1734X_OFFSET)
#define CA4_G (0x56 - SLED1734X_OFFSET)
#define CA4_H (0x57 - SLED1734X_OFFSET)
#define CA4_I (0x58 - SLED1734X_OFFSET)
#define CA4_J (0x59 - SLED1734X_OFFSET)
#define CA4_K (0x5A - SLED1734X_OFFSET)
#define CA4_L (0x5B - SLED1734X_OFFSET)
#define CA4_M (0x5C - SLED1734X_OFFSET)
#define CA4_N (0x5D - SLED1734X_OFFSET)
#define CA4_O (0x5E - SLED1734X_OFFSET)
#define CA4_P (0x5F - SLED1734X_OFFSET)

#define CA5_A (0x60 - SLED1734X_OFFSET)
#define CA5_B (0x61 - SLED1734X_OFFSET)
#define CA5_C (0x62 - SLED1734X_OFFSET)
#define CA5_D (0x63 - SLED1734X_OFFSET)
#define CA5_E (0x64 - SLED1734X_OFFSET)
#define CA5_F (0x65 - SLED1734X_OFFSET)
#define CA5_G (0x66 - SLED1734X_OFFSET)
#define CA5_H (0x67 - SLED1734X_OFFSET)
#define CA5_I (0x68 - SLED1734X_OFFSET)
#define CA5_J (0x69 - SLED1734X_OFFSET)
#define CA5_K (0x6A - SLED1734X_OFFSET)
#define CA5_L (0x6B - SLED1734X_OFFSET)
#define CA5_M (0x6C - SLED1734X_OFFSET)
#define CA5_N (0x6D - SLED1734X_OFFSET)
#define CA5_O (0x6E - SLED1734X_OFFSET)
#define CA5_P (0x6F - SLED1734X_OFFSET)

#define CA6_A (0x70 - SLED1734X_OFFSET)
#define CA6_B (0x71 - SLED1734X_OFFSET)
#define CA6_C (0x72 - SLED1734X_OFFSET)
#define CA6_D (0x73 - SLED1734X_OFFSET)
#define CA6_E (0x74 - SLED1734X_OFFSET)
#define CA6_F (0x75 - SLED1734X_OFFSET)
#define CA6_G (0x76 - SLED1734X_OFFSET)
#define CA6_H (0x77 - SLED1734X_OFFSET)
#define CA6_I (0x78 - SLED1734X_OFFSET)
#define CA6_J (0x79 - SLED1734X_OFFSET)
#define CA6_K (0x7A - SLED1734X_OFFSET)
#define CA6_L (0x7B - SLED1734X_OFFSET)
#define CA6_M (0x7C - SLED1734X_OFFSET)
#define CA6_N (0x7D - SLED1734X_OFFSET)
#define CA6_O (0x7E - SLED1734X_OFFSET)
#define CA6_P (0x7F - SLED1734X_OFFSET)

#define CA7_A (0x80 - SLED1734X_OFFSET)
#define CA7_B (0x81 - SLED1734X_OFFSET)
#define CA7_C (0x82 - SLED1734X_OFFSET)
#define CA7_D (0x83 - SLED1734X_OFFSET)
#define CA7_E (0x84 - SLED1734X_OFFSET)
#define CA7_F (0x85 - SLED1734X_OFFSET)
#define CA7_G (0x86 - SLED1734X_OFFSET)
#define CA7_H (0x87 - SLED1734X_OFFSET)
#define CA7_I (0x88 - SLED1734X_OFFSET)
#define CA7_J (0x89 - SLED1734X_OFFSET)
#define CA7_K (0x8A - SLED1734X_OFFSET)
#define CA7_L (0x8B - SLED1734X_OFFSET)
#define CA7_M (0x8C - SLED1734X_OFFSET)
#define CA7_N (0x8D - SLED1734X_OFFSET)
#define CA7_O (0x8E - SLED1734X_OFFSET)
#define CA7_P (0x8F - SLED1734X_OFFSET)

#define CA8_A (0x90 - SLED1734X_OFFSET)
#define CA8_B (0x91 - SLED1734X_OFFSET)
#define CA8_C (0x92 - SLED1734X_OFFSET)
#define CA8_D (0x93 - SLED1734X_OFFSET)
#define CA8_E (0x94 - SLED1734X_OFFSET)
#define CA8_F (0x95 - SLED1734X_OFFSET)
#define CA8_G (0x96 - SLED1734X_OFFSET)
#define CA8_H (0x97 - SLED1734X_OFFSET)
#define CA8_I (0x98 - SLED1734X_OFFSET)
#define CA8_J (0x99 - SLED1734X_OFFSET)
#define CA8_K (0x9A - SLED1734X_OFFSET)
#define CA8_L (0x9B - SLED1734X_OFFSET)
#define CA8_M (0x9C - SLED1734X_OFFSET)
#define CA8_N (0x9D - SLED1734X_OFFSET)
#define CA8_O (0x9E - SLED1734X_OFFSET)
#define CA8_P (0x9F - SLED1734X_OFFSET)

/* Fake the locations with an offset in software for B side */

#define CA9_A (CA1_A + SLED1734X_FRAME_OFFSET)
#define CA9_B (CA1_B + SLED1734X_FRAME_OFFSET)
#define CA9_C (CA1_C + SLED1734X_FRAME_OFFSET)
#define CA9_D (CA1_D + SLED1734X_FRAME_OFFSET)
#define CA9_E (CA1_E + SLED1734X_FRAME_OFFSET)
#define CA9_F (CA1_F + SLED1734X_FRAME_OFFSET)
#define CA9_G (CA1_G + SLED1734X_FRAME_OFFSET)
#define CA9_H (CA1_H + SLED1734X_FRAME_OFFSET)
#define CA9_I (CA1_I + SLED1734X_FRAME_OFFSET)
#define CA9_J (CA1_J + SLED1734X_FRAME_OFFSET)
#define CA9_K (CA1_K + SLED1734X_FRAME_OFFSET)
#define CA9_L (CA1_L + SLED1734X_FRAME_OFFSET)
#define CA9_M (CA1_M + SLED1734X_FRAME_OFFSET)
#define CA9_N (CA1_N + SLED1734X_FRAME_OFFSET)
#define CA9_O (CA1_O + SLED1734X_FRAME_OFFSET)
#define CA9_P (CA1_P + SLED1734X_FRAME_OFFSET)

#define CB1_A (CA2_A + SLED1734X_FRAME_OFFSET)
#define CB1_B (CA2_B + SLED1734X_FRAME_OFFSET)
#define CB1_C (CA2_C + SLED1734X_FRAME_OFFSET)
#define CB1_D (CA2_D + SLED1734X_FRAME_OFFSET)
#define CB1_E (CA2_E + SLED1734X_FRAME_OFFSET)
#define CB1_F (CA2_F + SLED1734X_FRAME_OFFSET)
#define CB1_G (CA2_G + SLED1734X_FRAME_OFFSET)
#define CB1_H (CA2_H + SLED1734X_FRAME_OFFSET)
#define CB1_I (CA2_I + SLED1734X_FRAME_OFFSET)
#define CB1_J (CA2_J + SLED1734X_FRAME_OFFSET)
#define CB1_K (CA2_K + SLED1734X_FRAME_OFFSET)
#define CB1_L (CA2_L + SLED1734X_FRAME_OFFSET)
#define CB1_M (CA2_M + SLED1734X_FRAME_OFFSET)
#define CB1_N (CA2_N + SLED1734X_FRAME_OFFSET)
#define CB1_O (CA2_O + SLED1734X_FRAME_OFFSET)
#define CB1_P (CA2_P + SLED1734X_FRAME_OFFSET)

#define CB2_A (CA3_A + SLED1734X_FRAME_OFFSET)
#define CB2_B (CA3_B + SLED1734X_FRAME_OFFSET)
#define CB2_C (CA3_C + SLED1734X_FRAME_OFFSET)
#define CB2_D (CA3_D + SLED1734X_FRAME_OFFSET)
#define CB2_E (CA3_E + SLED1734X_FRAME_OFFSET)
#define CB2_F (CA3_F + SLED1734X_FRAME_OFFSET)
#define CB2_G (CA3_G + SLED1734X_FRAME_OFFSET)
#define CB2_H (CA3_H + SLED1734X_FRAME_OFFSET)
#define CB2_I (CA3_I + SLED1734X_FRAME_OFFSET)
#define CB2_J (CA3_J + SLED1734X_FRAME_OFFSET)
#define CB2_K (CA3_K + SLED1734X_FRAME_OFFSET)
#define CB2_L (CA3_L + SLED1734X_FRAME_OFFSET)
#define CB2_M (CA3_M + SLED1734X_FRAME_OFFSET)
#define CB2_N (CA3_N + SLED1734X_FRAME_OFFSET)
#define CB2_O (CA3_O + SLED1734X_FRAME_OFFSET)
#define CB2_P (CA3_P + SLED1734X_FRAME_OFFSET)

#define CB3_A (CA4_A + SLED1734X_FRAME_OFFSET)
#define CB3_B (CA4_B + SLED1734X_FRAME_OFFSET)
#define CB3_C (CA4_C + SLED1734X_FRAME_OFFSET)
#define CB3_D (CA4_D + SLED1734X_FRAME_OFFSET)
#define CB3_E (CA4_E + SLED1734X_FRAME_OFFSET)
#define CB3_F (CA4_F + SLED1734X_FRAME_OFFSET)
#define CB3_G (CA4_G + SLED1734X_FRAME_OFFSET)
#define CB3_H (CA4_H + SLED1734X_FRAME_OFFSET)
#define CB3_I (CA4_I + SLED1734X_FRAME_OFFSET)
#define CB3_J (CA4_J + SLED1734X_FRAME_OFFSET)
#define CB3_K (CA4_K + SLED1734X_FRAME_OFFSET)
#define CB3_L (CA4_L + SLED1734X_FRAME_OFFSET)
#define CB3_M (CA4_M + SLED1734X_FRAME_OFFSET)
#define CB3_N (CA4_N + SLED1734X_FRAME_OFFSET)
#define CB3_O (CA4_O + SLED1734X_FRAME_OFFSET)
#define CB3_P (CA4_P + SLED1734X_FRAME_OFFSET)

#define CB4_A (CA5_A + SLED1734X_FRAME_OFFSET)
#define CB4_B (CA5_B + SLED1734X_FRAME_OFFSET)
#define CB4_C (CA5_C + SLED1734X_FRAME_OFFSET)
#define CB4_D (CA5_D + SLED1734X_FRAME_OFFSET)
#define CB4_E (CA5_E + SLED1734X_FRAME_OFFSET)
#define CB4_F (CA5_F + SLED1734X_FRAME_OFFSET)
#define CB4_G (CA5_G + SLED1734X_FRAME_OFFSET)
#define CB4_H (CA5_H + SLED1734X_FRAME_OFFSET)
#define CB4_I (CA5_I + SLED1734X_FRAME_OFFSET)
#define CB4_J (CA5_J + SLED1734X_FRAME_OFFSET)
#define CB4_K (CA5_K + SLED1734X_FRAME_OFFSET)
#define CB4_L (CA5_L + SLED1734X_FRAME_OFFSET)
#define CB4_M (CA5_M + SLED1734X_FRAME_OFFSET)
#define CB4_N (CA5_N + SLED1734X_FRAME_OFFSET)
#define CB4_O (CA5_O + SLED1734X_FRAME_OFFSET)
#define CB4_P (CA5_P + SLED1734X_FRAME_OFFSET)

#define CB5_A (CA6_A + SLED1734X_FRAME_OFFSET)
#define CB5_B (CA6_B + SLED1734X_FRAME_OFFSET)
#define CB5_C (CA6_C + SLED1734X_FRAME_OFFSET)
#define CB5_D (CA6_D + SLED1734X_FRAME_OFFSET)
#define CB5_E (CA6_E + SLED1734X_FRAME_OFFSET)
#define CB5_F (CA6_F + SLED1734X_FRAME_OFFSET)
#define CB5_G (CA6_G + SLED1734X_FRAME_OFFSET)
#define CB5_H (CA6_H + SLED1734X_FRAME_OFFSET)
#define CB5_I (CA6_I + SLED1734X_FRAME_OFFSET)
#define CB5_J (CA6_J + SLED1734X_FRAME_OFFSET)
#define CB5_K (CA6_K + SLED1734X_FRAME_OFFSET)
#define CB5_L (CA6_L + SLED1734X_FRAME_OFFSET)
#define CB5_M (CA6_M + SLED1734X_FRAME_OFFSET)
#define CB5_N (CA6_N + SLED1734X_FRAME_OFFSET)
#define CB5_O (CA6_O + SLED1734X_FRAME_OFFSET)
#define CB5_P (CA6_P + SLED1734X_FRAME_OFFSET)

#define CB6_A (CA7_A + SLED1734X_FRAME_OFFSET)
#define CB6_B (CA7_B + SLED1734X_FRAME_OFFSET)
#define CB6_C (CA7_C + SLED1734X_FRAME_OFFSET)
#define CB6_D (CA7_D + SLED1734X_FRAME_OFFSET)
#define CB6_E (CA7_E + SLED1734X_FRAME_OFFSET)
#define CB6_F (CA7_F + SLED1734X_FRAME_OFFSET)
#define CB6_G (CA7_G + SLED1734X_FRAME_OFFSET)
#define CB6_H (CA7_H + SLED1734X_FRAME_OFFSET)
#define CB6_I (CA7_I + SLED1734X_FRAME_OFFSET)
#define CB6_J (CA7_J + SLED1734X_FRAME_OFFSET)
#define CB6_K (CA7_K + SLED1734X_FRAME_OFFSET)
#define CB6_L (CA7_L + SLED1734X_FRAME_OFFSET)
#define CB6_M (CA7_M + SLED1734X_FRAME_OFFSET)
#define CB6_N (CA7_N + SLED1734X_FRAME_OFFSET)
#define CB6_O (CA7_O + SLED1734X_FRAME_OFFSET)
#define CB6_P (CA7_P + SLED1734X_FRAME_OFFSET)

#define CB7_A (CA8_A + SLED1734X_FRAME_OFFSET)
#define CB7_B (CA8_B + SLED1734X_FRAME_OFFSET)
#define CB7_C (CA8_C + SLED1734X_FRAME_OFFSET)
#define CB7_D (CA8_D + SLED1734X_FRAME_OFFSET)
#define CB7_E (CA8_E + SLED1734X_FRAME_OFFSET)
#define CB7_F (CA8_F + SLED1734X_FRAME_OFFSET)
#define CB7_G (CA8_G + SLED1734X_FRAME_OFFSET)
#define CB7_H (CA8_H + SLED1734X_FRAME_OFFSET)
#define CB7_I (CA8_I + SLED1734X_FRAME_OFFSET)
#define CB7_J (CA8_J + SLED1734X_FRAME_OFFSET)
#define CB7_K (CA8_K + SLED1734X_FRAME_OFFSET)
#define CB7_L (CA8_L + SLED1734X_FRAME_OFFSET)
#define CB7_M (CA8_M + SLED1734X_FRAME_OFFSET)
#define CB7_N (CA8_N + SLED1734X_FRAME_OFFSET)
#define CB7_O (CA8_O + SLED1734X_FRAME_OFFSET)
#define CB7_P (CA8_P + SLED1734X_FRAME_OFFSET)
