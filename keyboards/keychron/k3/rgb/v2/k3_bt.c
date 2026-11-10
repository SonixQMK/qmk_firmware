// Copyright 2026 mgrenonville
// SPDX-License-Identifier: GPL-2.0-or-later
//
// ITON module transport for the Keychron K3 v2, written after the stock
// firmware (V1.10) disassembly:
//   - SPI0 slave, 8 bit, MSB first, CPOL0/CPHA0, hardware SEL, IRQ prio 0
//   - A0 (IRQ, out): MCU raises it to request a transfer, drops it after the
//     last byte has been clocked
//   - A1 (INT, in) : high while the module is sending to the MCU
//   - TX: only byte 0 is preloaded; every RX interrupt (one per clocked byte)
//     loads the next byte. No FIFO pre-staging.
//   - RX: bytes clocked while A1 is high are collected; the frame is parsed
//     once A1 is low again.
// Implements QMK's "custom" Bluetooth driver.

#include "quantum.h"
#include "bluetooth.h"
#include "report.h"
#include "k3_bt.h"
#include <string.h>

#define K3BT_IRQ_PIN A0
#define K3BT_INT_PIN A1
#define K3BT_TX_TIMEOUT_MS 200
#define K3BT_MAX_FRAME 16

#define SPI0_IE_RXFIFOTH (1u << 2)
#define SPI0_IC_ALL      0xFu
#define SPI0_STAT_BUSY   (1u << 4)
#define SPI0_CTRL0_FRESET (3u << 6)

static volatile uint8_t  tx_buf[K3BT_MAX_FRAME];
static volatile uint8_t  tx_len;
static volatile uint8_t  tx_idx;
static volatile bool     tx_ready = true;
static uint16_t          tx_timer;

static volatile uint8_t  rx_buf[K3BT_MAX_FRAME];
static volatile uint8_t  rx_len;
static volatile bool     rx_pending;

volatile bool k3bt_connected = false;
uint8_t       k3bt_led_state = 0;
k3bt_stats_t  k3bt_stats;

__attribute__((weak)) void k3bt_on_notification(uint8_t group, uint8_t code) {}

OSAL_IRQ_HANDLER(SN32_SPI0_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    if (SN_SPI0->RIS & SPI0_IE_RXFIFOTH) {
        if (SN_GPIO0->DATA & (1u << 1)) { // A1 high: module -> MCU
            uint8_t b = SN_SPI0->DATA;
            if (rx_len < K3BT_MAX_FRAME) rx_buf[rx_len++] = b;
            rx_pending = true;
        } else { // MCU -> module: feed the next byte
            if (tx_idx < tx_len) {
                SN_SPI0->DATA = tx_buf[tx_idx++];
            } else {
                SN_GPIO0->BCLR = (1u << 0); // A0 low
                tx_ready = true;
                tx_idx   = 0;
            }
            (void)SN_SPI0->DATA;
        }
        SN_SPI0->IC = SPI0_IE_RXFIFOTH;
    }
    OSAL_IRQ_EPILOGUE();
}

void k3bt_init(void) {
    gpio_set_pin_output(K3BT_IRQ_PIN);
    gpio_write_pin_low(K3BT_IRQ_PIN);
    gpio_set_pin_input_high(K3BT_INT_PIN);

    SN_SYS1->AHBCLKEN |= (1u << 12); // SPI0 clock
    uint32_t c = SN_SPI0->CTRL0;
    c = (c & ~(0xFu << 8)) | (7u << 8); // DL = 8 bit
    c |= (1u << 3);                     // MS: slave
    c &= ~((1u << 1) | (1u << 2));      // no loopback, SDO enabled
    c &= ~(7u << 15);                   // RX FIFO threshold 0
    c &= ~(7u << 12);                   // TX FIFO threshold 0
    SN_SPI0->CTRL0  = c;
    SN_SPI0->CLKDIV = (SN_SPI0->CLKDIV & ~0xFFu) | 2u;
    SN_SPI0->CTRL1  = 0; // MSB first, CPOL0, CPHA0
    SN_SPI0->CTRL0 &= ~(1u << 18);      // SELDIS = 0: hardware SEL
    SN_SPI0->CTRL0 |= SPI0_CTRL0_FRESET; // FIFO/FSM reset
    SN_SPI0->IC = SPI0_IC_ALL;
    SN_SPI0->IE = SPI0_IE_RXFIFOTH;
    nvicEnableVector(SN32_SPI0_NUMBER, 0);
    SN_SPI0->CTRL0 |= 1u; // SPIEN
    tx_ready = true;
}

