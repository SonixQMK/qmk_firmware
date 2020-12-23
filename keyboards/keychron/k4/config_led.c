#ifdef RGB_MATRIX_ENABLE

#include "rgb_matrix.h"
#include "config_led.h"

led_config_t g_led_config = { {
  {  0,      1,  2,      3,      4,      5,   6,      7,      8,      9,  10,     11,     12,  13,  14,  15,  16,  17,     18 },
  { 19,     20, 21,     22,     23,     24,  25,     26,     27,     28,  29,     30,     31,  32,  33,  34,  35,  36,     37 },
  { 38,     39, 40,     41,     42,     43,  44,     45,     46,     47,  48,     49,     50,  51,  52,  53,  54,  55,     56 },
  { 57,     58, 59,     60,     61,     62,  63,     64,     65,     66,  67,     68, NO_LED,  70,  71,  72,  73,  74, NO_LED },
  { 76, NO_LED, 78,     79,     80,     81,  82,     83,     84,     85,  86,     87,     88,  89,  90,  91,  92,  93,     94 },
  { 95,     96, 97, NO_LED, NO_LED, NO_LED, 101, NO_LED, NO_LED, NO_LED, 105, NO_LED,    106, 107, 108, 109, 110, 111,    112 }
}, {
    { 1 *12, 0 }, { 2 *12, 0 }, { 3 *12, 0 }, { 4 *12, 0 }, { 5 *12, 0 },  { 6 *12, 0  }, { 7 *12, 0  }, { 8 *12, 0 }, { 9 *12, 0  }, { 10*12, 0 }, { 11*12, 0 }, { 12*12, 0 }, { 13*12, 0 }, { 14*12, 0 }, { 15*12, 0 }, { 16*12, 0 }, { 17*12, 0 }, { 18*12, 0 }, { 19*12, 0 },
    { 1 *12, 1 }, { 2 *12, 1 }, { 3 *12, 1 }, { 4 *12, 1 }, { 5 *12, 1 },  { 6 *12, 1  }, { 7 *12, 1  }, { 8 *12, 1 }, { 9 *12, 1  }, { 10*12, 1 }, { 11*12, 1 }, { 12*12, 1 }, { 13*12, 1 }, { 14*12, 1 }, { 15*12, 1 }, { 16*12, 1 }, { 17*12, 1 }, { 18*12, 1 }, { 19*12, 1 },
    { 1 *12, 2 }, { 2 *12, 2 }, { 3 *12, 2 }, { 4 *12, 2 }, { 5 *12, 2 },  { 6 *12, 2  }, { 7 *12, 2  }, { 8 *12, 2 }, { 9 *12, 2  }, { 10*12, 2 }, { 11*12, 2 }, { 12*12, 2 }, { 13*12, 2 }, { 14*12, 2 }, { 15*12, 2 }, { 16*12, 2 }, { 17*12, 2 }, { 18*12, 2 }, { 19*12, 2 },
    { 1 *12, 3 }, { 2 *12, 3 }, { 3 *12, 3 }, { 4 *12, 3 }, { 5 *12, 3 },  { 6 *12, 3  }, { 7 *12, 3  }, { 8 *12, 3 }, { 9 *12, 3  }, { 10*12, 3 }, { 11*12, 3 }, { 12*12, 3 }, { 13*12, 3 }, { 14*12, 3 }, { 15*12, 3 }, { 16*12, 3 }, { 17*12, 3 }, { 18*12, 3 }, { 19*12, 3 },
    { 1 *12, 4 }, { 2 *12, 4 }, { 3 *12, 4 }, { 4 *12, 4 }, { 5 *12, 4 },  { 6 *12, 4  }, { 7 *12, 4  }, { 8 *12, 4 }, { 9 *12, 4  }, { 10*12, 4 }, { 11*12, 4 }, { 12*12, 4 }, { 13*12, 4 }, { 14*12, 4 }, { 15*12, 4 }, { 16*12, 4 }, { 17*12, 4 }, { 18*12, 4 }, { 19*12, 4 },
    { 1 *12, 5 }, { 2 *12, 5 }, { 3 *12, 5 }, { 7 *12, 5 }, { 11 *12, 5 }, { 12 *12, 5 }, { 13 *12, 5 }, { 8 *12, 5 }, { 14 *12, 5 }, { 10*12, 5 }, { 11*12, 5 }, { 12*12, 5 }, { 13*12, 5 }, { 14*12, 5 }, { 15*12, 5 }, { 16*12, 5 }, { 17*12, 5 }, { 18*12, 5 }, { 19*12, 5 },
}, {
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 0,
   4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
} };

#endif
