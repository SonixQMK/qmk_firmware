// Copyright 2026
// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Host side of the Redragon K530 Draconic radio link.
 *
 * The PixArt PAR2801 keeps its own firmware. The Sonix is an SPI slave
 * (P0.2 MISO, P0.3 MOSI, P0.4 SCK, P0.5 SEL) and pulses P0.1 when a new
 * 39-byte frame is ready. Byte 0 is the command, byte 38 is the sum of
 * bytes 0..37. Reconstructed from the stock SN32 image, see stock_radio.md.
 *
 * The first clocked byte is the module's command and is answered with
 * tx[0]. Later bytes are stored and, when that command is 0xA2, answered
 * with the rest of the frame. 0x20 is echoed. Anything else is answered
 * with zeros after the first byte.
 *
 * Stock never sends a control frame at boot in wireless. The module
 * reconnects to its saved host by itself. Pairing a new host is a held
 * key (0xA6, byte 3 = 1), and leaving or entering wired mode is 0xA5.
 */

#include "quantum.h"
#include "bluetooth.h"
#include "connection.h"
#include "usb_main.h"
#include "k530.h"

#define K530_BT_FRAME_LEN 39
#define K530_BT_READY_PIN A1
#define K530_BT_MODE_PIN B13

/* Stock waits up to 32 x delay(2000) for USB, about 13 s at 48 MHz. */
#define K530_BT_USB_WAIT_MS 3000
#define K530_BT_LINK_DEBOUNCE_MS 1000
#define K530_BT_PAIR_HOLD_MS 3000
/* Frame byte 2 of 0xAA key reports: stock's status byte 0x200000B6, 0 in normal use. */
#define K530_BT_REPORT_STATUS 0
#define K530_BT_REPULSE_MS 10
#define K530_BT_REPULSE_MAX 5
/*
 * Pulses of one press before its release may go out. A layer-tap sends the
 * press once and then waits inside the key handler (80 ms for Caps Lock,
 * 0 for Escape) before the release. That wait blocks this loop, so the
 * press is not offered again. The release has to wait until the pulses
 * have actually happened.
 */
#define K530_BT_MIN_PULSES 5
#define K530_BT_LED 62
/* Stock blink periods for module states 1 and 2: 32 and 96 ticks of 20 ms. */
#define K530_BT_BLINK_FAST_MS 640
#define K530_BT_BLINK_SLOW_MS 1920
#define K530_BT_PAIR_TIMEOUT_MS 60000
#define K530_BT_RECONNECT_TIMEOUT_MS 10000
#define K530_BT_CONNECTED_SHOW_MS 2000
#define K530_BT_IDLE_LEVEL 48

enum k530_link {
    LINK_UNKNOWN,
    LINK_WIRED,    /* stock 0x200000BB = 1 */
    LINK_WIRELESS, /* stock 0x200000BB = 2 */
};

static uint8_t          frame[K530_BT_FRAME_LEN];
static volatile uint8_t tx[K530_BT_FRAME_LEN + 1];
/* Only the 0xA7 bytes, the running sum, and the previous byte for the 0x20 echo are kept. */
static volatile uint8_t rx[6];
static volatile uint8_t rx_sum;
static volatile uint8_t rx_prev;
static volatile uint8_t txn_open;
static volatile uint8_t rx_count;
static volatile uint8_t tx_count;
static volatile uint8_t first_byte;
static volatile uint8_t pulse_pending;
static volatile uint8_t burst;
static volatile uint8_t publish_seq;
static volatile uint8_t txn_seq;
static uint16_t         txn_idle_at;

/*
 * 0xA7 bytes 3, 4, and 5, as stock stores them at 0x200000C0, 0x200000BF,
 * and 0x200000C1. Byte 4 is the connection state. Byte 3 is the host LED
 * byte: stock 0x6572 paints bit 1 (Caps Lock) and clears the byte while
 * wired. Byte 5 is read at 0x65A0 (values 1, 2, and 0xF0) for a different
 * lamp; it is kept and not shown.
 */
