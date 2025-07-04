// Copyright 2024 Dimitris Mantzouranis (@dexter93)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

// These are needed if the communication is on the SD0 UART, either with standard or alternate pins
#undef SN32_SERIAL_USE_UART0
#define SN32_SERIAL_USE_UART0 TRUE

// These are needed if the communication is on the SD1 UART, either with standard or alternate pins
//#undef SN32_SERIAL_USE_UART1
//#define SN32_SERIAL_USE_UART1 TRUE

// These are needed if the communication is on the SD2 UART, either with standard or alternate pins
//#undef SN32_SERIAL_USE_UART2
//#define SN32_SERIAL_USE_UART2 TRUE