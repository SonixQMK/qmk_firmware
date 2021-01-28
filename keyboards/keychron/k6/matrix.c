/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

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

Ported to QMK by Stephen Peery <https://github.com/smp4488/>
*/

#include <stdint.h>
#include <stdbool.h>
#include <SN32F240B.h>
#include "ch.h"
#include "hal.h"
#include "CT16.h"

#include "wait.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"

static const pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
static const pin_t led_row_pins[LED_MATRIX_ROWS_HW] = LED_MATRIX_ROW_PINS;
static uint16_t row_ofsts[LED_MATRIX_ROWS];

matrix_row_t raw_matrix[MATRIX_ROWS]; //raw values
matrix_row_t last_matrix[MATRIX_ROWS] = {0};  // raw values
matrix_row_t matrix[MATRIX_ROWS]; //debounced values

static uint8_t current_led_row =0;

extern volatile LED_TYPE led_state[DRIVER_LED_TOTAL];

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

inline matrix_row_t matrix_get_row(uint8_t row) { return matrix[row]; }

void sn32_wait_x10us(uint32_t n) {
    n*=21;
    while(n--);
}

void matrix_print(void) {}

static void init_pins(void) {

    //  Unselect ROWs
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        setPinOutput(row_pins[x]);
        writePinHigh(row_pins[x]);
    }

    // Unselect COLs
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        setPinInput(col_pins[x]);
        writePinHigh(col_pins[x]);
    }

   for (uint8_t x = 0; x < LED_MATRIX_ROWS_HW; x++) {
        setPinOutput(led_row_pins[x]);
        writePinLow(led_row_pins[x]);
   }
}

static void unselect_led_rows(void) {
	for (uint8_t x = 0; x < LED_MATRIX_ROWS_HW; x++) {
        writePinLow(led_row_pins[x]);
   }
}

static void disable_rgb_matrix(void) {
    // Disable LED row output
    unselect_led_rows();
    // Disable PWM outputs on column pins
    // Enable GPIO control on colun pins
    SN_CT16B1->PWMIOENB = 0;
    // Clear match interrupt status
    SN_CT16B1->IC = mskCT16_MR24IC;
    // Reset the counter
    SN_CT16B1->TMRCTRL = CT16_CRST;
    // Disable the LED interrupts
    CT16B1_NvicDisable();
}

static void enable_rgb_matrix(void) {
   // Enable PWM outputs on column pins
    SN_CT16B1->PWMIOENB   = (mskCT16_PWM8EN_EN  \
                            |mskCT16_PWM9EN_EN  \
                            |mskCT16_PWM10EN_EN \
                            |mskCT16_PWM11EN_EN \
                            |mskCT16_PWM12EN_EN \
                            |mskCT16_PWM13EN_EN \
                            |mskCT16_PWM14EN_EN \
                            |mskCT16_PWM15EN_EN \
                            |mskCT16_PWM16EN_EN \
                            |mskCT16_PWM17EN_EN \
                            |mskCT16_PWM18EN_EN \
                            |mskCT16_PWM19EN_EN \
                            |mskCT16_PWM20EN_EN \
                            |mskCT16_PWM21EN_EN \
                            |mskCT16_PWM22EN_EN \
                            |mskCT16_PWM23EN_EN);

    //Set CT16B1 as the up-counting mode.
    SN_CT16B1->TMRCTRL = (mskCT16_CRST);
    // Wait until timer reset done.
    while (SN_CT16B1->TMRCTRL & mskCT16_CRST);
    // Let TC start counting.
    SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;
    // Enable the LED interrupts
    CT16B1_NvicEnable();
}
static void init_rgb_matrix(void) {
    // Calculate the row offsets
    for (uint8_t i = 0; i < LED_MATRIX_ROWS; i++) {
        row_ofsts[i] = i * LED_MATRIX_COLS;
    }
    // Enable Timer Clock
    SN_SYS1->AHBCLKEN_b.CT16B1CLKEN = 1;
    // PFPA - Map the PWM outputs to their PWM A pins
    // PWM0-2 is B for the k4
    SN_PFPA->CT16B1 = 0x00000000;

    // Enable PWM function, IOs and select the PWM modes
    // Enable PWM0-PWM2, PWM8-23
    SN_CT16B1->PWMENB   =   (mskCT16_PWM8EN_EN  \
                            |mskCT16_PWM9EN_EN  \
                            |mskCT16_PWM10EN_EN \
                            |mskCT16_PWM11EN_EN \
                            |mskCT16_PWM12EN_EN \
                            |mskCT16_PWM13EN_EN \
                            |mskCT16_PWM14EN_EN \
                            |mskCT16_PWM15EN_EN \
                            |mskCT16_PWM16EN_EN \
                            |mskCT16_PWM17EN_EN \
                            |mskCT16_PWM18EN_EN \
                            |mskCT16_PWM19EN_EN \
                            |mskCT16_PWM20EN_EN \
                            |mskCT16_PWM21EN_EN \
                            |mskCT16_PWM22EN_EN \
                            |mskCT16_PWM23EN_EN);

    // Set match interrupts and TC reset
    SN_CT16B1->MCTRL3 = (mskCT16_MR24IE_EN);
    SN_CT16B1->MCTRL3_b.MR24RST = 1;

    // COL match register
    SN_CT16B1->MR24 = 0xFF;

    // Set prescale value
    SN_CT16B1->PRE = 0x03;

	enable_rgb_matrix();
}

