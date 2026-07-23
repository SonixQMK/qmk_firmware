#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SN32F260.h>
#include "ch.h"
#include "hal.h"
#include "color.h"
#include "wait.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"

#define I2C_SCL A4
#define I2C_SDA A5
#define I2C_SDB B0

#define I2C_SCL_IN readPin(I2C_SCL)
#define I2C_SDA_IN readPin(I2C_SDA)

#define I2C_SCL_HI  do { setPinOutput(I2C_SCL); writePinHigh(I2C_SCL); } while (0)
#define I2C_SCL_LO  do { setPinOutput(I2C_SCL); writePinLow(I2C_SCL); } while (0)
#define I2C_SCL_HIZ do { setPinInputHigh(I2C_SCL); } while (0)

#define I2C_SDA_HI  do { setPinOutput(I2C_SDA); writePinHigh(I2C_SDA); } while (0)
#define I2C_SDA_LO  do { setPinOutput(I2C_SDA); writePinLow(I2C_SDA); } while (0)
#define I2C_SDA_HIZ do { setPinInputHigh(I2C_SDA); } while (0)

/*
 * according to the spec, high SCL peroid 0.7us, low SCL peroid 1.3us
 *
 * i2c_delay 1 loop about 7 cycles. Under 48MHz, the actual delay is around 0.9us and 1.5us respectively.
 */
#define I2C_DELAY           i2c_delay(2)

#ifdef USE_FRAMEBUFFER
static RGB g_fb[DRIVER_LED_TOTAL];
#else
static uint8_t sel_frame[2] = {0xFF, 0xFF};
static uint8_t sel_frame_idx = 0;
#endif

extern uint8_t is_orgb_mode;

static __inline void i2c_delay(uint32_t loop)
{
    // #pragma GCC unroll 0
    for (int32_t i = 0; i < loop; i++)
        __NOP();
        // SN_WDT->FEED = 0x5AFA55AA;
}

void i2c_init(void)
{   
    // drive strength all gpio A 20ma
    SN_GPIO0->MODE |= 0xFFFF0000;

    I2C_SCL_HI;
    I2C_SDA_HIZ;

    setPinOutput(I2C_SDB);
    writePinHigh(I2C_SDB);
    i2c_delay(100);
}

// #pragma GCC push_options
// #pragma GCC optimize ("-O2")

static void i2c_process_bit(uint8_t *i2c_tx_byte)
{
    if (*i2c_tx_byte & 0x80)
    {
        I2C_SDA_HI;
    }
    else
    {
        I2C_SDA_LO;
    }

    *i2c_tx_byte = *i2c_tx_byte << 1;

    I2C_SCL_HI;
    I2C_DELAY;
    I2C_SCL_LO;
    I2C_DELAY;
}

static uint8_t i2c_writeb(uint8_t b)
{
	uint8_t fail = 0;

    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);
    i2c_process_bit(&b);

/*
    if (!is_ack)
        return 0;
*/
    /* ack bit */
    setPinInput(I2C_SDA);
    
    I2C_SCL_HI;
    I2C_DELAY;
    
    fail = I2C_SDA_IN;
    
    I2C_SCL_LO;
    I2C_DELAY;

    return fail;
}

static void i2c_start_phase(void)
{
    /* START */
    I2C_SDA_LO;
    I2C_DELAY;
    I2C_SCL_LO;
    I2C_DELAY;
}

static void i2c_stop_phase(void)
{
    I2C_SDA_LO;
    I2C_DELAY;

    /* STOP */
    I2C_SCL_HI;
    I2C_DELAY;
    I2C_SDA_HIZ;

    I2C_DELAY;
    I2C_DELAY;
}

// #pragma GCC pop_options

static uint8_t i2c_transaction(uint8_t i2c_addr_rw, uint8_t* i2c_data_ptr, uint8_t i2c_byte_ct)
{
    i2c_start_phase();

    i2c_writeb(i2c_addr_rw);

    for (uint8_t i = 0; i < i2c_byte_ct; i++)
        i2c_writeb(i2c_data_ptr[i]);

    i2c_stop_phase();

    return 0;
}

static uint8_t i2c_write_buf(uint8_t devid, uint8_t* data, uint8_t len)
{
    int32_t tries = 1;
    
    while ((tries-- > 0) && i2c_transaction(devid, data, len));
    
    return 0;
}

