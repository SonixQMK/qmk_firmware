/* Copyright 2024 Dimitris Mantzouranis <d3xter93@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "matrix.h"
#include "rgb_matrix.h"
#include "sn32f2xx.h"

#define ROWS_PER_HAND (MATRIX_ROWS)

#if !defined(MATRIX_IO_DELAY)
#    define MATRIX_IO_DELAY 30
#endif

#define SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH PWM_OUTPUT_ACTIVE_HIGH
#define SN32F2XX_PWM_OUTPUT_ACTIVE_LOW PWM_OUTPUT_ACTIVE_LOW

#define SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH PWM_OUTPUT_ACTIVE_HIGH
#define SN32F2XX_RGB_OUTPUT_ACTIVE_LOW PWM_OUTPUT_ACTIVE_LOW

#define HARDWARE_PWM 0
#define SOFTWARE_PWM 1

/*
    Default configuration example

    COLS key / led
    SS8050 transistors NPN driven low
    base      - GPIO
    collector - LED Col pins
    emitter   - VDD

    VDD     GPIO
    (E)     (B)
     |  PNP  |
     |_______|
         |
         |
        (C)
        LED

    ROWS RGB
    SS8550 transistors PNP driven high
    base      - GPIO
    collector - LED RGB row pins
    emitter   - GND

        LED
        (C)
         |
         |
      _______
     |  NPN  |
     |       |
    (B)     (E)
    GPIO    GND
*/
#if (SN32F2XX_PWM_DIRECTION == COL2ROW)
static uint8_t chan_col_order[SN32F2XX_RGB_MATRIX_COLS] = {0}; // track the channel col order
static uint8_t current_row                              = 0;   // LED row scan counter
static uint8_t current_key_row                          = 0;   // key row scan counter
#    if (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
static uint8_t led_duty_cycle[SN32F2XX_RGB_MATRIX_COLS] = {0}; // track the channel duty cycle
#    endif
#elif (SN32F2XX_PWM_DIRECTION == ROW2COL)
/* make sure to `#define MATRIX_UNSELECT_DRIVE_HIGH` in this configuration*/
static uint8_t chan_row_order[SN32F2XX_RGB_MATRIX_ROWS_HW] = {0}; // track the channel row order
static uint8_t current_key_col                             = 0;   // key col scan counter
static uint8_t last_key_col                                = 0;   // key col scan counter
#    if (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
static uint8_t led_duty_cycle[SN32F2XX_RGB_MATRIX_ROWS_HW] = {0}; // track the channel duty cycle
#    endif
#endif
#if (DIODE_DIRECTION == ROW2COL)
static matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
#endif
#if defined(SHARED_MATRIX)
extern matrix_row_t  raw_matrix[MATRIX_ROWS];    // raw values
extern matrix_row_t  matrix[MATRIX_ROWS];        // debounced values
static matrix_row_t  shared_matrix[MATRIX_ROWS]; // scan values
static volatile bool matrix_locked  = false;     // matrix update check
static volatile bool matrix_scanned = false;
#endif // SHARED MATRIX
static const uint32_t periodticks                               = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
static const uint32_t freq                                      = (RGB_MATRIX_HUE_STEP * RGB_MATRIX_SAT_STEP * RGB_MATRIX_VAL_STEP * RGB_MATRIX_SPD_STEP * RGB_MATRIX_LED_PROCESS_LIMIT);
static const pin_t    led_row_pins[SN32F2XX_RGB_MATRIX_ROWS_HW] = SN32F2XX_RGB_MATRIX_ROW_PINS; // We expect a R,B,G order here
static const pin_t    led_col_pins[SN32F2XX_RGB_MATRIX_COLS]    = SN32F2XX_RGB_MATRIX_COL_PINS;
static RGB            led_state[SN32F2XX_LED_COUNT];     // led state buffer
static RGB            led_state_buf[SN32F2XX_LED_COUNT]; // led state buffer
bool                  led_state_buf_update_required = false;
#ifdef UNDERGLOW_RBG // handle underglow with flipped B,G channels
static const uint8_t underglow_leds[UNDERGLOW_LEDS] = UNDERGLOW_IDX;
#endif

#if defined(SHARED_MATRIX)
void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
    for (int i = 0; i < TIME_US2I(MATRIX_IO_DELAY); ++i) {
        __asm__ volatile("" ::: "memory");
    }
}
bool matrix_can_read(void) {
    return matrix_scanned;
}
#endif // SHARED_MATRIX

