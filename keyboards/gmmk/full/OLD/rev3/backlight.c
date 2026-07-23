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

#include "SPI.h"

extern void SPI0_Flush(void);
extern void SPI0_Read3(unsigned char b1, unsigned char b2, unsigned char *b3);

#ifdef USE_FRAMEBUFFER
static RGB g_fb[DRIVER_LED_TOTAL];
#endif

#define SDB B0
#define SPI_DELAY()   for (int32_t _del = 0; _del < 10; _del++) __NOP();

static int g_cs_pin = 0;

void spi_init(void)
{
    SPI0_Init();
    SPI0_Enable();
}

void spi_set_cspin(int pin)
{   
    if (g_cs_pin == pin)
        return;

    writePinHigh(B2); 
    setPinOutput(B2);    
    writePinHigh(B1); 
    setPinOutput(B1);

    g_cs_pin = pin;
}

void spi_read3(unsigned char b1, unsigned char b2, unsigned char *b3)
{    
    writePinLow(g_cs_pin);    
    SPI0_Read3(b1, b2, b3);    
    writePinHigh(g_cs_pin);
}

void spi_write(uint8_t *data_p, int len)
{    
    writePinLow(g_cs_pin);    
    SPI0_Write(data_p, len);
    SPI0_Flush();
    writePinHigh(g_cs_pin);
}

void spi_w2(uint8_t page, uint8_t addr)
{
    uint8_t c[2];
    c[0] = page | 0x20;
    c[1] = addr;
    spi_write(c, 2);
}

void spi_w3(uint8_t page, uint8_t addr, uint8_t data)
{
    uint8_t c[4];
    c[0] = page | 0x20;
    c[1] = addr;
    c[2] = data;
    spi_write(c, 3);
}

void spi_r3(uint8_t page, uint8_t addr, uint8_t *data)
{
    uint8_t c[4];
    c[0] = page | 0x20;
    c[1] = addr;    
    spi_read3(c[0], c[1], data);
}

/* 
 * LED index to RGB address
 * >100 means it belongs to pin B1 chipselected SN2735 chip, the real addr is minus by 100 
 */
#ifdef KEYMAP_ISO
 
/* ISO */
static const uint8_t g_led_pos[DRIVER_LED_TOTAL] = {
/* 0*/    0,   2,   3,   4,   5,   6,   7,   8,   9,  10,   11,  12,  13,  14,  15,  16,
/*16*/  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,  110, 111, 112, 113,  21,  22,  23,  24,   25,   26,  27,   
/*37*/  116, 117, 118, 119, 120, 121, 122, 123, 124, 125,  126, 127, 128, 129,  32,  33,  34,  35,   36,   37,  38, 
/*58*/  132, 133, 134, 135, 136, 137, 138, 139, 140, 141,  142, 143, 145,  42,  43,  44,    
/*74*/  148, 150, 151, 152, 153, 154, 155, 156, 157, 158,  159, 161,  49,  51,  52,  53,  54,
/*91*/  114, 115, 130, 131, 146, 147, 162, 163,  55,  56,   57,  59,  60, 149 /* KC_NUBS */
};

#else

/* ANSI */
static const uint8_t g_led_pos[DRIVER_LED_TOTAL] = {
/* 0*/    0,   2,   3,   4,   5,   6,   7,   8,   9,  10,   11,  12,  13,  14,  15,  16,
/*16*/  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,  110, 111, 112, 113,  21,  22,  23,  24,   25,   26,  27,   
/*37*/  116, 117, 118, 119, 120, 121, 122, 123, 124, 125,  126, 127, 128, 129,  32,  33,  34,  35,   36,   37,  38, 
/*58*/  132, 133, 134, 135, 136, 137, 138, 139, 140, 141,  142, 143, 145,  42,  43,  44,    
/*74*/  148, 150, 151, 152, 153, 154, 155, 156, 157, 158,  159, 161,  49,  51,  52,  53,  54,
/*91*/  114, 115, 130, 131, 146, 147, 162, 163,  55,  56,   57,  59,  60
};
    
#endif

#ifdef USE_FRAMEBUFFER
// B1
static const uint8_t led_b1_map[64] = {
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 91, 92,
    37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 93, 94,
    58, 59, 60, 61, 62, 63, 64, 65,
    66, 67, 68, 69, 255, 70, 95, 96,
    74, 104, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 255, 85, 97, 98
};

