// Copyright 2024 1Conan (@1Conan) <me@1conan.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include <hal.h>
#include "gpio.h"
#include "config.h"
#include "iton_bt.h"

#ifndef ITON_BT_SPI_PORT
#    define ITON_BT_SPI_PORT SPID0
#endif

#ifndef ITON_BT_IRQ_LINE
#    define ITON_BT_IRQ_LINE A0
#endif

#ifndef ITON_BT_INT_LINE
#    define ITON_BT_INT_LINE A1
#endif

#ifndef ITON_BT_BUFFER_LEN
#    define ITON_BT_BUFFER_LEN 16
#endif

/**
 * Driver Macros
 */
#define HIGH_BITS(x) ((uint8_t)(x >> 8))
#define LOW_BITS(x) ((uint8_t)(x & 0x00FF))

/**
 * Function definitions
 */
void iton_bt_data_cb(SPIDriver *spip);

/**
 * Callbacks
 */
__attribute__((weak)) void iton_bt_battery_voltage_low(void) {}
__attribute__((weak)) void iton_bt_battery_exit_low_battery_mode(void) {}
__attribute__((weak)) void iton_bt_battery_low_power_shutdown(void) {}
__attribute__((weak)) void iton_bt_battery_level(uint8_t level) {}

__attribute__((weak)) void iton_bt_connection_successful(void) {}
__attribute__((weak)) void iton_bt_entered_pairing(void) {}
__attribute__((weak)) void iton_bt_disconnected(void) {}
__attribute__((weak)) void iton_bt_enters_connection_state(void) {}

/**
 * Driver variables
 */
bool iton_bt_is_connected = false;
#ifdef ITON_BT_ENABLE_LED_STATE
uint8_t iton_bt_led_state = 0x00;
#endif

static uint8_t iton_bt_buffer[ITON_BT_BUFFER_LEN];
uint8_t        iton_bt_send_kb_last_key = 0x00;

const SPIConfig iton_bt_spicfg = {
    .slave   = true,
    .data_cb = iton_bt_data_cb,
#if defined(MCU_SN32)
    .ctrl0 = SPI_DATA_LENGTH(8),
#endif
};

/**
 * Callbacks
 */
#if defined(PAL_USE_CALLBACKS) || defined(PAL_USE_WAIT)
static inline void iton_bt_rx_battery_notif(uint8_t data) {
    switch (data) {
        case batt_voltage_low:
            iton_bt_battery_voltage_low();
            break;
        case batt_exit_low_battery_mode:
            iton_bt_battery_exit_low_battery_mode();
            break;
        case batt_low_power_shutdown:
            iton_bt_battery_low_power_shutdown();
            break;
        case batt_above_70:
        case batt_between_30_70:
        case batt_below_30:
            iton_bt_battery_level(data);
            break;
        case batt_wake_mcu:
#    ifdef ITON_BT_ENABLE_ACK
            iton_bt_send_ack(control_bt, wake_ack);
#    endif
            break;
        case batt_unknown:
#    ifdef ITON_BT_ENABLE_ACK
            iton_bt_send_ack(control_bt, unknown_ack);
#    endif
            break;
        case query_working_mode:
            break;
        case query_bt_name:
            break;
    }
}

static inline void iton_bt_rx_bluetooth_notif(uint8_t data) {
    switch (iton_bt_buffer[2]) {
        case bt_connection_success:
            iton_bt_is_connected = true;

#    ifdef ITON_BT_ENABLE_ACK
            iton_bt_send_ack(control_bt, connect_ack);
#    endif

            iton_bt_connection_successful();
            break;
        case bt_entered_pairing:
            iton_bt_entered_pairing();
            break;
        case bt_disconected:
            iton_bt_is_connected = false;

#    ifdef ITON_BT_ENABLE_ACK
            iton_bt_send_ack(control_bt, disconnect_ack);
#    endif

            iton_bt_disconnected();
            break;
        case bt_enters_connection:
            iton_bt_enters_connection_state();
            break;
    }
}