bool k3bt_send(const uint8_t *frame, uint8_t len) {
    if (len == 0 || len > K3BT_MAX_FRAME) return false;
    // Wait for the previous transfer (bounded; k3bt_task handles timeouts).
    uint16_t t = timer_read();
    while (!tx_ready) {
        k3bt_task();
        if (timer_elapsed(t) > K3BT_TX_TIMEOUT_MS + 20) {
            k3bt_stats.busy_drops++;
            return false;
        }
    }
    chSysLock();
    SN_GPIO0->BSET = (1u << 0); // A0 high: request
    for (uint8_t i = 0; i < len; i++) tx_buf[i] = frame[i];
    tx_len = len;
    while (SN_SPI0->STAT & SPI0_STAT_BUSY) {}
    SN_SPI0->CTRL0 |= SPI0_CTRL0_FRESET;
    for (uint16_t w = 0; ((SN_SPI0->CTRL0 & SPI0_CTRL0_FRESET) == SPI0_CTRL0_FRESET) && w < 1000; w++) {}
    tx_idx     = 1;
    rx_len     = 0;
    rx_pending = false;
    tx_ready   = false;
    SN_SPI0->DATA = tx_buf[0];
    chSysUnlock();
    tx_timer = timer_read();
    k3bt_stats.sends++;
    return true;
}

bool k3bt_control(uint8_t group, uint8_t param) {
    uint8_t f[3] = {K3BT_CONTROL, group, param};
    return k3bt_send(f, 3);
}

static void k3bt_parse(uint8_t *f, uint8_t n) {
    k3bt_stats.rx_frames++;
    k3bt_stats.last_rx_len = n;
    for (uint8_t i = 0; i < 4; i++) k3bt_stats.last_rx[i] = i < n ? f[i] : 0;
    if (n >= 2 && f[0] == K3BT_LED_STATE) {
        k3bt_led_state = f[1] & 7;
        return;
    }
    if (n >= 3 && f[0] == K3BT_NOTIFICATION) {
        if (f[1] == K3BT_NOTIF_BT) {
            if (f[2] == K3BT_BT_CONNECTED) k3bt_connected = true;
            if (f[2] == K3BT_BT_DISCONNECTED || f[2] == K3BT_BT_PAIRING || f[2] == K3BT_BT_CONNECTING) k3bt_connected = false;
        }
        if (f[1] == K3BT_NOTIF_BATTERY && f[2] == K3BT_BATT_WAKE_MCU) {
            wait_ms(10);
            k3bt_control(K3BT_GROUP_BT, K3BT_WAKE_ACK); // stock acks this one only
        }
        k3bt_on_notification(f[1], f[2]);
        return;
    }
    k3bt_stats.rx_bad++;
}

void k3bt_task(void) {
    // TX timeout, as stock: release A0 and re-arm.
    if (!tx_ready && timer_elapsed(tx_timer) > K3BT_TX_TIMEOUT_MS) {
        chSysLock();
        SN_GPIO0->BCLR = (1u << 0);
        tx_ready = true;
        tx_idx   = 0;
        SN_SPI0->IE = SPI0_IE_RXFIFOTH;
        chSysUnlock();
        k3bt_stats.timeouts++;
    } else if (!tx_ready) {
        return;
    } else if (tx_idx == 0 && tx_len) {
        tx_len = 0;
        k3bt_stats.done++;
    }
    // A complete module frame: bytes received and A1 released.
    if (rx_pending && !(SN_GPIO0->DATA & (1u << 1))) {
        uint8_t f[K3BT_MAX_FRAME], n;
        chSysLock();
        n = rx_len;
        for (uint8_t i = 0; i < n; i++) f[i] = rx_buf[i];
        rx_len     = 0;
        rx_pending = false;
        chSysUnlock();
        if (n) k3bt_parse(f, n);
    }
}

// ---- QMK custom Bluetooth driver -------------------------------------------

void bluetooth_init(void) {
    k3bt_init();
}

void bluetooth_task(void) {
    k3bt_task();
}

bool bluetooth_is_connected(void) {
    return true; // stock sends reports regardless; the module buffers/drops
}

uint8_t bluetooth_keyboard_leds(void) {
    return k3bt_led_state;
}

void bluetooth_send_keyboard(report_keyboard_t *report) {
    uint8_t f[9];
    f[0] = K3BT_REPORT_KEYBOARD;
    f[1] = report->mods;
    f[2] = 0;
    memcpy(&f[3], report->keys, 6);
    k3bt_send(f, 9);
}

void bluetooth_send_consumer(uint16_t usage) {
    uint8_t f[3] = {K3BT_REPORT_CONSUMER, (uint8_t)(usage >> 8), (uint8_t)(usage & 0xFF)};
    k3bt_send(f, 3);
}

void bluetooth_send_system(uint16_t usage) {
    uint8_t f[2] = {K3BT_REPORT_SYSTEM, (uint8_t)usage};
    k3bt_send(f, 2);
}
