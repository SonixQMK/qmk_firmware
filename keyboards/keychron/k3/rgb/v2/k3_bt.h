// Copyright 2026 mgrenonville
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Minimal ITON Bluetooth transport for the Keychron K3 v2, mirroring the stock
// firmware (SPI0 slave, byte-per-interrupt TX, A0 = request, A1 = module busy).
#pragma once

#include <stdbool.h>
#include <stdint.h>

// Frame types (first byte)
#define K3BT_REPORT_KEYBOARD 0xA1 // + 8 bytes
#define K3BT_REPORT_CONSUMER 0xA3 // + 2 bytes (high, low)
#define K3BT_REPORT_SYSTEM   0xA4 // + 1 byte
#define K3BT_CONTROL         0xA6 // + 2 bytes
#define K3BT_LED_STATE       0xB1 // module -> MCU, + 1 byte
#define K3BT_NOTIFICATION    0xB6 // module -> MCU, + 2 bytes

// Control groups / parameters (A6 <group> <param>)
#define K3BT_GROUP_POWER 0x25
#define K3BT_GROUP_USB   0x58
#define K3BT_GROUP_BT    0x51

#define K3BT_USB_MODE        0x01 // A6 58 01
#define K3BT_WAKE_ACK        0x60 // A6 51 60
#define K3BT_QUERY_BATTERY   0x61
#define K3BT_DISABLE_SLEEP   0x65
#define K3BT_ENABLE_SLEEP    0x68
#define K3BT_RESET_PAIRING   0x70
#define K3BT_SWITCH_PROFILE  0x81 // + profile 0..2
#define K3BT_ENTER_PAIRING   0x89

// Notifications (B6 <group> <code>)
#define K3BT_NOTIF_BATTERY   0x5A
#define K3BT_NOTIF_BT        0x51
#define K3BT_BATT_BELOW_30   0x01
#define K3BT_BATT_30_70      0x02
#define K3BT_BATT_ABOVE_70   0x04
#define K3BT_BATT_LOW        0x06
#define K3BT_BATT_SHUTDOWN   0x07
#define K3BT_BATT_SLEEP_REQ  0x08 // module -> MCU: go to sleep
#define K3BT_BATT_WAKE_MCU   0x0B
#define K3BT_BT_CONNECTED    0x76
#define K3BT_BT_PAIRING      0x77
#define K3BT_BT_DISCONNECTED 0x78
#define K3BT_BT_CONNECTING   0x79

typedef struct {
    uint16_t sends, done, timeouts, busy_drops, rx_frames, rx_bad;
    uint8_t  last_rx[4];
    uint8_t  last_rx_len;
} k3bt_stats_t;

extern volatile bool k3bt_connected;
extern uint8_t       k3bt_led_state;
extern k3bt_stats_t  k3bt_stats;

void k3bt_init(void);
void k3bt_task(void);
bool k3bt_send(const uint8_t *frame, uint8_t len);
bool k3bt_control(uint8_t group, uint8_t param);

// Board-level callbacks (weak), called from k3bt_task (thread context).
void k3bt_on_notification(uint8_t group, uint8_t code);