static void rgb_callback(PWMDriver *pwmp);

#if !defined(SN32F2)
#    error Driver is MCU specific to the Sonix SN32F2 family.
#endif // !defined(SN32F2)

#if defined(SN32F240B)
/* PWM configuration structure. We use timer CT16B1 with 24 channels. */
static PWMConfig pwmcfg = {
    freq,        /* PWM clock frequency. */
    periodticks, /* PWM period = periodticks * (1 / CH_CFG_ST_FREQUENCY) ≈ 255 * (1 / 187500) ≈ 1.36 ms */
    NULL,        /* RGB Callback */
    {
        /* Default all channels to disabled - Channels will be configured during init */
        [0 ... PWM_CHANNELS - 1] = {PWM_OUTPUT_DISABLED, NULL, 0},
    },
    0 /* HW dependent part.*/
};

static void rgb_ch_ctrl(PWMConfig *cfg) {
    /* Enable PWM function, IOs and select the PWM modes for the LED pins */
#    if (SN32F2XX_PWM_DIRECTION == COL2ROW)
    for (uint8_t i = 0; i < SN32F2XX_RGB_MATRIX_COLS; i++) {
#        if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P2.15 can be used as pwm output
        if (led_col_pins[i] > C15) continue;
#        endif // SN32F2XX_PWM_CONTROL
        /* We use a trick here, according to pfpa table of sn32f240b datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%24
             pfpa = 1, when (x*16+y)>23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_col_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(B0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_col_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_col_order[i] = ch_idx;
#    elif (SN32F2XX_PWM_DIRECTION == ROW2COL)
    for (uint8_t i = 0; i < SN32F2XX_RGB_MATRIX_ROWS_HW; i++) {
#        if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P2.15 can be used as pwm output
        if (led_row_pins[i] > C15) continue;
#        endif // SN32F2XX_PWM_CONTROL
        /* We use a trick here, according to pfpa table of sn32f240b datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%24
             pfpa = 1, when (x*16+y)>23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_row_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(B0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_row_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_row_order[i] = ch_idx;
#    endif     // SN32F2XX_PWM_DIRECTION
#    if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        cfg->channels[ch_idx].pfpamsk = pio_value > (PWM_CHANNELS - 1);
        cfg->channels[ch_idx].mode    = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL;
#    endif // SN32F2XX_PWM_CONTROL
    }
}
#elif defined(SN32F260)
/* PWM configuration structure. We use timer CT16B1 with 23 channels. */
static const PWMConfig pwmcfg = {
    freq,         /* PWM clock frequency. */
    periodticks,  /* PWM period (in ticks) 1S (1/10kHz=0.1mS 0.1ms*10000 ticks=1S) */
    rgb_callback, /* led Callback */
    .channels =
        {
            /* Default all channels to disabled */
            [0 ... PWM_CHANNELS - 1] = {.mode = PWM_OUTPUT_DISABLED},
/* Enable selected channels */
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_0)
            [0]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_0
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_1)
            [1]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_1
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_2)
            [2]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_2
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_3)
            [3]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_3
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_4)
            [4]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_4
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_5)
            [5]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_5
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_6)
            [6]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_6
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_7)
            [7]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_7
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_8)
            [8]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_8
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_9)
            [9]  = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_9
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_10)
            [10] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_10
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_11)
            [11] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_11
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_12)
            [12] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_12
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_13)
            [13] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_13
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_14)
            [14] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_14
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_15)
            [15] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_15
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_16)
            [16] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_16
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_17)
            [17] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_17
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_18)
            [18] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_18
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_19)
            [19] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_19
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_20)
            [20] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_20
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_21)
            [21] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_21
#    if defined(SN32F2XX_ACTIVATE_PWM_CHAN_22)
            [22] = {.mode = SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL},
#    endif // SN32F2XX_ACTIVATE_PWM_CHAN_22
        },
    0 /* HW dependent part.*/
};

