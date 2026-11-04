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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SN32F240.h>
#include "ch.h"
#include "hal.h"
#include "color.h"
#include "wait.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"
#include "rgb_matrix.h"
#include "vs11k09a.h"

// CT16/CT32 bitmasks
#define mskCT16_CEN_EN (1 << 0)
#define mskCT16_CRST (1 << 1)
#define mskCT16_MR3IC (1 << 3)
#define mskCT16_MR3IE_EN (1 << 9)
#define mskCT16_MR3STOP_EN (1 << 11)
#define mskCT16_PWM0EN_EN (1 << 0)
#define mskCT16_PWM1EN_EN (1 << 1)
#define mskCT16_PWM2EN_EN (1 << 2)
#define mskCT16_PWM0IOEN_EN (1 << 20)
#define mskCT16_PWM1IOEN_EN (1 << 21)
#define mskCT16_PWM2IOEN_EN (1 << 22)

#define mskCT32_CEN_EN (1 << 0)
#define mskCT32_CRST (1 << 1)
#define mskCT32_PWM0EN_EN (1 << 0)
#define mskCT32_PWM1EN_EN (1 << 1)
#define mskCT32_PWM2EN_EN (1 << 2)
#define mskCT32_PWM3EN_EN (1 << 3)
#define mskCT32_PWM0IOEN_EN (1 << 20)
#define mskCT32_PWM1IOEN_EN (1 << 21)
#define mskCT32_PWM2IOEN_EN (1 << 22)
#define mskCT32_PWM3IOEN_EN (1 << 23)

static const pin_t row_pins[MATRIX_ROWS]                     = MATRIX_ROW_PINS;
static const pin_t col_pins[MATRIX_COLS]                     = MATRIX_COL_PINS;
static const pin_t led_row_pins[VS11K09A_RGB_MATRIX_ROWS_HW] = VS11K09A_RGB_MATRIX_ROW_PINS;
static uint16_t    row_ofsts[VS11K09A_RGB_MATRIX_ROWS];

static volatile RGB led_state[VS11K09A_RGB_MATRIX_ROWS * VS11K09A_RGB_MATRIX_COLS];
static uint8_t      led_pos[RGB_MATRIX_LED_COUNT];

static matrix_row_t  shared_matrix[MATRIX_ROWS];
static volatile bool matrix_scanned = false;
static uint8_t       current_row    = 0;

__attribute__((weak)) void board_init(void) {
    SN_SYS0->EXRSTCTRL_b.RESETDIS = 1; // Disable RESET
    SN_SYS0->SWDCTRL_b.SWDDIS     = 1; // Disable SWD so A14/A15 function as normal GPIO
    sys1EnableUSB();
}

__attribute__((weak)) bool bootmagic_should_reset(void) {
    return false;
}

static void init_pins(void) {
    // Unselect ROWs (enable pull-ups so active-low reads work properly)
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        setPinInputHigh(row_pins[x]);
    }

    // Unselect COLs
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        setPinOutput(col_pins[x]);
        writePinHigh(col_pins[x]);
    }

    // Turn LED rows off
    for (uint8_t x = 0; x < VS11K09A_RGB_MATRIX_ROWS_HW; x++) {
        setPinOutput(led_row_pins[x]);
        writePinHigh(led_row_pins[x]);
    }
}