static bool read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Store last value of row prior to reading
    matrix_row_t last_row_value = current_matrix[current_row];
    // Clear data in matrix row
    current_matrix[current_row] = 0;
    // Enable current matrix row
    writePinLow(row_pins[current_row]);
    // Wait to stabilize
    sn32_wait_x10us(2);

    // Read the key matrix
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++) {
    	disable_rgb_matrix();
        // Enable the column
        writePinHigh(col_pins[col_index]);
        // Check col pin state
        if (readPin(col_pins[col_index]) == 0) {
            // Pin LO, set col bit
            current_matrix[current_row] |= (MATRIX_ROW_SHIFTER << col_index);
        } else {
            // Pin HI, clear col bit
            current_matrix[current_row] &= ~(MATRIX_ROW_SHIFTER << col_index);
        }

        // Disable the column
        writePinLow(col_pins[col_index]);
        enable_rgb_matrix();
    }

    // Disable current matrix row
    writePinHigh(row_pins[current_row]);
    return (last_row_value != current_matrix[current_row]);
}

void matrix_init(void) {
    // Enable GPIO Clock
    SN_SYS1->AHBCLKEN_b.P0CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.P1CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.P2CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.P3CLKEN = 1;	
    // initialize key pins
    init_pins();

    // initialize matrix state: all keys off
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        raw_matrix[i] = 0;
        matrix[i]     = 0;
    }

    debounce_init(MATRIX_ROWS);

    matrix_init_quantum();
    // initialize the rgb matrix
    init_rgb_matrix();
}

uint8_t matrix_scan(void) {
    bool changed = false;

    // Set row, read cols
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        changed |= read_cols_on_row(raw_matrix, current_row);

    }
    debounce(raw_matrix, matrix, MATRIX_ROWS, changed);
    matrix_scan_quantum();
    return (uint8_t)changed;

}

