// Copyright 2024 Dimitris Mantzouranis (@dexter93)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(SPLIT_SERIAL_FULLDUPLEX)
#    define SERIAL_USART_FULL_DUPLEX // Enable full duplex operation mode.
#endif
#if defined(SPLIT_SERIAL_HALFDUPLEX) || defined(SPLIT_SERIAL_FULLDUPLEX)
#    if defined(USE_SDI0)
#        define SOFT_SERIAL_PIN A10
#        define SERIAL_USART_RX_PIN A11 // USART RX pin
#        define SERIAL_USART_DRIVER SD0
#    elif defined(USE_SDI1)
#        define SOFT_SERIAL_PIN B9
#        define SERIAL_USART_RX_PIN B8 // USART RX pin
#    elif defined(USE_SDI2)
#        define SOFT_SERIAL_PIN A1
#        define SERIAL_USART_RX_PIN A0  // USART RX pin
#        define SERIAL_USART_DRIVER SD2 // USART driver of TX and RX pin. default: SD1
#    else
#        error "splittest serial keymap is missing a configuration"
#    endif
#else // defined(SPLIT_BITBANG)
#    define SOFT_SERIAL_PIN B8
#endif