static void i2c_write_reg(uint8_t devid, uint8_t reg, uint8_t data)
{
    uint8_t i2c_data[2];
    
    i2c_data[0] = reg;
    i2c_data[1] = data;

    i2c_write_buf(devid, i2c_data, 2);
}

static void reset_rgb(int devid)
{
    i2c_write_reg(devid, 0xFD, 0x0B);

    i2c_write_reg(devid, 0x0A, 0x00);
    i2c_write_reg(devid, 0x00, 0x00);
    i2c_write_reg(devid, 0x01, 0x10);
    i2c_write_reg(devid, 0x05, 0x00);
    i2c_write_reg(devid, 0x06, 0x00);
    i2c_write_reg(devid, 0x08, 0x00);
    i2c_write_reg(devid, 0x09, 0x00);
    i2c_write_reg(devid, 0x0B, 0x00);
    i2c_write_reg(devid, 0x0D, 0x0F);
    i2c_write_reg(devid, 0x0E, 0x01);
    i2c_write_reg(devid, 0x14, 68);
    i2c_write_reg(devid, 0x15, 128);
    i2c_write_reg(devid, 0x0F, 153);

    i2c_write_reg(devid, 0xFD, 0);
    for (int32_t i = 0; i < 0x10; i++)
        i2c_write_reg(devid, i, 0xFF);
    /* skip blink control 0x10~0x1F as reset 0 */
    for (int32_t i = 0x20; i < 0xA0; i++)
        i2c_write_reg(devid, i, 0);

    i2c_write_reg(devid, 0xFD, 1);
    for (int32_t i = 0; i < 0x10; i++)
        i2c_write_reg(devid, i, 0xFF);
    /* skip blink control 0x10~0x1F as reset 0 */
    for (int32_t i = 0x20; i < 0xA0; i++)
        i2c_write_reg(devid, i, 0);

    i2c_write_reg(devid, 0xFD, 0xB);
    i2c_write_reg(devid, 0x0A, 1);
}

#ifdef USE_FRAMEBUFFER
// EE chip
static const uint8_t led_ee_map[80] = {
    // page 1
    255, 255, 255, 74, 104, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,

    58, 59, 60, 255, 255, 255, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,

    37, 38, 39, 40, 41, 42, 255, 255, 255, 43, 44, 45, 46, 47, 48, 49,

    // page 2
    16, 17, 18, 19, 20, 21, 22, 23, 24, 255, 255, 255, 25, 26, 27, 28,

    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 255, 255, 255, 255, 255
};

// E8 chip
static const uint8_t led_e8_map[80] = {
    // page 1
    255, 255, 255, 91, 92, 93, 255, 94, 255, 95, 96, 97, 255, 98, 99, 100,

    71, 72, 73, 255, 255, 255, 86, 87, 88, 89, 90, 101, 102, 103, 255, 255,

    52, 53, 54, 55, 56, 57, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

    // page 2
    30, 31, 32, 33, 34, 35, 36, 50, 51, 255, 255, 255, 255, 255, 255, 255,

    11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 29, 255, 255, 255, 255, 255
};

static void flush_led_fb(uint8_t devid, const uint8_t *map)
{
    int32_t led_idx = 0, i, j;
    uint8_t block[48];

    // page 1
    i2c_write_reg(devid, 0xFD, 0);

    i2c_start_phase();
    i2c_writeb(devid);
    i2c_writeb(0x20);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 16; j++)
        {
            uint8_t r, g, b;
            int32_t mi = map[led_idx++];

            if (mi >= DRIVER_LED_TOTAL)
                r = g = b = 0;
            else {
                r = g_fb[mi].r;
                g = g_fb[mi].g;
                b = g_fb[mi].b;
            }

            block[j] = r;
            block[j + 16] = g;
            block[j + 32] = b;
        }

        int32_t block_size;

        if (i <= 1)
            block_size = 48;
        else
            block_size = 32;            

        for (j = 0; j < block_size; j++)
            i2c_writeb(block[j]);
    }

    i2c_stop_phase();

    // page 2
    i2c_write_reg(devid, 0xFD, 1);

    // write the blue channel of last row in page 1
    i2c_start_phase();
    i2c_writeb(devid);
    i2c_writeb(0x20);
    for (i = 0; i < 16; i++)
        i2c_writeb(block[32 + i]);

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 16; j++)
        {
            uint8_t r, g, b;
            int32_t mi = map[led_idx++];

            if (mi >= DRIVER_LED_TOTAL)
                r = g = b = 0;
            else {
                r = g_fb[mi].r;
                g = g_fb[mi].g;
                b = g_fb[mi].b;
            }

            block[j] = r;
            block[j + 16] = g;
            block[j + 32] = b;
        }

        for (j = 0; j < 48; j++)
            i2c_writeb(block[j]);
    }

    i2c_stop_phase();
}