uint8_t hw_row_to_matrix_row[15] = { 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 0 };
/**
 * @brief   CT16B1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SN32_CT16B1_HANDLER) {

    OSAL_IRQ_PROLOGUE();
    // Turn the selected LED row on
    // This is to minimize dead time
    writePinHigh(led_row_pins[current_led_row]);    
    // Turn the selected LED row off
    writePinLow(led_row_pins[current_led_row]);
    // Turn the next row on
    current_led_row = (current_led_row + 1) % LED_MATRIX_ROWS_HW;
    uint8_t row_idx = hw_row_to_matrix_row[current_led_row];
    uint16_t row_ofst = row_ofsts[row_idx];

    if(current_led_row % 3 == 0)
    {
        SN_CT16B1->MR8  = led_state[row_ofst + 0 ].b | 1;
        SN_CT16B1->MR9  = led_state[row_ofst + 1 ].b | 1;
        SN_CT16B1->MR10 = led_state[row_ofst + 2 ].b | 1;
        SN_CT16B1->MR11 = led_state[row_ofst + 3 ].b | 1;
        SN_CT16B1->MR12 = led_state[row_ofst + 4 ].b | 1;
        SN_CT16B1->MR13 = led_state[row_ofst + 5 ].b | 1;
        SN_CT16B1->MR14 = led_state[row_ofst + 6 ].b | 1;
        SN_CT16B1->MR15 = led_state[row_ofst + 7 ].b | 1;
        SN_CT16B1->MR16 = led_state[row_ofst + 8 ].b | 1;
        SN_CT16B1->MR17 = led_state[row_ofst + 9 ].b | 1;
        SN_CT16B1->MR18 = led_state[row_ofst + 10].b | 1;
        SN_CT16B1->MR19 = led_state[row_ofst + 11].b | 1;
        SN_CT16B1->MR20 = led_state[row_ofst + 12].b | 1;
        SN_CT16B1->MR21 = led_state[row_ofst + 13].b | 1;
        SN_CT16B1->MR22 = led_state[row_ofst + 14].b | 1;
        SN_CT16B1->MR23 = led_state[row_ofst + 15].b | 1;
    }

    if(current_led_row % 3 == 1)
    {
        SN_CT16B1->MR8  = led_state[row_ofst + 0 ].g | 1;
        SN_CT16B1->MR9  = led_state[row_ofst + 1 ].g | 1;
        SN_CT16B1->MR10 = led_state[row_ofst + 2 ].g | 1;
        SN_CT16B1->MR11 = led_state[row_ofst + 3 ].g | 1;
        SN_CT16B1->MR12 = led_state[row_ofst + 4 ].g | 1;
        SN_CT16B1->MR13 = led_state[row_ofst + 5 ].g | 1;
        SN_CT16B1->MR14 = led_state[row_ofst + 6 ].g | 1;
        SN_CT16B1->MR15 = led_state[row_ofst + 7 ].g | 1;
        SN_CT16B1->MR16 = led_state[row_ofst + 8 ].g | 1;
        SN_CT16B1->MR17 = led_state[row_ofst + 9 ].g | 1;
        SN_CT16B1->MR18 = led_state[row_ofst + 10].g | 1;
        SN_CT16B1->MR19 = led_state[row_ofst + 11].g | 1;
        SN_CT16B1->MR20 = led_state[row_ofst + 12].g | 1;
        SN_CT16B1->MR21 = led_state[row_ofst + 13].g | 1;
        SN_CT16B1->MR22 = led_state[row_ofst + 14].g | 1;
        SN_CT16B1->MR23 = led_state[row_ofst + 15].g | 1;
    }
    if(current_led_row % 3 == 2)
    {
        SN_CT16B1->MR8  = led_state[row_ofst + 0 ].r | 1;
        SN_CT16B1->MR9  = led_state[row_ofst + 1 ].r | 1;
        SN_CT16B1->MR10 = led_state[row_ofst + 2 ].r | 1;
        SN_CT16B1->MR11 = led_state[row_ofst + 3 ].r | 1;
        SN_CT16B1->MR12 = led_state[row_ofst + 4 ].r | 1;
        SN_CT16B1->MR13 = led_state[row_ofst + 5 ].r | 1;
        SN_CT16B1->MR14 = led_state[row_ofst + 6 ].r | 1;
        SN_CT16B1->MR15 = led_state[row_ofst + 7 ].r | 1;
        SN_CT16B1->MR16 = led_state[row_ofst + 8 ].r | 1;
        SN_CT16B1->MR17 = led_state[row_ofst + 9 ].r | 1;
        SN_CT16B1->MR18 = led_state[row_ofst + 10].r | 1;
        SN_CT16B1->MR19 = led_state[row_ofst + 11].r | 1;
        SN_CT16B1->MR20 = led_state[row_ofst + 12].r | 1;
        SN_CT16B1->MR21 = led_state[row_ofst + 13].r | 1;
        SN_CT16B1->MR22 = led_state[row_ofst + 14].r | 1;
        SN_CT16B1->MR23 = led_state[row_ofst + 15].r | 1;
    }

    writePinHigh(led_row_pins[current_led_row]);
    CT16B1_IRQHandler();
    OSAL_IRQ_EPILOGUE();
}

void HardFault_Handler(void) {
    NVIC_SystemReset();
}