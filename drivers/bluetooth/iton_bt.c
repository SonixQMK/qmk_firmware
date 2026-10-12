// Copyright 2024 1Conan (@1Conan) <me@1conan.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include <hal.h>
#include "gpio.h"
#include "config.h"
#include "timer.h"
#include "wait.h"
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

#ifdef CONSOLE_ENABLE
extern uint16_t iton_bt_dbg_sends, iton_bt_dbg_timeouts, iton_bt_dbg_rx_seq;
extern uint16_t iton_bt_dbg_done_tx, iton_bt_dbg_done_rx, iton_bt_dbg_tx_aborted;
extern uint16_t iton_bt_dbg_int_hi, iton_bt_dbg_int_lo, iton_bt_dbg_sent_hi, iton_bt_dbg_sent_lo;
extern bool     iton_bt_dbg_last_tx;
extern uint8_t  iton_bt_dbg_rx_ring[8][6];
extern uint8_t  iton_bt_dbg_rx_len[8];
extern uint8_t  iton_bt_dbg_reply_ring[8][12];
extern uint8_t  iton_bt_dbg_reply_idx[8];
extern size_t   iton_bt_dbg_rx_idx;
#endif

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
// The SN32 SPI LLD keeps writing rxbuf[idx] for every byte the module clocks,
// even after the requested count is reached, so oversize the buffer to hold
// the module's longer reply frames without corrupting neighbouring statics.
static uint8_t iton_bt_rx_buffer[96];
static size_t  iton_bt_rx_frame_start = 0;
static bool    iton_bt_tx_active  = false;
static bool    iton_bt_tx_pending = false;
static uint8_t iton_bt_last_tx_len = 0;
uint8_t        iton_bt_send_kb_last_key = 0x00;
// Timestamp of the end of our last completed frame; used to keep a minimum
// spacing between frames so back-to-back control/report sends can't race
// the module's processing window. Only TX completions update it: stray
// module clocks fire the completion path constantly and would otherwise
// keep the quiet window open forever.
static volatile systime_t iton_bt_last_bus_time = 0;
static inline void iton_bt_preload_tx(size_t frame_len);