static void rgb_ch_ctrl(void) {
    /* Enable PWM function, IOs and select the PWM modes for the LED pins */
#    if (SN32F2XX_PWM_DIRECTION == COL2ROW)
    for (uint8_t i = 0; i < SN32F2XX_RGB_MATRIX_COLS; i++) {
#        if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P0.15 and P3.0 to P3.8 can be used as pwm output
        if (led_col_pins[i] > A15 && led_col_pins[i] < D0) continue;
#        endif // SN32F2XX_PWM_CONTROL
        /* We use a trick here, according to pfpa table of sn32f260 datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_col_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(D0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_col_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_col_order[i] = ch_idx;
#    elif (SN32F2XX_PWM_DIRECTION == ROW2COL)
    for (uint8_t i = 0; i < SN32F2XX_RGB_MATRIX_ROWS_HW; i++) {
#        if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        // Only P0.0 to P0.15 and P3.0 to P3.8 can be used as pwm output
        if (led_row_pins[i] > A15 && led_row_pins[i] < D0) continue;
#        endif // SN32F2XX_PWM_CONTROL
        /* We use a trick here, according to pfpa table of sn32f260 datasheet,
           pwm channel and pfpa of pin Px.y can be calculated as below:
             channel = (x*16+y)%23
        */
        uint8_t pio_value = ((uint32_t)(PAL_PORT(led_row_pins[i])) - (uint32_t)(PAL_PORT(A0))) / ((uint32_t)(PAL_PORT(D0)) - (uint32_t)(PAL_PORT(A0))) * PAL_IOPORTS_WIDTH + PAL_PAD(led_row_pins[i]);
        uint8_t ch_idx    = pio_value % PWM_CHANNELS;
        chan_row_order[i] = ch_idx;
#    endif     // SN32F2XX_PWM_DIRECTION
    }
}
#else
#    error Unsupported MCU. Driver instance cant be configured.
#endif // chip selection

static void shared_matrix_rgb_enable(void) {
#if !defined(SN32F260)
    pwmcfg.callback = rgb_callback;
#endif // SN32F260 needs static allocation
    pwmEnablePeriodicNotification(&PWMD1);
}

#if defined(SHARED_MATRIX)
static void shared_matrix_scan_keys(matrix_row_t current_matrix[], uint8_t current_key, uint8_t last_key) {
    // Scan the key matrix row or col, depending on DIODE_DIRECTION
    static uint8_t first_scanned;
    if (!matrix_scanned) {
        if (!matrix_locked) {
            matrix_locked = true;
            first_scanned = current_key;
        } else {
            if ((last_key != current_key) && (current_key == first_scanned)) {
                matrix_locked = false;
            }
        }
        if (matrix_locked) {
#    if (DIODE_DIRECTION == COL2ROW)
#        if (SN32F2XX_PWM_DIRECTION == DIODE_DIRECTION)
            matrix_read_cols_on_row(current_matrix, current_key);
#        else
            // For each row...
            for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
                matrix_read_cols_on_row(current_matrix, row_index);
            }

#        endif // DIODE_DIRECTION == SN32F2XX_PWM_DIRECTION
#    elif (DIODE_DIRECTION == ROW2COL)
#        if (SN32F2XX_PWM_DIRECTION == DIODE_DIRECTION)
            matrix_read_rows_on_col(current_matrix, current_key, row_shifter);
#        else
            // For each col...
            matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
            for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
                matrix_read_rows_on_col(current_matrix, current_key, row_shifter);
            }
#        endif // SN32F2XX_PWM_DIRECTION
#    endif     // DIODE_DIRECTION
            matrix_scanned = true;
        }
    }
}
#endif // SHARED_MATRIX

#if (SN32F2XX_PWM_DIRECTION == COL2ROW)

static void shared_matrix_rgb_disable_output(void) {
    // Disable PWM outputs on column pins
    for (uint8_t y = 0; y < SN32F2XX_RGB_MATRIX_COLS; y++) {
#    if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        pwmDisableChannel(&PWMD1, chan_col_order[y]);
#    elif (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
        gpio_set_pin_input(led_col_pins[y]);
#    endif // SN32F2XX_PWM_CONTROL
    }
    // Disable LED outputs on RGB channel pins
    for (uint8_t x = 0; x < SN32F2XX_RGB_MATRIX_ROWS_HW; x++) {
#    if (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_low(led_row_pins[x]);
#    elif (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_high(led_row_pins[x]);
#    endif // SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL
    }
}

