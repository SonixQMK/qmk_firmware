// Copyright 2024 Dimitris Mantzouranis (@dexter93)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define SPLIT_HAND_PIN A7

#if !(defined(USE_SDI0) || defined(USE_SDI1) || defined(USE_SDI2))
    #define USE_SDI0
#endif