static volatile uint8_t  module_stat[3];
static volatile uint8_t  module_seen;
static volatile uint16_t a7_seen_at; /* last 0xA7 reply */
static uint16_t          aa_sent_at;
static uint16_t          pulsed_at;
static uint8_t           repulses;
static uint8_t           down_pulses; /* ready pulses of the current key set */

static volatile uint8_t hold_key;
static volatile uint8_t need_release;
static bool             keys_down; /* the last report published carried a key */
/* Last wireless key report, republished when a media usage changes. */
static uint8_t          key_mods;
static uint8_t          key_reserved;
static uint8_t          key_codes[6];
/* Stock puts this in 0xAA bytes 33 and 34. The wire opcode stays 0xAA. */
static uint16_t         consumer_usage;
/*
 * Stock copies 0x20000032 bytes 1..7 into 0xAA bytes 26..32. Byte 0 of
 * that buffer is report id 5 and is not copied. The HID descriptor for
 * report 5 is five buttons, 16-bit X, 16-bit Y, an 8-bit wheel, and an
 * 8-bit horizontal wheel.
 */
static uint8_t          mouse_buttons;
static int16_t          mouse_x;
static int16_t          mouse_y;
static int8_t           mouse_v;
static int8_t           mouse_h;
/*
 * The radio forwards a payload once and drops a repeat of the same bytes.
 * That suits a held letter. A held mouse key repeats one delta, so each
 * step flips a padding bit in the button byte (HID report 5 has five
 * buttons and three constant bits) and the step is forwarded again.
 */
static uint8_t          mouse_mark;

static uint8_t  link = LINK_UNKNOWN;
static uint16_t boot_at;
static bool     link_want_wired;
static uint16_t link_changed_at;

static uint8_t  bt_channel;
static uint8_t  sent_channel; /* stock 0x200000BD, 0 at boot */
static uint8_t  channel_seen;
static uint8_t  channel_count;
static uint16_t channel_sampled_at;
static uint16_t channel_changed_at;
static bool     pair_held;
static bool     pair_long_sent;
static uint16_t pair_pressed_at;

/*
 * Module state for the Bluetooth LED, as stock keeps it in 0x200000BF:
 * 0 quiet, 1 and 2 blink. A pair or reconnect command sets it at once;
 * the 0xA7 reply clocked with that command predates it, so replies are
 * ignored for the first second.
 */
static uint8_t  radio_state;
static uint16_t state_hold_at;
static uint16_t state_hold_ms;
static uint16_t a7_used_at;
static uint16_t connected_at;

static bool usb_up(void) {
    return usbGetDriverStateI(&USB_DRIVER) == USB_ACTIVE;
}

static void pull_link_pin(void) {
    /* Stock writes 0 to GPIO1 CFG, which pull-ups every pin on the port.
     * P1.14 is the mode pin copied into the radio frame. */
    SN_GPIO1->MODE &= ~(1U << 14);
    SN_GPIO1->CFG &= ~(3U << 28);
}

static bool p1_14_high(void) {
    pull_link_pin();
    return (SN_GPIO1->DATA & (1U << 14)) != 0;
}

static uint8_t link_mode(void) {
    /* Stock firmware: GPIOB.14 high -> 1, low -> 2. */
    return p1_14_high() ? 1 : 2;
}

static void select_host(void) {
    connection_host_t host;
    if (link == LINK_WIRED || (link == LINK_UNKNOWN && usb_up())) {
        host = CONNECTION_HOST_USB;
    } else {
        host = CONNECTION_HOST_BLUETOOTH;
    }
    if (connection_get_host_raw() != host) {
        connection_set_host_noeeprom(host);
    }
}