static void update_pwm_channels(PWMDriver *pwmp) {
    // Advance to the next LED RGB channels
    current_row++;
    /* Check if counter has wrapped around, reset before the next pass */
    if (current_row == SN32F2XX_RGB_MATRIX_ROWS_HW) current_row = 0;
#    if defined(SHARED_MATRIX)
    uint8_t last_key_row = current_key_row;
#    endif // SHARED_MATRIX
    // Advance to the next key matrix row
#    if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
    if (current_row % SN32F2XX_RGB_MATRIX_ROW_CHANNELS == 2) current_key_row++;
#    elif (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
    if (current_row % SN32F2XX_RGB_MATRIX_ROW_CHANNELS == 0) current_key_row++;
#    endif // SN32F2XX_PWM_CONTROL
    /* Check if counter has wrapped around, reset before the next pass */
    if (current_key_row == SN32F2XX_RGB_MATRIX_ROWS) current_key_row = 0;
    // Disable LED output before scanning the key matrix
    if (current_key_row < ROWS_PER_HAND) {
        shared_matrix_rgb_disable_output();
#    if defined(SHARED_MATRIX)
        shared_matrix_scan_keys(shared_matrix, current_key_row, last_key_row);
#    endif // SHARED_MATRIX
    }
    bool enable_pwm_output = false;
    for (uint8_t current_key_col = 0; current_key_col < SN32F2XX_RGB_MATRIX_COLS; current_key_col++) {
        uint8_t led_index = g_led_config.matrix_co[current_key_row][current_key_col];
#    if (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
        if (led_index >= SN32F2XX_LED_COUNT) continue;
#    endif // SN32F2XX_PWM_CONTROL
        // Check if we need to enable RGB output
        if (led_state[led_index].b > 0) enable_pwm_output |= true;
        if (led_state[led_index].g > 0) enable_pwm_output |= true;
        if (led_state[led_index].r > 0) enable_pwm_output |= true;
            // Update matching RGB channel PWM configuration
#    if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        switch (current_row % SN32F2XX_RGB_MATRIX_ROW_CHANNELS) {
            case 0:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].b);
                break;
            case 1:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].g);
                break;
            case 2:
                pwmEnableChannel(pwmp, chan_col_order[current_key_col], led_state[led_index].r);
                break;
            default:;
        }
#    elif (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
        switch (current_row % SN32F2XX_RGB_MATRIX_ROW_CHANNELS) {
            case 0:
                led_duty_cycle[current_key_col] = led_state[led_index].r;
                break;
            case 1:
                led_duty_cycle[current_key_col] = led_state[led_index].b;
                break;
            case 2:
                led_duty_cycle[current_key_col] = led_state[led_index].g;
                break;
            default:;
        }
#    endif
    }
    // Enable RGB output
    if (enable_pwm_output) {
#    if (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_high(led_row_pins[current_row]);
#    elif (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_low(led_row_pins[current_row]);
#    endif
    }
}
#elif (SN32F2XX_PWM_DIRECTION == ROW2COL)

static void shared_matrix_rgb_disable_output(void) {
    // Disable LED outputs on RGB channel pins
    for (uint8_t x = 0; x < SN32F2XX_RGB_MATRIX_COLS; x++) {
#    if (DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION)
        gpio_set_pin_input(led_col_pins[x]);
#    endif // DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION
        // Unselect all columns before scanning the key matrix
#    if (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_high(led_col_pins[x]);
#    elif (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_low(led_col_pins[x]);
#    endif
    }
#    if (DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION)
    // Disable PWM outputs on row pins
    for (uint8_t x = 0; x < SN32F2XX_RGB_MATRIX_ROWS_HW; x++) {
#        if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        pwmDisableChannel(&PWMD1, chan_row_order[x]);
#        endif // SN32F2XX_PWM_CONTROL
#        if (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_low(led_row_pins[x]);
#        elif (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_high(led_row_pins[x]);
#        endif // SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL
    }
#    endif     // DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION
}