const SPIConfig iton_bt_spicfg = {
    .slave   = true,
    .data_cb = iton_bt_data_cb,
#if defined(MCU_SN32)
    .ctrl0 = SPI_DATA_LENGTH(8),
    // In slave mode the module drives SCK, but the SN32 LLD still derives a
    // clock from clkdiv and enables DFETCH_EN when the result exceeds 6 MHz.
    // With clkdiv unset it computes HCLK/2 and wrongly sets DFDLY, making the
    // slave prefetch TX data the module never expects. Keep it disabled.
    .clkdiv = 0xff,
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
    switch (data) {
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
#ifdef CONSOLE_ENABLE
        iton_bt_dbg_int_hi++;
#endif
        chSysLockFromISR();
        if (iton_bt_tx_active) {
            // Module started clocking a reply while our TX was still armed.
            // Queue the frame for re-send once its frame ends.
            spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
            iton_bt_tx_active  = false;
            iton_bt_tx_pending = true;
            writePinLow(ITON_BT_IRQ_LINE);
#ifdef CONSOLE_ENABLE
            iton_bt_dbg_tx_aborted++;
#endif
        }
#ifdef CONSOLE_ENABLE
        iton_bt_dbg_last_tx = false;
#endif
        // A receive window stays armed at all times so a push's first byte
        // is captured even when clocked before this handler runs. Re-arm
        // only if the window has been exhausted; the INT edge marks where
        // this frame begins inside the window, not where it must restart.
        // Only arm from SPI_READY: starting over ACTIVE/COMPLETE reconfigures
        // the engine mid-transfer and can wedge it.
        if (ITON_BT_SPI_PORT.state == SPI_READY) {
            spiStartReceiveI(&ITON_BT_SPI_PORT, sizeof(iton_bt_rx_buffer), &iton_bt_rx_buffer[0]);
            iton_bt_rx_frame_start = 0;
        } else {
            iton_bt_rx_frame_start = ITON_BT_SPI_PORT.idx;
        }
        chSysUnlockFromISR();
    } else {
#ifdef CONSOLE_ENABLE
        iton_bt_dbg_int_lo++;
#endif
        chSysLockFromISR();
        size_t end = ITON_BT_SPI_PORT.idx;
        spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
        chSysUnlockFromISR();

#    ifdef ITON_BT_ENABLE_ACK
        // hack to make sure irq is low since acks messes with stuff
        writePinLow(ITON_BT_IRQ_LINE);
#    endif

        if (end > sizeof(iton_bt_rx_buffer)) {
            end = sizeof(iton_bt_rx_buffer);
        }
        size_t start = iton_bt_rx_frame_start;
        if (start > end) {
            start = end;
        }
        uint8_t len     = end - start;
        uint8_t *frame  = &iton_bt_rx_buffer[start];

#    ifdef CONSOLE_ENABLE
        iton_bt_dbg_rx_seq++;
        iton_bt_dbg_rx_len[iton_bt_dbg_rx_seq % 8] = len;
        memcpy(iton_bt_dbg_rx_ring[iton_bt_dbg_rx_seq % 8], frame, len < 6 ? len : 6);
#    endif

        switch (len ? frame[0] : 0) {
            case led_state:
#    ifdef ITON_BT_ENABLE_LED_STATE
                if (len >= 2) {
                    iton_bt_led_state = frame[1];
                }
#    endif
                break;
            case notification:
                if (len >= 3) {
                    switch (frame[1]) {
                        case notif_battery:
                            iton_bt_rx_battery_notif(frame[2]);
                            break;
                        case notif_bluetooth:
                            iton_bt_rx_bluetooth_notif(frame[2]);
                            break;
                    }
                }
                break;
        }

        if (iton_bt_tx_pending && iton_bt_last_tx_len) {
            // Re-send the frame the module's reply interrupted.
            iton_bt_tx_pending = false;
            iton_bt_tx_active  = true;
            writePinHigh(ITON_BT_IRQ_LINE);
            chSysLockFromISR();
            // A queued SPI-complete IRQ may have re-armed a transfer while
            // the frame was being parsed above.
            spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
            spiStartSendI(&ITON_BT_SPI_PORT, iton_bt_last_tx_len, &iton_bt_buffer[0]);
            chSysUnlockFromISR();
        } else {
            // Re-arm the listening window for the module's next push.
            chSysLockFromISR();
            spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
            spiStartReceiveI(&ITON_BT_SPI_PORT, sizeof(iton_bt_rx_buffer), &iton_bt_rx_buffer[0]);
            iton_bt_rx_frame_start = 0;
            chSysUnlockFromISR();
        }
    }
}
#endif

void iton_bt_data_cb(SPIDriver *spip) {
    if (iton_bt_tx_active) {
        iton_bt_last_bus_time = chVTGetSystemTimeX();
        iton_bt_tx_active = false;
        writePinLow(ITON_BT_IRQ_LINE);
#ifdef CONSOLE_ENABLE
        iton_bt_dbg_done_tx++;
#endif
        // Re-arm the listening window so the module's next push is captured
        // from its first byte instead of hitting an exhausted transfer. The
        // complete-code macro permits starting a new transfer from the
        // callback once the driver is back in SPI_READY. Skip if the INT
        // handler preempted and armed one already.
        if (spip->state == SPI_COMPLETE) {
            spip->state = SPI_READY;
            spiStartReceiveI(spip, sizeof(iton_bt_rx_buffer), &iton_bt_rx_buffer[0]);
            iton_bt_rx_frame_start = 0;
        }
    }
#ifdef CONSOLE_ENABLE
    else {
        iton_bt_dbg_done_rx++;
    }
    iton_bt_dbg_rx_idx = spip->idx;
#endif
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

    // Keep a receive window armed from boot: the module can raise INT and
    // clock a push at any time, and bytes clocked into a dead transfer are
    // silently dropped by the ISR.
    chSysLock();
    spiStartReceiveI(&ITON_BT_SPI_PORT, sizeof(iton_bt_rx_buffer), &iton_bt_rx_buffer[0]);
    chSysUnlock();
}