void matrix_init_custom(void) {
    init_pins();

    for (uint8_t i = 0; i < VS11K09A_RGB_MATRIX_ROWS; i++) {
        row_ofsts[i] = i * VS11K09A_RGB_MATRIX_COLS;
    }

    // Enable Timer Clocks
    SN_SYS1->AHBCLKEN_b.CT16B0CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.CT16B1CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.CT16B2CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.CT32B0CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.CT32B1CLKEN = 1;
    SN_SYS1->AHBCLKEN_b.CT32B2CLKEN = 1;

    // Set Timer Clock Prescalers
    SN_SYS1->APBCP0_b.CT16B0PRE = 0x00;
    SN_SYS1->APBCP0_b.CT16B1PRE = 0x00;
    SN_SYS1->APBCP1_b.CT16B2PRE = 0x00;
    SN_SYS1->APBCP0_b.CT32B0PRE = 0x00;
    SN_SYS1->APBCP0_b.CT32B1PRE = 0x00;
    SN_SYS1->APBCP0_b.CT32B2PRE = 0x00;

    // PFPA Timer Mapping
    SN_PFPA->CT16B0 = 0x00000;
    SN_PFPA->CT16B1 = 0x02330;
    SN_PFPA->CT16B2 = 0x02010;
    SN_PFPA->CT32B0 = 0x00410;
    SN_PFPA->CT32B1 = 0x21300;
    SN_PFPA->CT32B2 = 0x00030;

    // Set match interrupts and TC reset for CT16B0, which is used to generate interrupt
    SN_CT16B0->MCTRL = (mskCT16_MR3IE_EN | mskCT16_MR3STOP_EN);

    // COL match register
    SN_CT16B0->MR3 = 0xFF;

    // Set prescale value
    SN_CT16B0->PRE = 0x28;
    SN_CT16B1->PRE = 0x28;
    SN_CT16B2->PRE = 0x28;
    SN_CT32B0->PRE = 0x28;
    SN_CT32B1->PRE = 0x28;
    SN_CT32B2->PRE = 0x28;

    // Enable PWM on all timers
    SN_CT16B0->PWMCTRL = (mskCT16_PWM0EN_EN | mskCT16_PWM1EN_EN | mskCT16_PWM2EN_EN);
    SN_CT16B1->PWMCTRL = (mskCT16_PWM0EN_EN | mskCT16_PWM1EN_EN | mskCT16_PWM2EN_EN);
    SN_CT16B2->PWMCTRL = (mskCT32_PWM0EN_EN | mskCT32_PWM1EN_EN | mskCT32_PWM2EN_EN | mskCT32_PWM3EN_EN);
    SN_CT32B0->PWMCTRL = (mskCT32_PWM0EN_EN | mskCT32_PWM1EN_EN | mskCT32_PWM2EN_EN | mskCT32_PWM3EN_EN);
    SN_CT32B1->PWMCTRL = (mskCT32_PWM0EN_EN | mskCT32_PWM1EN_EN | mskCT32_PWM2EN_EN | mskCT32_PWM3EN_EN);
    SN_CT32B2->PWMCTRL = (mskCT32_PWM0EN_EN | mskCT32_PWM1EN_EN | mskCT32_PWM2EN_EN | mskCT32_PWM3EN_EN);

    // Enable IO
    SN_CT16B0->PWMCTRL |= (mskCT16_PWM0IOEN_EN | mskCT16_PWM1IOEN_EN | mskCT16_PWM2IOEN_EN);
    SN_CT16B1->PWMCTRL |= (mskCT16_PWM0IOEN_EN | mskCT16_PWM1IOEN_EN | mskCT16_PWM2IOEN_EN);
    SN_CT16B2->PWMCTRL |= (mskCT16_PWM0IOEN_EN | mskCT16_PWM1IOEN_EN | mskCT16_PWM2IOEN_EN);
    SN_CT32B0->PWMCTRL |= (mskCT32_PWM0IOEN_EN | mskCT32_PWM1IOEN_EN | mskCT32_PWM2IOEN_EN | mskCT32_PWM3IOEN_EN);
    SN_CT32B1->PWMCTRL |= (mskCT32_PWM0IOEN_EN | mskCT32_PWM1IOEN_EN | mskCT32_PWM2IOEN_EN | mskCT32_PWM3IOEN_EN);
    SN_CT32B2->PWMCTRL |= (mskCT32_PWM0IOEN_EN | mskCT32_PWM1IOEN_EN | mskCT32_PWM2IOEN_EN | mskCT32_PWM3IOEN_EN);

    // Clear match registers
    SN_CT16B0->MR0 = 0x0;
    SN_CT16B0->MR1 = 0x0;
    SN_CT16B0->MR2 = 0x0;
    SN_CT16B1->MR0 = 0x0;
    SN_CT16B1->MR1 = 0x0;
    SN_CT16B1->MR2 = 0x0;
    SN_CT16B1->MR3 = 0x0;
    SN_CT16B2->MR0 = 0x0;
    SN_CT16B2->MR1 = 0x0;
    SN_CT16B2->MR2 = 0x0;
    SN_CT16B2->MR3 = 0x0;
    SN_CT32B0->MR0 = 0x0;
    SN_CT32B0->MR1 = 0x0;
    SN_CT32B0->MR2 = 0x0;
    SN_CT32B0->MR3 = 0x0;
    SN_CT32B1->MR0 = 0x0;
    SN_CT32B1->MR1 = 0x0;
    SN_CT32B1->MR2 = 0x0;
    SN_CT32B1->MR3 = 0x0;
    SN_CT32B2->MR0 = 0x0;
    SN_CT32B2->MR1 = 0x0;
    SN_CT32B2->MR2 = 0x0;
    SN_CT32B2->MR3 = 0x0;

    // Set timers to the up-counting mode.
    SN_CT16B0->TMRCTRL = (mskCT16_CRST);
    SN_CT16B1->TMRCTRL = (mskCT16_CRST);
    SN_CT16B2->TMRCTRL = (mskCT16_CRST);
    SN_CT32B0->TMRCTRL = (mskCT32_CRST);
    SN_CT32B1->TMRCTRL = (mskCT32_CRST);
    SN_CT32B2->TMRCTRL = (mskCT32_CRST);

    // Wait until timer reset done.
    while ((SN_CT16B0->TMRCTRL & mskCT16_CRST) || (SN_CT16B1->TMRCTRL & mskCT16_CRST) || (SN_CT16B2->TMRCTRL & mskCT16_CRST) || (SN_CT32B0->TMRCTRL & mskCT32_CRST) || (SN_CT32B1->TMRCTRL & mskCT32_CRST) || (SN_CT32B2->TMRCTRL & mskCT32_CRST))
        ;

    // Let TC start counting. Start CT16B0 last because it generates our interrupt
    SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;
    SN_CT16B2->TMRCTRL |= mskCT16_CEN_EN;
    SN_CT32B0->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT32B1->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT32B2->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;

    NVIC_ClearPendingIRQ(CT16B0_IRQn);
    nvicEnableVector(CT16B0_IRQn, 2);
}