static void spi_slave_init(void) {
    SN_SYS1->AHBCLKEN |= (1U << 12); /* SPI0 */

    SN_SPI0->CTRL0 = (SN_SPI0->CTRL0 & ~0xF00U) | 0x700U; /* 8-bit */
    SN_SPI0->CTRL0 = (SN_SPI0->CTRL0 & ~0x8U) | 0x8U;     /* slave */
    SN_SPI0->CTRL0 &= ~(1U << 1);                         /* loopback off */
    SN_SPI0->CTRL0 &= ~(1U << 2);                         /* SDO on */
    SN_SPI0->CLKDIV = (SN_SPI0->CLKDIV & ~0xFFU) | 2U;
    SN_SPI0->CTRL1  = 0;
    SN_SPI0->CTRL0 &= ~(1U << 18); /* auto CS */

    SN_SPI0->IC = 0x0F;
    SN_SPI0->IE = 0x04; /* RX FIFO threshold */

    nvicDisableVector(SPI0_IRQn);
    /*
     * Above the RGB matrix PWM timer (CT16B1, priority 2), whose callback
     * also scans the shared matrix. A late reply here sends every following
     * byte one slot late, and the radio then sees a shifted frame.
     */
    nvicEnableVector(SPI0_IRQn, 1);

    txn_open = 0;
    rx_count = 0;
    tx_count = 0;

    SN_SPI0->CTRL0 |= 1U;                               /* enable */
    SN_SPI0->CTRL0 = (SN_SPI0->CTRL0 & ~0xC0U) | 0xC0U; /* FIFO reset */
}

static void ready_pulse(void) {
    burst = 0;
    gpio_write_pin_high(K530_BT_READY_PIN);
    gpio_set_pin_output(K530_BT_READY_PIN);
    gpio_write_pin_high(K530_BT_READY_PIN);
    wait_us(100);
    gpio_write_pin_low(K530_BT_READY_PIN);
    pulsed_at = timer_read();
    if (tx[0] == 0xAA && (tx[3] | tx[5] | tx[6] | tx[7] | tx[8] | tx[9] | tx[10]) != 0 && down_pulses < 255) {
        down_pulses++;
    }
}

static void copy_tx(void) {
    for (uint8_t i = 0; i < K530_BT_FRAME_LEN; i++) {
        tx[i] = frame[i];
    }
    tx[K530_BT_FRAME_LEN] = 0;
}

static void publish_frame(void) {
    frame[1] = link_mode();

    uint8_t sum = 0;
    for (uint8_t i = 0; i < K530_BT_FRAME_LEN - 1; i++) {
        sum += frame[i];
    }
    frame[K530_BT_FRAME_LEN - 1] = sum;
    publish_seq++;

    /*
     * A transfer already on the wire keeps the bytes it started with.
     * Copying here would replace a pressed key with the release report.
     */
    chSysLock();
    if (txn_open) {
        pulse_pending = 1;
        chSysUnlock();
        return;
    }
    copy_tx();
    pulse_pending = 0;
    chSysUnlock();

    /*
     * SPI is initialized at boot and again inside zero_pulse(). A new key
     * frame only pulses ready. Disabling the interrupt here shifts the frame.
     */
    repulses = 0;
    ready_pulse();
}

static void clear_frame(void) {
    for (uint8_t i = 0; i < K530_BT_FRAME_LEN; i++) {
        frame[i] = 0;
    }
}

/*
 * Stock 0x43C: zero the TX buffer, pulse ready, hold it about 0.2 ms,
 * drop it, then wait about 20 ms (delay(1) and delay(100) of a helper
 * that spins roughly 2000 cycles per unit). Every 0xA5 and 0xA6 follows it.
 */
static void zero_pulse(void) {
    chSysLock();
    for (uint8_t i = 0; i < K530_BT_FRAME_LEN + 1; i++) {
        tx[i] = 0;
    }
    pulse_pending = 0;
    chSysUnlock();

    spi_slave_init();
    gpio_write_pin_high(K530_BT_READY_PIN);
    gpio_set_pin_output(K530_BT_READY_PIN);
    gpio_write_pin_high(K530_BT_READY_PIN);
    wait_us(200);
    gpio_write_pin_low(K530_BT_READY_PIN);
    wait_ms(20);
}