#ifndef ITON_BT_IRQ_WAIT_TIMEOUT_MS
#    define ITON_BT_IRQ_WAIT_TIMEOUT_MS 50
#endif

#ifdef CONSOLE_ENABLE
uint16_t iton_bt_dbg_sends      = 0;
uint16_t iton_bt_dbg_timeouts   = 0;
uint16_t iton_bt_dbg_rx_seq     = 0;
uint16_t iton_bt_dbg_done_tx    = 0;
uint16_t iton_bt_dbg_done_rx    = 0;
uint16_t iton_bt_dbg_tx_aborted = 0;
uint16_t iton_bt_dbg_int_hi     = 0;
uint16_t iton_bt_dbg_int_lo     = 0;
uint16_t iton_bt_dbg_sent_hi    = 0;
uint16_t iton_bt_dbg_sent_lo    = 0;
bool     iton_bt_dbg_last_tx    = false;
uint8_t  iton_bt_dbg_rx_ring[8][6] = {0};
uint8_t  iton_bt_dbg_rx_len[8]     = {0};
uint8_t  iton_bt_dbg_reply_ring[8][12] = {0};
uint8_t  iton_bt_dbg_reply_idx[8]      = {0};
size_t   iton_bt_dbg_rx_idx            = 0;

void iton_bt_debug_stats(uint16_t *sends, uint16_t *done_tx, uint16_t *done_rx, uint16_t *timeouts, uint16_t *tx_aborted, uint16_t *rx_seq, uint16_t *int_hi, uint16_t *int_lo, uint16_t *sent_hi, uint16_t *sent_lo) {
    *sends      = iton_bt_dbg_sends;
    *done_tx    = iton_bt_dbg_done_tx;
    *done_rx    = iton_bt_dbg_done_rx;
    *timeouts   = iton_bt_dbg_timeouts;
    *tx_aborted = iton_bt_dbg_tx_aborted;
    *rx_seq     = iton_bt_dbg_rx_seq;
    *int_hi     = iton_bt_dbg_int_hi;
    *int_lo     = iton_bt_dbg_int_lo;
    *sent_hi    = iton_bt_dbg_sent_hi;
    *sent_lo    = iton_bt_dbg_sent_lo;
}

uint8_t iton_bt_debug_rx_frame(uint16_t seq, uint8_t out[6]) {
    if (seq == 0 || seq > iton_bt_dbg_rx_seq || iton_bt_dbg_rx_seq - seq >= 8) {
        return 0;
    }
    memcpy(out, iton_bt_dbg_rx_ring[seq % 8], 6);
    return iton_bt_dbg_rx_len[seq % 8];
}

uint8_t iton_bt_debug_reply(uint16_t send_seq, uint8_t out[12]) {
    memcpy(out, iton_bt_dbg_reply_ring[send_seq % 8], 12);
    return iton_bt_dbg_reply_idx[send_seq % 8];
}
#endif

#ifndef ITON_BT_TX_QUIET_US
#    define ITON_BT_TX_QUIET_US 10000
#endif

#ifndef ITON_BT_TX_QUIET_TIMEOUT_MS
#    define ITON_BT_TX_QUIET_TIMEOUT_MS 30
#endif

// Keep consecutive frames spaced so the module finishes processing one
// before the next IRQ pulse; back-to-back control frames at boot can
// otherwise be dropped silently.
static void iton_bt_wait_bus_idle(void) {
    uint16_t start = timer_read();
    while ((systime_t)(chVTGetSystemTime() - iton_bt_last_bus_time) < TIME_US2I(ITON_BT_TX_QUIET_US)) {
        if (timer_elapsed(start) > ITON_BT_TX_QUIET_TIMEOUT_MS) {
            return;
        }
        wait_us(100);
    }
}

