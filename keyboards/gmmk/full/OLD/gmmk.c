#include "rgb_matrix.h"

/*
 * turn off rgb when power off / suspend
 */
void suspend_power_down_kb(void) {
    rgb_matrix_set_suspend_state(true);
    suspend_power_down_user();
}

void housekeeping_task_kb(void) { rgb_matrix_set_suspend_state(false); }

#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
            wait_ms(5);
        } else {
            tap_code(KC_VOLD);
            wait_ms(5);
        }
    }
    return true;
}

bool        encoder_update_kb(uint8_t index, bool clockwise) { return encoder_update_user(index, clockwise); }
#endif

#ifdef RAW_CONSOLE
extern void raw_hid_send(uint8_t *data, uint8_t length);

int8_t sendchar(uint8_t c) {
    raw_hid_send(&c, 1);

    return 1;
}
#endif

/*
 * must override sleep functions without idle thread
 */
#if CH_CFG_NO_IDLE_THREAD == TRUE

#    define CYCLES_PER_LOOP 9
#    define LOOP_TIMES (48000000 / (CH_CFG_ST_FREQUENCY) / (CYCLES_PER_LOOP))

void chThdSleep(sysinterval_t time) {
    uint32_t loops = time * LOOP_TIMES;

    for (uint32_t i = 0; i < loops; i++) __NOP();
}

/* suspend thread used in usb_main.c */
msg_t chThdSuspendTimeoutS(thread_reference_t *trp, sysinterval_t timeout) {
    osalSysUnlock();
    wait_ms(1);
    //while (usbGetTransmitStatusI(&USBD1, KEYBOARD_IN_EPNUM));
#ifdef SHARED_EP_ENABLE
    //while (usbGetTransmitStatusI(&USBD1, SHARED_IN_EPNUM));
#endif
    osalSysLock();

    return MSG_OK;
}

#endif /* CH_CFG_NO_IDLE_THREAD */

int rand(void) {
    static uint32_t seed = 134775813U;
    seed                 = seed * 1664525U + 1013904223U;
    return seed;
}

#ifdef GMMK_DEBUG
void rgbprint(int v, uint8_t r, uint8_t g, uint8_t b) {
    int32_t kc = 26;

    if (v != 0) kc = 17 + v;

    rgb_matrix_set_color(kc, r, g, b);
}
#endif

#ifdef VIA_OPENRGB_HYBRID
uint8_t is_orgb_mode = 1;
#endif