static void update_pwm_channels(PWMDriver *pwmp) {
    /* Advance to the next LED RGB channel and get ready for the next pass */
    last_key_col = current_key_col;
    current_key_col++;
    /* Check if counter has wrapped around, reset before the next pass */
#    if (DIODE_DIRECTION == ROW2COL)
    if (current_key_col < MATRIX_COLS) row_shifter <<= 1;
    if (current_key_col == MATRIX_COLS) row_shifter = MATRIX_ROW_SHIFTER;
#    endif // DIODE_DIRECTION == ROW2COL
    if (current_key_col == SN32F2XX_RGB_MATRIX_COLS) current_key_col = 0;
    // Disable LED output before scanning the key matrix
    if (current_key_col < MATRIX_COLS) {
        shared_matrix_rgb_disable_output();
#    if defined(SHARED_MATRIX)
        shared_matrix_scan_keys(shared_matrix, current_key_col, last_key_col);
#    endif // SHARED_MATRIX
    }

    bool enable_pwm_output = false;
    for (uint8_t current_key_row = 0; current_key_row < MATRIX_ROWS; current_key_row++) {
        uint8_t led_index = g_led_config.matrix_co[current_key_row][current_key_col];
#    if (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
        if (led_index >= SN32F2XX_LED_COUNT) continue;
#    endif
        uint8_t led_row_id = (current_key_row * SN32F2XX_RGB_MATRIX_ROW_CHANNELS);
        // Check if we need to enable RGB output
        if (led_state[led_index].b > 0) enable_pwm_output |= true;
        if (led_state[led_index].g > 0) enable_pwm_output |= true;
        if (led_state[led_index].r > 0) enable_pwm_output |= true;
            // Update matching RGB channel PWM configuration
#    if (SN32F2XX_PWM_CONTROL == HARDWARE_PWM)
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 0)], led_state[led_index].r);
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 1)], led_state[led_index].b);
        pwmEnableChannelI(pwmp, chan_row_order[(led_row_id + 2)], led_state[led_index].g);
    }
    // Enable RGB output
    if (enable_pwm_output) {
        gpio_set_pin_output_push_pull(led_col_pins[last_key_col]);
#        if (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_high(led_col_pins[last_key_col]);
#        elif (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_low(led_col_pins[last_key_col]);
#        endif // SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL
    }
#    elif (SN32F2XX_PWM_CONTROL == SOFTWARE_PWM)
        led_duty_cycle[(led_row_id + 0)] = led_state[led_index].r;
        led_duty_cycle[(led_row_id + 1)] = led_state[led_index].b;
        led_duty_cycle[(led_row_id + 2)] = led_state[led_index].g;
    }
    // Enable RGB output
    if (enable_pwm_output) {
        gpio_set_pin_output_push_pull(led_col_pins[last_key_col]);
#        if (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH)
        gpio_write_pin_high(led_col_pins[current_key_col]);
#        elif (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW)
        gpio_write_pin_low(led_col_pins[current_key_col]);
#        endif // SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL
    }
#    endif     // SN32F2XX_PWM_CONTROL
}
#endif         // SN32F2XX_PWM_DIRECTION == ROW2COL

static void rgb_callback(PWMDriver *pwmp) {
    // Disable the interrupt
    pwmDisablePeriodicNotification(pwmp);
#if ((SN32F2XX_PWM_CONTROL == SOFTWARE_PWM) && (SN32F2XX_PWM_DIRECTION == COL2ROW))
    for (uint8_t pwm_cnt = 0; pwm_cnt < (SN32F2XX_RGB_MATRIX_COLS * RGB_MATRIX_HUE_STEP); pwm_cnt++) {
        uint8_t pwm_index = (pwm_cnt % SN32F2XX_RGB_MATRIX_COLS);
        if (((uint16_t)(pwmp->ct->TC) < ((uint16_t)(led_duty_cycle[pwm_index] + periodticks))) && (led_duty_cycle[pwm_index] > 0)) {
            gpio_set_pin_output_push_pull(led_col_pins[pwm_index]);
#    if (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_LOW)
            gpio_write_pin_low(led_col_pins[pwm_index]);
        } else {
            gpio_set_pin_input_high(led_col_pins[pwm_index]);
#    elif (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH)
            gpio_write_pin_high(led_col_pins[pwm_index]);
        } else {
            gpio_set_pin_input_low(led_col_pins[pwm_index]);
#    endif // SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL
        }
    }
#elif ((SN32F2XX_PWM_CONTROL == SOFTWARE_PWM) && (SN32F2XX_PWM_DIRECTION == ROW2COL))
    for (uint8_t pwm_cnt = 0; pwm_cnt < (SN32F2XX_RGB_MATRIX_ROWS_HW * RGB_MATRIX_HUE_STEP); pwm_cnt++) {
        uint8_t pwm_index = (pwm_cnt % SN32F2XX_RGB_MATRIX_ROWS_HW);
        if (((uint16_t)(pwmp->ct->TC) < ((uint16_t)(led_duty_cycle[pwm_index] + periodticks))) && (led_duty_cycle[pwm_index] > 0)) {
#    if (DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION)
            gpio_set_pin_output_push_pull(led_row_pins[pwm_index]);
#    endif // DIODE_DIRECTION != SN32F2XX_PWM_DIRECTION

#    if (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_LOW)
            gpio_write_pin_low(led_row_pins[pwm_index]);
        } else {
            gpio_write_pin_high(led_row_pins[pwm_index]);
#    elif (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH)
            gpio_write_pin_high(led_row_pins[pwm_index]);
        } else {
            gpio_write_pin_low(led_row_pins[pwm_index]);
#    endif // SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL
        }
    }
#endif
    // Scan the rgb and key matrix
    update_pwm_channels(pwmp);
    chSysLockFromISR();
    // Advance the timer to just before the wrap-around, that will start a new PWM cycle
    pwm_lld_change_counter(pwmp, UINT16_MAX);
    // Enable the interrupt
    pwmEnablePeriodicNotificationI(pwmp);
    chSysUnlockFromISR();
}