static void remember_report(const report_keyboard_t *report) {
    key_mods     = report->mods;
    key_reserved = report->reserved;
    uint8_t nkeys = KEYBOARD_REPORT_KEYS;
    if (nkeys > 6) {
        nkeys = 6;
    }
    for (uint8_t i = 0; i < 6; i++) {
        key_codes[i] = (i < nkeys) ? report->keys[i] : 0;
    }
}

/*
 * Stock 0x360. Bytes 33 and 34 are the consumer usage from 0x20000042.
 * A key-up keeps a held media key, and a media release keeps held letters.
 */
static void publish_aa(uint8_t mods, uint8_t reserved, const uint8_t *keys) {
    clear_frame();
    frame[0] = 0xAA;
    frame[2] = K530_BT_REPORT_STATUS;
    frame[3] = mods;
    frame[4] = reserved;
    for (uint8_t i = 0; i < 6; i++) {
        frame[5 + i] = keys ? keys[i] : 0;
    }
    /*
     * Bytes 11..25 stay zero. Stock (0x442A) fills its overflow bitmap
     * there only once all six key slots are taken, and the radio forwards
     * those bytes as an extra report. bluetooth_can_send_nkro() is false,
     * so a seventh key is dropped instead of going into that bitmap.
     */
    frame[33] = (uint8_t)consumer_usage;
    frame[34] = (uint8_t)(consumer_usage >> 8);
    frame[26] = (uint8_t)((mouse_buttons & 0x1FU) | (mouse_mark << 5));
    frame[27] = (uint8_t)mouse_x;
    frame[28] = (uint8_t)(mouse_x >> 8);
    frame[29] = (uint8_t)mouse_y;
    frame[30] = (uint8_t)(mouse_y >> 8);
    frame[31] = (uint8_t)mouse_v;
    frame[32] = (uint8_t)mouse_h;
    frame[35] = 0x2C;
    frame[36] = 0x2C;

    bool keyed = mods != 0;
    for (uint8_t i = 0; keyed == false && keys && i < 6; i++) {
        keyed = keys[i] != 0;
    }
    if (keyed) {
        hold_key     = 1;
        keys_down    = true;
        need_release = 0;
        aa_sent_at   = timer_read();
    }
    publish_frame();
}

static void send_release(void) {
    hold_key     = 0;
    need_release = 0;
    key_mods     = 0;
    key_reserved = 0;
    for (uint8_t i = 0; i < 6; i++) {
        key_codes[i] = 0;
    }
    publish_aa(0, 0, key_codes);
}

/* Stock 0x30E: bytes 4 and 5 are always 10 and 25, byte 7 is the link byte. */
static void send_a5(uint8_t mode, uint8_t link_byte) {
    zero_pulse();
    clear_frame();
    frame[0] = 0xA5;
    frame[3] = mode;
    frame[4] = 10;
    frame[5] = 25;
    frame[7] = link_byte;
    publish_frame();
}

/* Stock 0x336: byte 3 is 1 to pair a new host, 2 to reconnect; byte 4 is the channel. */
static void send_a6(uint8_t action) {
    zero_pulse();
    clear_frame();
    frame[0]       = 0xA6;
    frame[3]       = action;
    frame[4]       = bt_channel;
    module_stat[1] = 0;
    sent_channel   = bt_channel;
    radio_state    = action;
    state_hold_at  = timer_read();
    state_hold_ms  = action == 1 ? K530_BT_PAIR_TIMEOUT_MS : K530_BT_RECONNECT_TIMEOUT_MS;
    connected_at   = 0;
    publish_frame();
}

static void set_radio_state(uint8_t state) {
    if (radio_state != 0 && state == 0) {
        connected_at = timer_read() | 1;
    }
    radio_state = state;
}