static void iton_bt_rx_cb(void *arg) {
    if (readPin(ITON_BT_INT_LINE)) {
        chSysLockFromISR();
        spiStartReceiveI(&ITON_BT_SPI_PORT, ITON_BT_BUFFER_LEN, &iton_bt_buffer[0]);
        chSysUnlockFromISR();
    } else {
        chSysLockFromISR();
        spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
        chSysUnlockFromISR();

#    ifdef ITON_BT_ENABLE_ACK
        // hack to make sure irq is low since acks messes with stuff
        writePinLow(ITON_BT_IRQ_LINE);
#    endif

        switch (iton_bt_buffer[0]) {
            case led_state:
#    ifdef ITON_BT_ENABLE_LED_STATE
                iton_bt_led_state = iton_bt_buffer[1];
#    endif
                break;
            case notification:
                switch (iton_bt_buffer[1]) {
                    case notif_battery:
                        iton_bt_rx_battery_notif(iton_bt_buffer[2]);
                        break;
                    case notif_bluetooth:
                        iton_bt_rx_bluetooth_notif(iton_bt_buffer[2]);
                        break;
                }
                break;
        }
    }
}
#endif

void iton_bt_data_cb(SPIDriver *spip) {
    writePinLow(ITON_BT_IRQ_LINE);
}

/**
 * Driver Functions
 */
void iton_bt_init(void) {
    setPinOutput(ITON_BT_IRQ_LINE);
    setPinInput(ITON_BT_INT_LINE);

    writePinLow(ITON_BT_IRQ_LINE);

#if defined(PAL_USE_CALLBACKS) || defined(PAL_USE_WAIT)
    palSetLineCallback(ITON_BT_INT_LINE, iton_bt_rx_cb, NULL);
    palEnableLineEvent(ITON_BT_INT_LINE, PAL_EVENT_MODE_BOTH_EDGES);
#endif

    spiStart(&ITON_BT_SPI_PORT, &iton_bt_spicfg);
}

void iton_bt_send(uint8_t cmd, uint8_t *data, uint8_t len) {
    while (readPin(ITON_BT_IRQ_LINE))
        ;

    writePinHigh(ITON_BT_IRQ_LINE);
    iton_bt_buffer[0] = cmd;
    memcpy(&iton_bt_buffer[1], data, len);
    spiStartSend(&ITON_BT_SPI_PORT, len + 1, &iton_bt_buffer[0]);
}

void iton_bt_send2(uint8_t cmd, uint8_t b1, uint8_t b2) {
    while (readPin(ITON_BT_IRQ_LINE))
        ;

    writePinHigh(ITON_BT_IRQ_LINE);
    iton_bt_buffer[0] = cmd;
    iton_bt_buffer[1] = b1;
    iton_bt_buffer[2] = b2;

    spiStartSend(&ITON_BT_SPI_PORT, 3, &iton_bt_buffer[0]);
}

inline void iton_bt_send_ack(uint8_t b1, uint8_t b2) {
    writePinHigh(ITON_BT_IRQ_LINE);
    iton_bt_buffer[0] = control;
    iton_bt_buffer[1] = b1;
    iton_bt_buffer[2] = b2;
    chSysLockFromISR();
    spiStartSendI(&ITON_BT_SPI_PORT, 3, &iton_bt_buffer[0]);
    chSysUnlockFromISR();
}

void iton_bt_send_fn(bool pressed) {
    uint8_t data = pressed ? 0xA3 : 0x00;

    iton_bt_send(report_fn, &data, 1);
}

void iton_bt_send_system(uint16_t data) {
    iton_bt_send(report_system, (uint8_t *)&data, 1);
}

void iton_bt_send_consumer(uint16_t data) {
    iton_bt_send2(report_consumer, HIGH_BITS(data), LOW_BITS(data));
}

void iton_bt_send_keyboard(report_keyboard_t *report) {
    // iton module only reads 5 of the keys in the hid report
    // so we send the last key as an nkro report.
    if (report->keys[5] != iton_bt_send_kb_last_key) {
        uint8_t nkro_report[15] = {0};
        nkro_report[report->keys[5] >> 3] |= (1 << (report->keys[5] & 7));
        iton_bt_send_kb_last_key = report->keys[5];

        return iton_bt_send(report_nkro, &nkro_report[0], 15);
    }

    iton_bt_send(report_hid, &report->mods, 8);
}