// B2
static const uint8_t led_b2_map[64] = {
    0, 255, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14,
    15, 255, 255, 255, 255, 30, 31, 32,
    33, 34, 35, 36, 255, 255, 255, 255,
    51, 52, 53, 54, 55, 56, 57, 255,
    255, 255, 71, 72, 73, 255, 255, 255,
    255, 86, 255, 87, 88, 89, 90, 99,
    100, 101, 255, 102, 103, 255, 255, 255
};

// #pragma GCC push_options
// #pragma GCC optimize ("-O2")

static void flush_led_fb(int32_t pin, const uint8_t *map)
{
    uint8_t block[48];

    spi_set_cspin(pin);

    writePinHigh(pin);
    SPI_DELAY();
    writePinLow(pin);
    SPI0_Write1(0x21);
    SPI0_Write1(0);

    int32_t led_idx = 0, i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 16; j++)
        {
            uint8_t r, g, b;
            uint8_t mi = map[led_idx++];

            if (mi >= DRIVER_LED_TOTAL)
                r = g = b = 0;
            else {
                RGB *v = &g_fb[mi];
                r = v->r;
                g = v->g;
                b = v->b;
            }

            block[j] = r;
            block[j + 16] = b;
            block[j + 32] = g;
        }

        SPI0_Write(block, 48);
    }

    writePinHigh(pin);
}

// #pragma GCC pop_options

#else

static void _set_color_direct(int index, uint8_t r, uint8_t g, uint8_t b)
{
    int l = g_led_pos[index];

    if (l >= 100)
    {
        l -= 100;
        spi_set_cspin(B1);
    }
    else
        spi_set_cspin(B2);

    int y = l / 16;
    int a = l % 16;
            
    spi_w3(1, y * 48 + a, r); // r
    spi_w3(1, y * 48 + a + 2 * 8, b); // b
    spi_w3(1, y * 48 + a + 4 * 8, g); // g  
}

#endif

void _set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
#ifdef USE_FRAMEBUFFER
    g_fb[index].r = r;
    g_fb[index].g = g;
    g_fb[index].b = b;
#else
    _set_color_direct(index, r, g, b);
#endif
}

void _read_color(int index, uint8_t *r, uint8_t *g, uint8_t *b)
{
    int l = g_led_pos[index];

    if (l >= 100)
    {
        l -= 100;
        spi_set_cspin(B1);
    }
    else
        spi_set_cspin(B2);

    int y = l / 16;
    int a = l % 16;
            
    spi_r3(1, y * 48 + a, r); // r
    spi_r3(1, y * 48 + a + 2 * 8, b); // b
    spi_r3(1, y * 48 + a + 4 * 8, g); // g  
}

void reset_rgb(int pin)
{
    spi_set_cspin(pin);
    
    spi_w3(3, 0, 0);
    spi_w3(3, 0x13, 0xAA);
    spi_w3(3, 0x14, 0);
    spi_w3(3, 0x15, 4);
    spi_w3(3, 0x16, 0xC0);
    spi_w3(3, 0x1A, 0);
    
    // set curent
    for (int i = 0; i < 12; i++)
    {
        spi_w3(4, i, 0x80);
    }               
    
    // led all on
    for (int i = 0; i < 192/8; i++)
    {
        spi_w3(0, i, 0xFF);
    }

    // turn off pwm
    for (int i = 0; i < 192; i++)
    {
        spi_w3(1, i, 0);
    }
                
    // normal mode
    spi_w3(3, 0, 1);
}

#ifdef VIA_OPENRGB_HYBRID
extern uint8_t is_orgb_mode;
#endif

void process_backlight(uint8_t devid, volatile LED_TYPE* states)
{
    static unsigned char state = 0;       

    switch (state)
    {
        case 0: /* init RGB chips */
            spi_init();

            writePinHigh(SDB);
            setPinOutput(SDB);
            reset_rgb(B1);
            reset_rgb(B2);

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

            flush_led_fb(B1, led_b1_map);
            flush_led_fb(B2, led_b2_map);
        #endif        
            break;

    }
}