static void radio_state_task(void) {
    uint16_t seen = a7_seen_at;
    if (module_seen && seen != a7_used_at) {
        a7_used_at = seen;
        if (!state_hold_ms || (uint16_t)(seen - state_hold_at) >= 1000) {
            state_hold_ms = 0;
            set_radio_state(module_stat[1]);
        }
    }
    if (state_hold_ms && timer_elapsed(state_hold_at) > state_hold_ms) {
        state_hold_ms = 0;
        radio_state   = 0;
    }
}

/*
 * Stock 0x5512/0x5724: the BT1/BT2/BT3 switch is A6 (pull-up) sampled
 * with A7 driven low and then high. Both high is 0, low then high is 1,
 * both low is 2. Each phase is debounced 16 times.
 */
static uint8_t read_channel(void) {
    gpio_set_pin_output(A7);
    gpio_write_pin_low(A7);
    wait_us(10);
    bool with_low = gpio_read_pin(A6);
    gpio_write_pin_high(A7);
    wait_us(10);
    bool with_high = gpio_read_pin(A6);

    if (with_low && with_high) {
        return 0;
    }
    if (!with_low && with_high) {
        return 1;
    }
    if (!with_low && !with_high) {
        return 2;
    }
    return 0xFF;
}

/*
 * When the debounced channel differs from the last one sent for 50 slow
 * ticks, stock 0x56B6 sends the reconnect 0xA6 for it.
 */
static void channel_task(void) {
    if (timer_elapsed(channel_sampled_at) < 10) {
        return;
    }
    channel_sampled_at = timer_read();

    uint8_t sample = read_channel();
    if (sample == 0xFF) {
        return;
    }
    if (sample != channel_seen) {
        channel_seen  = sample;
        channel_count = 0;
        return;
    }
    if (channel_count < 16) {
        channel_count++;
        return;
    }
    if (bt_channel != sample) {
        bt_channel         = sample;
        channel_changed_at = timer_read();
    }
    if (link == LINK_WIRELESS && bt_channel != sent_channel && !pair_held && timer_elapsed(channel_changed_at) > 500 && !txn_open && !pulse_pending) {
        send_a6(2);
    }
}

static void enter_wired(bool from_boot) {
    if (link == LINK_WIRELESS) {
        consumer_usage = 0;
        mouse_buttons  = 0;
        mouse_x = mouse_y = mouse_v = mouse_h = 0;
        send_release();
        wait_ms(40);
    }
    link = LINK_WIRED;
    /* Boot always sends 3; the runtime switch picks 3 or 2 from P1.14. */
    send_a5((from_boot || p1_14_high()) ? 3 : 2, 1);
}

static void enter_wireless(bool from_boot) {
    bool was_wired = link == LINK_WIRED;
    link           = LINK_WIRELESS;
    if (!from_boot && was_wired) {
        send_a5(0, 2);
    }
}

/*
 * Stock 0x4180 and 0x6868: P1.13 low is wireless with no frame. P1.13 high
 * goes wired once USB enumerates and falls back to wireless otherwise.
 * Wired also requires USB here, so a battery-only board never tells the
 * radio to stand down whichever level its switch gives.
 */
static void link_task(void) {
    bool want_wired = gpio_read_pin(K530_BT_MODE_PIN) && usb_up();

    if (link == LINK_UNKNOWN) {
        if (txn_open || pulse_pending) {
            return;
        }
        if (want_wired) {
            enter_wired(true);
        } else if (!gpio_read_pin(K530_BT_MODE_PIN) || timer_elapsed(boot_at) > K530_BT_USB_WAIT_MS) {
            enter_wireless(true);
        }
        link_want_wired = want_wired;
        link_changed_at = timer_read();
        return;
    }

    if (want_wired != link_want_wired) {
        link_want_wired = want_wired;
        link_changed_at = timer_read();
        return;
    }
    if (timer_elapsed(link_changed_at) < K530_BT_LINK_DEBOUNCE_MS || txn_open || pulse_pending) {
        return;
    }
    if (want_wired && link != LINK_WIRED) {
        enter_wired(false);
    } else if (!want_wired && link != LINK_WIRELESS) {
        enter_wireless(false);
    }
}