static void iton_bt_fifo_reset(void) {
    // Stock waits for the SPI engine to go idle, pulses FRESET, then polls
    // until the self-clearing reset bits release before loading data.
    // spiStopTransfer asserts FRESET; staging bytes while the reset is still
    // in flight flushes the head of the next frame. Both waits are bounded
    // so a wedged peripheral can't hang the keyboard.
    uint16_t start = timer_read();
    while (ITON_BT_SPI_PORT.spi->STAT_b.BUSY) {
        if (timer_elapsed(start) > 1) {
            break;
        }
    }
    spiStopTransfer(&ITON_BT_SPI_PORT, NULL);
    start = timer_read();
    while (ITON_BT_SPI_PORT.spi->CTRL0_b.FRESET) {
        if (timer_elapsed(start) > 1) {
            break;
        }
    }
}

// The SN32 SPI ISR only queues the next TX byte after each received byte,
// which can't keep pace with the module's clock bursts. An underflowed TX
// FIFO shifts out garbage mid-frame and the module drops the whole report.
// Preload the hardware FIFO before raising IRQ so the frame is staged ahead
// of time and the ISR only has to top up the tail. The shared rx/tx index is
// preset to the staged count, and the transfer count grown to match so
// completion still fires only once every real byte has been clocked out
// (trailing ISR writes shift out zero padding).
static inline void iton_bt_preload_tx(size_t frame_len) {
    // Room in iton_bt_buffer past the real frame bounds how far the shared
    // index may grow, which in turn bounds how many bytes can be prestaged.
    size_t limit = frame_len < ITON_BT_BUFFER_LEN ? ITON_BT_BUFFER_LEN - frame_len + 1 : 1;
    size_t pre   = 1; // spi_lld_exchange already queued byte 0
    while (pre < frame_len && pre < limit && !ITON_BT_SPI_PORT.spi->STAT_b.TX_FULL) {
        ITON_BT_SPI_PORT.spi->DATA = iton_bt_buffer[pre];
        pre++;
    }
    ITON_BT_SPI_PORT.idx   = pre - 1;
    ITON_BT_SPI_PORT.count = frame_len + pre - 1;
}

// The BT module may never clock the SPI bus (e.g. absent or asleep); bound the
// wait so the keyboard can't hang forever.
static bool iton_bt_wait_irq_clear(void) {
    uint16_t start = timer_read();
    while (readPin(ITON_BT_IRQ_LINE)) {
        if (timer_elapsed(start) > ITON_BT_IRQ_WAIT_TIMEOUT_MS) {
            writePinLow(ITON_BT_IRQ_LINE);
            iton_bt_tx_active = false;
            spiStopTransfer(&ITON_BT_SPI_PORT, NULL);
#ifdef CONSOLE_ENABLE
            iton_bt_dbg_timeouts++;
#endif
            return false;
        }
    }
    return true;
}

void iton_bt_send(uint8_t cmd, uint8_t *data, uint8_t len) {
    iton_bt_tx_active = true;
    if (!iton_bt_wait_irq_clear()) {
        return;
    }
    iton_bt_wait_bus_idle();

    // Abort any stale transfer and reset the FIFOs before framing a new one.
    iton_bt_fifo_reset();
    memset(iton_bt_rx_buffer, 0, sizeof(iton_bt_rx_buffer));
    iton_bt_buffer[0] = cmd;
    memcpy(&iton_bt_buffer[1], data, len);
    memset(&iton_bt_buffer[len + 1], 0, ITON_BT_BUFFER_LEN - len - 1);
    iton_bt_last_tx_len = len + 1;
    iton_bt_tx_pending  = false;
    iton_bt_tx_active   = true;
#ifdef CONSOLE_ENABLE
    // Snapshot the bytes the module clocked during/after the previous frame.
    memcpy(iton_bt_dbg_reply_ring[iton_bt_dbg_sends % 8], iton_bt_rx_buffer, 12);
    iton_bt_dbg_reply_idx[iton_bt_dbg_sends % 8] = iton_bt_dbg_rx_idx > 255 ? 255 : (uint8_t)iton_bt_dbg_rx_idx;
    iton_bt_dbg_sends++;
    iton_bt_dbg_last_tx = true;
    if (readPin(ITON_BT_INT_LINE)) {
        iton_bt_dbg_sent_hi++;
    } else {
        iton_bt_dbg_sent_lo++;
    }
#endif
    // Stop and start must be atomic: an INT edge between them would let the
    // ISR re-arm the receive window and this exchange would stomp an active
    // transfer, wedging the SPI engine.
    chSysLock();
    spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
    // If the ISR re-armed a transfer after fifo_reset, the stop above raises
    // FRESET again; wait for it to release before staging bytes.
    for (uint16_t w = 0; ITON_BT_SPI_PORT.spi->CTRL0_b.FRESET && w < 1000; w++) {}
    spiStartExchangeI(&ITON_BT_SPI_PORT, len + 1, &iton_bt_buffer[0], &iton_bt_rx_buffer[0]);
    iton_bt_preload_tx(len + 1);
    writePinHigh(ITON_BT_IRQ_LINE);
    chSysUnlock();
}

