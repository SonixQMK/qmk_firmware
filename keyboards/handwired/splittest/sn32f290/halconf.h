// Copyright 2024 Dimitris Mantzouranis (@dexter93)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* Needed for serial bitbang, half-duplex and full-duplex */
#define HAL_USE_PAL TRUE
// Needed for serial bitbang
//#define PAL_USE_WAIT TRUE
//#define PAL_USE_CALLBACKS TRUE
#include_next <halconf.h>