static void load_keyboard(const report_keyboard_t *report) {
    remember_report(report);
    publish_aa(key_mods, key_reserved, key_codes);
}

OSAL_IRQ_HANDLER(SN32_SPI0_HANDLER) {
    OSAL_IRQ_PROLOGUE();

    if ((SN_SPI0->RIS & 0x04U) == 0) {
        OSAL_IRQ_EPILOGUE();
        return;
    }

    /* Threshold flag with an empty FIFO reads as 0x00 and would be latched
     * as the command. The radio's read request is 0xA2, which then follows.
     */
    if ((SN_SPI0->STAT & (1U << 2)) != 0) {
        SN_SPI0->IC = 0x04;
        OSAL_IRQ_EPILOGUE();
        return;
    }

    uint8_t data = (uint8_t)SN_SPI0->DATA;
    SN_SPI0->IC  = 0x04;

    if (!txn_open) {
        /*
         * Keep the first byte of a clock burst. A later 0xA2 or 0x20
         * inside that burst is payload, not a new command. Stock answers
         * the first clock with tx[0] for every opcode.
         */
        if (!burst) {
            burst         = 1;
            rx_count      = 0;
            rx_sum        = 0;
            tx_count      = 0;
            txn_open      = 1;
            first_byte    = data;
            txn_seq       = publish_seq;
            SN_SPI0->DATA = tx[0];
            tx_count      = 1;
            OSAL_IRQ_EPILOGUE();
            return;
        }
        SN_SPI0->DATA = 0;
        OSAL_IRQ_EPILOGUE();
        return;
    }

    uint8_t idx = rx_count++;
    if (idx < sizeof(rx)) {
        rx[idx] = data;
    }
    if (idx < 0x26) {
        rx_sum += data;
    }

    if (first_byte == 0x20) {
        if (rx_count < 2) {
            SN_SPI0->DATA = 0xBB;
        } else {
            SN_SPI0->DATA = (idx <= K530_BT_FRAME_LEN) ? rx_prev : 0;
        }
    } else if (first_byte == 0xA2) {
        SN_SPI0->DATA = (tx_count < K530_BT_FRAME_LEN) ? tx[tx_count] : 0;
    } else {
        SN_SPI0->DATA = 0;
    }
    tx_count++;
    rx_prev = data;

    if (rx_count > 0x26 && idx < K530_BT_FRAME_LEN) {
        txn_open = 0;
        burst    = 0;
        SN_SPI0->CTRL0 = (SN_SPI0->CTRL0 & ~0xC0U) | 0xC0U;

        if (first_byte == 0xA2 && txn_seq == publish_seq) {
            bool keyed = (tx[3] | tx[5] | tx[6] | tx[7] | tx[8] | tx[9] | tx[10]) != 0;
            if (tx[0] == 0xAA && keyed) {
                hold_key = 0;
            }
        }
        /* Stock 0x9DAA: a received frame whose byte 0 is 0xA7 is the status reply. */
        if (data == rx_sum && rx[0] == 0xA7) {
            module_stat[0] = rx[3];
            module_stat[1] = rx[4];
            module_stat[2] = rx[5];
            module_seen    = 1;
            a7_seen_at     = timer_read();
        }
    }

    OSAL_IRQ_EPILOGUE();
}

void bluetooth_init(void) {
    gpio_set_pin_output(K530_BT_READY_PIN);
    gpio_write_pin_low(K530_BT_READY_PIN);

    spi_slave_init();
    clear_frame();
    boot_at = timer_read();
    link    = LINK_UNKNOWN;
    select_host();
}