void iton_bt_send2(uint8_t cmd, uint8_t b1, uint8_t b2) {
    iton_bt_tx_active = true;
    if (!iton_bt_wait_irq_clear()) {
        return;
    }
    iton_bt_wait_bus_idle();

    iton_bt_fifo_reset();
    memset(iton_bt_rx_buffer, 0, sizeof(iton_bt_rx_buffer));
    iton_bt_buffer[0] = cmd;
    iton_bt_buffer[1] = b1;
    iton_bt_buffer[2] = b2;
    memset(&iton_bt_buffer[3], 0, ITON_BT_BUFFER_LEN - 3);
    iton_bt_last_tx_len = 3;
    iton_bt_tx_pending  = false;
    iton_bt_tx_active   = true;
#ifdef CONSOLE_ENABLE
    memcpy(iton_bt_dbg_reply_ring[iton_bt_dbg_sends % 8], iton_bt_rx_buffer, 12);
    iton_bt_dbg_reply_idx[iton_bt_dbg_sends % 8] = iton_bt_dbg_rx_idx > 255 ? 255 : (uint8_t)iton_bt_dbg_rx_idx;
    iton_bt_dbg_sends++;
    iton_bt_dbg_last_tx = true;
    if (readPin(ITON_BT_INT_LINE)) {
        iton_bt_dbg_sent_hi++;
    } else {
        iton_bt_dbg_sent_lo++;
    }
#endif
    chSysLock();
    spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
    for (uint16_t w = 0; ITON_BT_SPI_PORT.spi->CTRL0_b.FRESET && w < 1000; w++) {}
    spiStartExchangeI(&ITON_BT_SPI_PORT, 3, &iton_bt_buffer[0], &iton_bt_rx_buffer[0]);
    iton_bt_preload_tx(3);
    writePinHigh(ITON_BT_IRQ_LINE);
    chSysUnlock();
}

inline void iton_bt_send_ack(uint8_t b1, uint8_t b2) {
    iton_bt_last_tx_len = 3;
    iton_bt_tx_pending  = false;
    iton_bt_tx_active   = true;
    writePinHigh(ITON_BT_IRQ_LINE);
    iton_bt_buffer[0] = control;
    iton_bt_buffer[1] = b1;
    iton_bt_buffer[2] = b2;
    chSysLockFromISR();
    spiStopTransferI(&ITON_BT_SPI_PORT, NULL);
    spiStartSendI(&ITON_BT_SPI_PORT, 3, &iton_bt_buffer[0]);
    chSysUnlockFromISR();
}

void iton_bt_send_fn(bool pressed) {
    // Stock K4 firmware signals Fn-held as 0x03, not the upstream 0xA3.
    uint8_t data = pressed ? 0x03 : 0x00;

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
    // so we send the last key as an nkro report. The A1 report still goes
    // out as well: suppressing it would drop simultaneous changes to
    // slots 0-4 that share the transition.
    if (report->keys[5] != iton_bt_send_kb_last_key) {
        uint8_t nkro_report[15] = {0};
        nkro_report[report->keys[5] >> 3] |= (1 << (report->keys[5] & 7));
        iton_bt_send_kb_last_key = report->keys[5];

        iton_bt_send(report_nkro, &nkro_report[0], 15);
    }

    iton_bt_send(report_hid, &report->mods, 8);
}
