#include "rgb_matrix.h"

extern void _set_color(int index, uint8_t r, uint8_t g, uint8_t b);

extern void process_backlight(uint8_t devid, volatile LED_TYPE* states);

static void flush(void)
{
    process_backlight(0, NULL);
}

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= 0 && index < DRIVER_LED_TOTAL)
    {
        _set_color(index, r, g, b);
    }   
}

static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++)
        set_color(i, r, g, b);
}

void init(void) {
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = init,
    .flush         = flush,
    .set_color     = set_color,
    .set_color_all = set_color_all,
};