static void pair_task(void) {
    if (!pair_held || pair_long_sent || timer_elapsed(pair_pressed_at) < K530_BT_PAIR_HOLD_MS) {
        return;
    }
    if (txn_open || pulse_pending) {
        return;
    }
    pair_long_sent = true;
    send_a6(1);
}

void bluetooth_task(void) {
    /*
     * A short clock burst leaves the transfer open, and the ready line is
     * then held until 39 bytes arrive. The radio never gets another pulse.
     * Drop it once SCK has been idle.
     */
    bool idle = ((SN_SPI0->STAT & 0x14U) == 0x04U);
    if (idle) {
        if (timer_elapsed(txn_idle_at) > 5) {
            if (txn_open && rx_count < 39) {
                txn_open = 0;
            }
            burst = 0;
        }
    } else {
        txn_idle_at = timer_read();
    }

    link_task();
    select_host();
    channel_task();
    pair_task();
    radio_state_task();

    /*
     * Catch up pulses that the key handler's wait skipped, then release.
     * A held key already pulsed on every scan, so its release is not delayed.
     * The 50 ms cap still releases a press the radio never clocks.
     */
    if (link == LINK_WIRELESS && need_release && down_pulses < K530_BT_MIN_PULSES && !txn_open && !pulse_pending) {
        if (timer_elapsed(pulsed_at) >= K530_BT_REPULSE_MS) {
            ready_pulse();
        }
        return;
    }
    if (link == LINK_WIRELESS && need_release && down_pulses >= K530_BT_MIN_PULSES && (!hold_key || timer_elapsed(aa_sent_at) > 50) && !txn_open && !pulse_pending) {
        send_release();
        return;
    }
    if (pulse_pending && !txn_open) {
        pulse_pending = 0;
        chSysLock();
        copy_tx();
        chSysUnlock();
        repulses = 0;
        ready_pulse();
        return;
    }
    /*
     * The radio reads every key report but forwards only some of them.
     * Offer each one again for a while, read or not, so a press or key-up
     * is not lost.
     */
    if (link == LINK_WIRELESS && tx[0] == 0xAA && !txn_open && repulses < K530_BT_REPULSE_MAX && timer_elapsed(pulsed_at) >= K530_BT_REPULSE_MS) {
        repulses++;
        ready_pulse();
    }
}

bool bluetooth_is_connected(void) {
    /*
     * Auto host selection only routes reports here once this is true.
     * The module is quiet until it sees a frame, so this has to be true
     * before the first exchange. select_host() keeps USB while wired.
     */
    return true;
}

bool bluetooth_can_send_nkro(void) {
    /* Six keys over the radio. The overflow bitmap is left empty. */
    return false;
}

uint8_t bluetooth_keyboard_leds(void) {
    /* 0xA7 byte 3. Stock paints bit 1; the keymap reads the whole byte. */
    return module_stat[0];
}

static bool report_is_empty(const report_keyboard_t *report) {
    if (report->mods != 0) {
        return false;
    }
    uint8_t nkeys = KEYBOARD_REPORT_KEYS;
    if (nkeys > 6) {
        nkeys = 6;
    }
    for (uint8_t i = 0; i < nkeys; i++) {
        if (report->keys[i] != 0) {
            return false;
        }
    }
    return true;
}

