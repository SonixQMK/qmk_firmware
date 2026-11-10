// Copyright 2026 mgrenonville
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

// Optical switches need time to settle after a row is selected: with the
// default (a few cycles) the first columns read mid-transition, which shows up
// as heavy chatter on columns 0-2 when running from the battery (BT mode).
// Runs inside the shared-matrix PWM ISR, so busy-wait (~8 cycles/iteration).
#ifndef K3V2_MATRIX_SELECT_DELAY_US
#    define K3V2_MATRIX_SELECT_DELAY_US 20
#endif
void matrix_output_select_delay(void) {
    for (volatile uint32_t i = 0; i < (K3V2_MATRIX_SELECT_DELAY_US * (CPU_CLOCK / 8000000UL)); i++) {
    }
}