bool matrix_can_read(void) {
    return matrix_scanned;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (!matrix_scanned) {
        return false;
    }

    bool changed = memcmp(current_matrix, shared_matrix, sizeof(shared_matrix)) != 0;
    if (changed) {
        memcpy(current_matrix, shared_matrix, sizeof(shared_matrix));
    }

    matrix_scanned = false;
    return changed;
}

OSAL_IRQ_HANDLER(SN32_CT16B0_HANDLER) {
    OSAL_IRQ_PROLOGUE();

    // Disable PWM outputs on column pins
    SN_CT16B0->PWMCTRL = 0;
    SN_CT16B1->PWMCTRL = 0;
    SN_CT16B2->PWMCTRL = 0;
    SN_CT32B0->PWMCTRL = 0;
    SN_CT32B1->PWMCTRL = 0;
    SN_CT32B2->PWMCTRL = 0;

    SN_CT16B0->IC = mskCT16_MR3IC; // Clear match interrupt status

    // Turn the selected row off
    writePinLow(led_row_pins[current_row]);

    // Turn the next row on
    current_row = (current_row + 1) % VS11K09A_RGB_MATRIX_ROWS_HW;

    // Read the key matrix
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++) {
        // Enable the column
        writePinLow(col_pins[col_index]);

        for (uint8_t row_index = 0; row_index < MATRIX_ROWS; row_index++) {
            // Check row pin state
            if (readPin(row_pins[row_index]) == 0) {
                // Pin LO, set col bit
                shared_matrix[row_index] |= (MATRIX_ROW_SHIFTER << col_index);
            } else {
                // Pin HI, clear col bit
                shared_matrix[row_index] &= ~(MATRIX_ROW_SHIFTER << col_index);
            }
        }

        // Disable the column
        for (uint8_t delay_idx = 0; delay_idx < 20; delay_idx++) {
            writePinHigh(col_pins[col_index]);
        }
    }
    matrix_scanned = true;

    uint8_t  row_idx  = current_row / VS11K09A_RGB_MATRIX_ROW_CHANNELS;
    uint16_t row_ofst = row_ofsts[row_idx];

    // Set timers to the up-counting mode.
    SN_CT16B0->TMRCTRL = (mskCT16_CRST);
    SN_CT16B1->TMRCTRL = (mskCT16_CRST);
    SN_CT16B2->TMRCTRL = (mskCT16_CRST);
    SN_CT32B0->TMRCTRL = (mskCT32_CRST);
    SN_CT32B1->TMRCTRL = (mskCT32_CRST);
    SN_CT32B2->TMRCTRL = (mskCT32_CRST);

    // Wait until timer reset done.
    while ((SN_CT16B0->TMRCTRL & mskCT16_CRST) || (SN_CT16B1->TMRCTRL & mskCT16_CRST) || (SN_CT16B2->TMRCTRL & mskCT16_CRST) || (SN_CT32B0->TMRCTRL & mskCT32_CRST) || (SN_CT32B1->TMRCTRL & mskCT32_CRST) || (SN_CT32B2->TMRCTRL & mskCT32_CRST))
        ;

    if (current_row % 3 == 0) {
        SN_CT16B0->MR0 = led_state[row_ofst + 0].r;
        SN_CT16B0->MR1 = led_state[row_ofst + 1].r;
        SN_CT16B2->MR0 = led_state[row_ofst + 2].r;
        SN_CT32B2->MR2 = led_state[row_ofst + 3].r;
        SN_CT32B0->MR0 = led_state[row_ofst + 4].r;
        SN_CT32B1->MR2 = led_state[row_ofst + 5].r;
        SN_CT32B2->MR3 = led_state[row_ofst + 6].r;
        SN_CT32B1->MR3 = led_state[row_ofst + 7].r;
        SN_CT32B2->MR1 = led_state[row_ofst + 8].r;
        SN_CT16B1->MR2 = led_state[row_ofst + 9].r;
        SN_CT16B2->MR2 = led_state[row_ofst + 10].r;
        SN_CT32B2->MR0 = led_state[row_ofst + 11].r;
        SN_CT32B0->MR3 = led_state[row_ofst + 12].r;
        SN_CT16B1->MR1 = led_state[row_ofst + 13].r;
        SN_CT16B2->MR1 = led_state[row_ofst + 14].r;
        SN_CT32B1->MR1 = led_state[row_ofst + 15].r;
        SN_CT32B0->MR1 = led_state[row_ofst + 16].r;
        SN_CT16B0->MR2 = led_state[row_ofst + 17].r;
        SN_CT32B1->MR0 = led_state[row_ofst + 18].r;
        SN_CT32B0->MR2 = led_state[row_ofst + 19].r;
        SN_CT16B1->MR0 = led_state[row_ofst + 20].r;
    }

    if (current_row % 3 == 1) {
        SN_CT16B0->MR0 = led_state[row_ofst + 0].g;
        SN_CT16B0->MR1 = led_state[row_ofst + 1].g;
        SN_CT16B2->MR0 = led_state[row_ofst + 2].g;
        SN_CT32B2->MR2 = led_state[row_ofst + 3].g;
        SN_CT32B0->MR0 = led_state[row_ofst + 4].g;
        SN_CT32B1->MR2 = led_state[row_ofst + 5].g;
        SN_CT32B2->MR3 = led_state[row_ofst + 6].g;
        SN_CT32B1->MR3 = led_state[row_ofst + 7].g;
        SN_CT32B2->MR1 = led_state[row_ofst + 8].g;
        SN_CT16B1->MR2 = led_state[row_ofst + 9].g;
        SN_CT16B2->MR2 = led_state[row_ofst + 10].g;
        SN_CT32B2->MR0 = led_state[row_ofst + 11].g;
        SN_CT32B0->MR3 = led_state[row_ofst + 12].g;
        SN_CT16B1->MR1 = led_state[row_ofst + 13].g;
        SN_CT16B2->MR1 = led_state[row_ofst + 14].g;
        SN_CT32B1->MR1 = led_state[row_ofst + 15].g;
        SN_CT32B0->MR1 = led_state[row_ofst + 16].g;
        SN_CT16B0->MR2 = led_state[row_ofst + 17].g;
        SN_CT32B1->MR0 = led_state[row_ofst + 18].g;
        SN_CT32B0->MR2 = led_state[row_ofst + 19].g;
        SN_CT16B1->MR0 = led_state[row_ofst + 20].g;
    }

    if (current_row % 3 == 2) {
        SN_CT16B0->MR0 = led_state[row_ofst + 0].b;
        SN_CT16B0->MR1 = led_state[row_ofst + 1].b;
        SN_CT16B2->MR0 = led_state[row_ofst + 2].b;
        SN_CT32B2->MR2 = led_state[row_ofst + 3].b;
        SN_CT32B0->MR0 = led_state[row_ofst + 4].b;
        SN_CT32B1->MR2 = led_state[row_ofst + 5].b;
        SN_CT32B2->MR3 = led_state[row_ofst + 6].b;
        SN_CT32B1->MR3 = led_state[row_ofst + 7].b;
        SN_CT32B2->MR1 = led_state[row_ofst + 8].b;
        SN_CT16B1->MR2 = led_state[row_ofst + 9].b;
        SN_CT16B2->MR2 = led_state[row_ofst + 10].b;
        SN_CT32B2->MR0 = led_state[row_ofst + 11].b;
        SN_CT32B0->MR3 = led_state[row_ofst + 12].b;
        SN_CT16B1->MR1 = led_state[row_ofst + 13].b;
        SN_CT16B2->MR1 = led_state[row_ofst + 14].b;
        SN_CT32B1->MR1 = led_state[row_ofst + 15].b;
        SN_CT32B0->MR1 = led_state[row_ofst + 16].b;
        SN_CT16B0->MR2 = led_state[row_ofst + 17].b;
        SN_CT32B1->MR0 = led_state[row_ofst + 18].b;
        SN_CT32B0->MR2 = led_state[row_ofst + 19].b;
        SN_CT16B1->MR0 = led_state[row_ofst + 20].b;
    }

    // Enable PWM on all timers
    if (SN_CT16B0->MR0 > 0) {
        SN_CT16B0->PWMCTRL |= (mskCT16_PWM0EN_EN | mskCT16_PWM0IOEN_EN);
    }
    if (SN_CT16B0->MR1 > 0) {
        SN_CT16B0->PWMCTRL |= (mskCT16_PWM1EN_EN | mskCT16_PWM1IOEN_EN);
    }
    if (SN_CT16B2->MR0 > 0) {
        SN_CT16B2->PWMCTRL |= (mskCT16_PWM0EN_EN | mskCT16_PWM0IOEN_EN);
    }
    if (SN_CT32B2->MR2 > 0) {
        SN_CT32B2->PWMCTRL |= (mskCT32_PWM2EN_EN | mskCT32_PWM2IOEN_EN);
    }
    if (SN_CT32B0->MR0 > 0) {
        SN_CT32B0->PWMCTRL |= (mskCT32_PWM0EN_EN | mskCT32_PWM0IOEN_EN);
    }
    if (SN_CT32B1->MR2 > 0) {
        SN_CT32B1->PWMCTRL |= (mskCT32_PWM2EN_EN | mskCT32_PWM2IOEN_EN);
    }
    if (SN_CT32B2->MR3 > 0) {
        SN_CT32B2->PWMCTRL |= (mskCT32_PWM3EN_EN | mskCT32_PWM3IOEN_EN);
    }
    if (SN_CT32B1->MR3 > 0) {
        SN_CT32B1->PWMCTRL |= (mskCT32_PWM3EN_EN | mskCT32_PWM3IOEN_EN);
    }
    if (SN_CT32B2->MR1 > 0) {
        SN_CT32B2->PWMCTRL |= (mskCT32_PWM1EN_EN | mskCT32_PWM1IOEN_EN);
    }
    if (SN_CT16B1->MR2 > 0) {
        SN_CT16B1->PWMCTRL |= (mskCT16_PWM2EN_EN | mskCT16_PWM2IOEN_EN);
    }
    if (SN_CT16B2->MR2 > 0) {
        SN_CT16B2->PWMCTRL |= (mskCT16_PWM2EN_EN | mskCT16_PWM2IOEN_EN);
    }
    if (SN_CT32B2->MR0 > 0) {
        SN_CT32B2->PWMCTRL |= (mskCT32_PWM0EN_EN | mskCT32_PWM0IOEN_EN);
    }
    if (SN_CT32B0->MR3 > 0) {
        SN_CT32B0->PWMCTRL |= (mskCT32_PWM3EN_EN | mskCT32_PWM3IOEN_EN);
    }
    if (SN_CT16B1->MR1 > 0) {
        SN_CT16B1->PWMCTRL |= (mskCT16_PWM1EN_EN | mskCT16_PWM1IOEN_EN);
    }
    if (SN_CT16B2->MR1 > 0) {
        SN_CT16B2->PWMCTRL |= (mskCT16_PWM1EN_EN | mskCT16_PWM1IOEN_EN);
    }
    if (SN_CT32B1->MR1 > 0) {
        SN_CT32B1->PWMCTRL |= (mskCT32_PWM1EN_EN | mskCT32_PWM1IOEN_EN);
    }
    if (SN_CT32B0->MR1 > 0) {
        SN_CT32B0->PWMCTRL |= (mskCT32_PWM1EN_EN | mskCT32_PWM1IOEN_EN);
    }
    if (SN_CT16B0->MR2 > 0) {
        SN_CT16B0->PWMCTRL |= (mskCT16_PWM2EN_EN | mskCT16_PWM2IOEN_EN);
    }
    if (SN_CT32B1->MR0 > 0) {
        SN_CT32B1->PWMCTRL |= (mskCT32_PWM0EN_EN | mskCT32_PWM0IOEN_EN);
    }
    if (SN_CT32B0->MR2 > 0) {
        SN_CT32B0->PWMCTRL |= (mskCT32_PWM2EN_EN | mskCT32_PWM2IOEN_EN);
    }
    if (SN_CT16B1->MR0 > 0) {
        SN_CT16B1->PWMCTRL |= (mskCT16_PWM0EN_EN | mskCT16_PWM0IOEN_EN);
    }

    // Set match interrupts and TC reset for CT16B0, which is used to generate interrupt
    SN_CT16B0->MCTRL = (mskCT16_MR3IE_EN | mskCT16_MR3STOP_EN);

    writePinHigh(led_row_pins[current_row]);

    // Let TC start counting. Start CT16B0 last because it generates our interrupt
    SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;
    SN_CT16B2->TMRCTRL |= mskCT16_CEN_EN;
    SN_CT32B0->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT32B1->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT32B2->TMRCTRL |= mskCT32_CEN_EN;
    SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;

    OSAL_IRQ_EPILOGUE();
}

void vs11k09a_init(void) {
    unsigned int i = 0;
    for (unsigned int y = 0; y < VS11K09A_RGB_MATRIX_ROWS; y++) {
        for (unsigned int x = 0; x < VS11K09A_RGB_MATRIX_COLS; x++) {
            if (g_led_config.matrix_co[y][x] != NO_LED) {
                led_pos[g_led_config.matrix_co[y][x]] = i;
            }
            i++;
        }
    }
}

void vs11k09a_flush(void) {}

void vs11k09a_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= RGB_MATRIX_LED_COUNT) return;
    int corrected_index          = led_pos[index];
    led_state[corrected_index].r = r;
    led_state[corrected_index].g = b;
    led_state[corrected_index].b = g;
}

void vs11k09a_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        vs11k09a_set_color(i, r, g, b);
    }
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = vs11k09a_init,
    .flush         = vs11k09a_flush,
    .set_color     = vs11k09a_set_color,
    .set_color_all = vs11k09a_set_color_all,
};