#else

/*
 * led index to RGB address
 */
static const uint8_t g_led_pos[DRIVER_LED_TOTAL] = {
/* 0*/ 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xC0,0xC1,0xC2,0xC3,0xC4,
/*16*/ 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x9C,0x9D,0x9E,0x9F,0xCA,0x90,0x91,0x92,0x93,0x94,0x95,0x96,
/*37*/ 0x60,0x61,0x62,0x63,0x64,0x65,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x97,0x98,0x60,0x61,0x62,0x63,0x64,0x65,
/*58*/ 0x30,0x31,0x32,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x30,0x31,0x32,
/*74*/ 0x03,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x36,0x37,0x38,0x39,0x3A,
/*91*/ 0x03,0x04,0x05,0x07,0x09,0x0A,0x0B,0x0D,0x0E,0x0F,0x3B,0x3C,0x3D
#ifdef KEYMAP_ISO
       ,0x04 /* KC_NUBS */
#endif    
};

/*
 * led index to chip selection table (0: E8, 1: EE)
 */
static const uint8_t g_led_chip[DRIVER_LED_TOTAL] = {
/* 0*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
/*16*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
/*37*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
/*58*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,
/*74*/    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
/*91*/    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0   
#ifdef KEYMAP_ISO
         ,1 /* KC_NUBS */
#endif
};

static void set_pwm(uint8_t dev, uint8_t addr, uint8_t value)
{
    /* >=0x80 for frame 2 otherwise frame 1 */
    if (addr >= 0x80) {
        if (sel_frame[sel_frame_idx] != 1) {
            i2c_write_reg(dev, 0xFD, 1);
            sel_frame[sel_frame_idx] = 1;
        }
        addr -= 0x80;
    }
    else if (sel_frame[sel_frame_idx] != 0) {
        i2c_write_reg(dev, 0xFD, 0);
        sel_frame[sel_frame_idx] = 0;
    }

    i2c_write_reg(dev, addr + 0x20, value);
}

static void _set_color_direct(int index, uint8_t r, uint8_t g, uint8_t b)
{   
    uint8_t dev;
    int l = g_led_pos[index];

    if (g_led_chip[index]) {
        dev = 0xEE;
        sel_frame_idx = 1;
    }
    else {
        dev = 0xE8;
        sel_frame_idx = 0;
    }

    set_pwm(dev, l, r);
    set_pwm(dev, l + 0x10, g);
    set_pwm(dev, l + 0x20, b);
}
#endif

void _set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
#ifdef USE_FRAMEBUFFER
    g_fb[index].r = r;
    g_fb[index].g = g;
    g_fb[index].b = b;
#else

#ifdef VIA_OPENRGB_HYBRID
    if (!is_orgb_mode && (index == 45 || index == 59 || index == 78))
        r = g = b = 255;
#endif

    _set_color_direct(index, r, g, b);
#endif
}

void _read_color(int index, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = 0;
    *g = 0;
    *b = 0;
}

void process_backlight(uint8_t devid, volatile LED_TYPE *states)
{
    static unsigned char state = 0;

    switch (state)
    {
        case 0: /* init RGB chips */
            i2c_init();
            reset_rgb(0xE8);
            reset_rgb(0xEE);

            state = 1;
            break;

        case 1:
        #ifdef USE_FRAMEBUFFER
            
        #ifdef VIA_OPENRGB_HYBRID            
            if (!is_orgb_mode) {
                rgb_matrix_set_color(78, 255, 255, 255);
                rgb_matrix_set_color(45, 255, 255, 255);
                rgb_matrix_set_color(59, 255, 255, 255);                    
            }
        #endif

            flush_led_fb(0xEE, led_ee_map);
            flush_led_fb(0xE8, led_e8_map);
        #endif
            break;
    }
}