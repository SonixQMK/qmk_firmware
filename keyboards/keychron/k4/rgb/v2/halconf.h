// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define HAL_USE_SPI TRUE
#define PAL_USE_CALLBACKS TRUE

// Hardware SEL framing for the SPI0 slave (SELDIS=0), matching stock firmware.
#define SPI_SELECT_MODE SPI_SELECT_MODE_LLD

#include_next <halconf.h>
