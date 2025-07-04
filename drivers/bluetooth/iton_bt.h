// Copyright 2024 1Conan (@1Conan) <me@1conan.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "report.h"

#pragma once

/**
 * Driver Structures
 */
enum iton_bt_cmd {
    // mcu to iton
    report_hid      = 0xA1,
    report_nkro     = 0xA2,
    report_consumer = 0xA3,
    report_system   = 0xA4,
    report_fn       = 0xA5,
    control         = 0xA6,
    set_name        = 0xA7,
    set_alarm_volt  = 0xA8,

    // iton to mcu
    led_state    = 0xB1,
    notification = 0xB6,
};

enum iton_bt_control_cmd {
    control_power = 0x25,
    control_usb   = 0x58,
    control_bt    = 0x51,
    control_pins  = 0x52,
};

enum iton_bt_control_param {
    // control_power
    sleep_idle_10m = 0x01,
    sleep_idle_20m = 0x02,
    sleep_idle_30m = 0x03,

    // control_usb
    mode_usb = 0x01,

    // control_bt
    mode_bt        = 0x62,
    reset_pairing  = 0x70,
    enter_pairing  = 0x89,
    switch_profile = 0x81, // + 0-5 profiles
    os_mac         = 0x74,
    os_win         = 0x75,

    connect_ack    = 0x50,
    disconnect_ack = 0x51,
    wake_ack       = 0x60,
    unknown_ack    = 0x52,

    query_voltage       = 0x66,
    query_battery_level = 0x61,

    disable_sleep = 0x65,
    enable_sleep  = 0x68,
};

enum iton_bt_notification_type {
    notif_battery   = 0x5A,
    notif_bluetooth = 0x51,
};

enum iton_bt_notification_param {
    // notif_battery
    batt_voltage_low           = 0x06,
    batt_exit_low_battery_mode = 0x0A,
    batt_low_power_shutdown    = 0x07,

    batt_above_70      = 0x04,
    batt_between_30_70 = 0x02,
    batt_below_30      = 0x01,

    query_working_mode = 0xA0,
    query_bt_name      = 0xA1,

    // Wake from batt_low_power_shutdown
    batt_wake_mcu = 0x0B,

    batt_unknown = 0x08,

    // notif_bluetooth
    bt_connection_success = 0x76,
    bt_entered_pairing    = 0x77,
    bt_disconected        = 0x78,
    bt_enters_connection  = 0x79,
};

/**
 * Exported Variables
 */
#ifdef ITON_BT_ENABLE_LED_STATE
uint8_t iton_bt_led_state;
#endif
bool iton_bt_is_connected;

/**
 * Driver Callbacks
 */

/**
 * Driver Callbacks
 */
void iton_bt_battery_voltage_low(void);
void iton_bt_battery_exit_low_battery_mode(void);
void iton_bt_battery_low_power_shutdown(void);
void iton_bt_battery_level(uint8_t level);

void iton_bt_connection_successful(void);
void iton_bt_entered_pairing(void);
void iton_bt_disconnected(void);
void iton_bt_enters_connection_state(void);

/**
 * Driver Functions
 */
void iton_bt_init(void);
void iton_bt_send(uint8_t cmd, uint8_t *data, uint8_t len);
void iton_bt_send2(uint8_t cmd, uint8_t b1, uint8_t b2);
void iton_bt_send_ack(uint8_t b1, uint8_t b2);

void iton_bt_send_fn(bool pressed);
void iton_bt_send_system(uint16_t data);
void iton_bt_send_consumer(uint16_t data);
void iton_bt_send_keyboard(report_keyboard_t *report);

/**
 * Driver Macros
 */
#define iton_bt_control(cmd, param) iton_bt_send2(control, cmd, param)
#define iton_bt_control_bt(param) iton_bt_control(control_bt, param)
#define iton_bt_control_usb(param) iton_bt_control(control_usb, param)

#define iton_bt_mode_usb() iton_bt_control_usb(mode_usb)
#define iton_bt_mode_bt() iton_bt_control_bt(mode_bt)
#define iton_bt_reset_pairing() iton_bt_control_bt(reset_pairing)
#define iton_bt_enter_pairing() iton_bt_control_bt(enter_pairing)
#define iton_bt_switch_profile(x) iton_bt_control_bt(switch_profile + x)
#define iton_bt_os_mac() iton_bt_control_bt(os_mac)
#define iton_bt_os_win() iton_bt_control_bt(os_win)
#define iton_bt_query_voltage() iton_bt_control_bt(query_voltage)

#define iton_bt_query_battery_level() iton_bt_control_bt(query_battery_level)
#define iton_bt_disable_sleep() iton_bt_control_bt(disable_sleep)
#define iton_bt_enable_sleep() iton_bt_control_bt(enable_sleep)