void bluetooth_send_keyboard(report_keyboard_t *report) {
    if (link != LINK_WIRELESS) {
        return;
    }
    /*
     * The matrix reports "no keys" on every scan. Stock sends a frame only
     * when a key changes. A release is published later, once, from
     * bluetooth_task.
     */
    if (report_is_empty(report)) {
        remember_report(report);
        if (keys_down) {
            keys_down    = false;
            need_release = 1;
        }
        return;
    }
    /*
     * Repeats of a held key keep down_pulses. A new key set starts again,
     * so the release waits out its own pulses.
     */
    bool changed = !keys_down || report->mods != key_mods || report->reserved != key_reserved;
    if (!changed) {
        uint8_t nkeys = KEYBOARD_REPORT_KEYS;
        if (nkeys > 6) {
            nkeys = 6;
        }
        for (uint8_t i = 0; i < nkeys; i++) {
            if (report->keys[i] != key_codes[i]) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        down_pulses = 0;
    }
    keys_down    = true;
    hold_key     = 1;
    need_release = 0;
    aa_sent_at   = timer_read();
    load_keyboard(report);
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
    switch (keycode) {
        case K530_BT_PAIR:
            /* Stock action 0x2C (Esc on its Fn layer) is ignored while wired. */
            if (record->event.pressed) {
                pair_held       = link == LINK_WIRELESS;
                pair_long_sent  = false;
                pair_pressed_at = timer_read();
            } else if (pair_held) {
                pair_held = false;
                if (!pair_long_sent) {
                    send_a6(2);
                }
            }
            return false;
    }
    return true;
}

static void set_led(uint8_t led_min, uint8_t led_max, uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    if (led >= led_min && led < led_max) {
        rgb_matrix_set_color(led, r, g, b);
    }
}

/*
 * Bluetooth case LED while wireless, after stock 0x6694/0x674A: the
 * channel colour (red BT1, green BT2, blue BT3) blinks fast while
 * pairing (state 1), slowly while reconnecting (state 2), and shows
 * bright for a moment once the radio goes quiet. Stock then turns it
 * off; here it stays dim so the active slot is always visible. While
 * wired the keymap's own panel colour stays.
 */
static void bt_led(uint8_t led_min, uint8_t led_max) {
    if (link != LINK_WIRELESS) {
        return;
    }
    uint8_t v;
    if (radio_state == 1 || radio_state == 2) {
        uint16_t period = radio_state == 1 ? K530_BT_BLINK_FAST_MS : K530_BT_BLINK_SLOW_MS;
        v               = (timer_read() % period) < period / 2 ? 255 : 0;
    } else if (connected_at && timer_elapsed(connected_at) < K530_BT_CONNECTED_SHOW_MS) {
        v = 255;
    } else {
        connected_at = 0;
        v            = K530_BT_IDLE_LEVEL;
    }
    set_led(led_min, led_max, K530_BT_LED, bt_channel == 0 ? v : 0, bt_channel == 1 ? v : 0, bt_channel == 2 ? v : 0);
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }
    bt_led(led_min, led_max);
    return true;
}

void bluetooth_send_nkro(report_nkro_t *report) {
    (void)report;
}

void bluetooth_send_mouse(report_mouse_t *report) {
    /*
     * Buttons stay for as long as they are held. Movement is relative, so
     * it is cleared after this frame; a later key report must not repeat it.
     * A held direction sends the same delta again, and the padding bit
     * makes that next frame different from the last one.
     */
    bool moved    = report->x || report->y || report->v || report->h;
    mouse_buttons = report->buttons;
    mouse_x       = report->x;
    mouse_y       = report->y;
    mouse_v       = (int8_t)report->v;
    mouse_h       = (int8_t)report->h;
    if (link != LINK_WIRELESS) {
        return;
    }
    if (moved) {
        mouse_mark ^= 1U;
    }
    publish_aa(key_mods, key_reserved, key_codes);
    mouse_x = mouse_y = mouse_v = mouse_h = 0;
}

void bluetooth_send_consumer(uint16_t usage) {
    /*
     * Stock 0x4F50 stores the usage and queues 0xAA. Bytes 33 and 34 of
     * that frame carry it. 0xA3 never goes on the wire.
     */
    consumer_usage = usage;
    if (link != LINK_WIRELESS) {
        return;
    }
    publish_aa(key_mods, key_reserved, key_codes);
}

void bluetooth_send_system(uint16_t usage) {
    /*
     * Stock 0x4F84 queues 0xAA as well, but the 0xAA builder never reads
     * the system buffer at 0x20000045. Leave the key frame alone.
     */
    (void)usage;
}

void bluetooth_send_raw_hid(uint8_t *data, uint8_t length) {
    (void)data;
    (void)length;
}