void sn32f2xx_init(void) {
    for (uint8_t x = 0; x < SN32F2XX_RGB_MATRIX_ROWS_HW; x++) {
        gpio_set_pin_output_push_pull(led_row_pins[x]);
#if ((SN32F2XX_PWM_DIRECTION == COL2ROW) && (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH) || (SN32F2XX_PWM_DIRECTION == ROW2COL) && (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_HIGH))
        gpio_write_pin_low(led_row_pins[x]);
#elif ((SN32F2XX_PWM_DIRECTION == COL2ROW) && (SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL == SN32F2XX_RGB_OUTPUT_ACTIVE_LOW) || (SN32F2XX_PWM_DIRECTION == ROW2COL) && (SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL == SN32F2XX_PWM_OUTPUT_ACTIVE_LOW))
        gpio_write_pin_high(led_row_pins[x]);
#endif // SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL
    }

    // Determine which PWM channels we need to control
#if defined(SN32F240B)
    rgb_ch_ctrl(&pwmcfg);
#elif defined(SN32F260)
    rgb_ch_ctrl();
#endif // chip selection

#if defined(SHARED_MATRIX)
    // initialize matrix state: all keys off
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        shared_matrix[i] = 0;
    }
#endif // SHARED_MATRIX

    pwmStart(&PWMD1, &pwmcfg);
    shared_matrix_rgb_enable();
}

void sn32f2xx_flush(void) {
    if (led_state_buf_update_required) {
        memcpy(led_state, led_state_buf, sizeof(RGB) * SN32F2XX_LED_COUNT);
        led_state_buf_update_required = false;
    }
}

void sn32f2xx_set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t color_r = r * SN32F2XX_LED_OUTPUT_LUMINOSITY_R;
    uint8_t color_g = g * SN32F2XX_LED_OUTPUT_LUMINOSITY_G;
    uint8_t color_b = b * SN32F2XX_LED_OUTPUT_LUMINOSITY_B;

#ifdef UNDERGLOW_RBG
    bool flip_gb = false;
    for (uint8_t led_id = 0; led_id < UNDERGLOW_LEDS; led_id++) {
        if (underglow_leds[led_id] == index) {
            flip_gb = true;
        }
    }
    if (flip_gb) {
        if (led_state_buf[index].r == color_r && led_state_buf[index].b == color_g && led_state_buf[index].g == color_b) {
            return;
        }

        led_state_buf[index].r        = color_r;
        led_state_buf[index].b        = color_g;
        led_state_buf[index].g        = color_b;
        led_state_buf_update_required = true;
    } else {
#endif // UNDERGLOW_RBG
        if (led_state_buf[index].r == color_r && led_state_buf[index].b == color_b && led_state_buf[index].g == color_g) {
            return;
        }

        led_state_buf[index].r        = color_r;
        led_state_buf[index].b        = color_b;
        led_state_buf[index].g        = color_g;
        led_state_buf_update_required = true;
#ifdef UNDERGLOW_RBG
    }
#endif // UNDERGLOW_RBG
}

void sn32f2xx_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < SN32F2XX_LED_COUNT; i++) {
        sn32f2xx_set_color(i, r, g, b);
    }
}

#if defined(SHARED_MATRIX)
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (!matrix_scanned) return false; // Nothing to process until we have the matrix scanned

    bool changed = memcmp(raw_matrix, shared_matrix, sizeof(shared_matrix)) != 0;
    if (changed) memcpy(raw_matrix, shared_matrix, sizeof(shared_matrix));

    matrix_scanned = false;

    return changed;
}
#endif // SHARED_MATRIX