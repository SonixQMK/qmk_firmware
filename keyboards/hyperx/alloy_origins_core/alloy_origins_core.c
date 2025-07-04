/*

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quantum.h"

#ifdef RGB_MATRIX_ENABLE
const is31fl3737_led_t PROGMEM g_is31fl3737_leds[IS31FL3737_LED_COUNT] = {
/* Refer to IS31 manual for these locations
 *   driver
 *   |  R location
 *   |  |         G location
 *   |  |         |         B location
 *   |  |         |         | */
    {0, SW2_CS12, SW1_CS12, SW3_CS12}, //3     0
    {0, SW2_CS11, SW1_CS11, SW3_CS11}, //2     1
    {0, SW2_CS10, SW1_CS10, SW3_CS10}, //1     2
    {0, SW2_CS9, SW1_CS9, SW3_CS9},    //~     3
    {0, SW2_CS8, SW1_CS8, SW3_CS8},    //f7    4
    {0, SW2_CS7, SW1_CS7, SW3_CS7},    //f6    5
    {0, SW2_CS6, SW1_CS6, SW3_CS6},    //f5    6
    {0, SW2_CS5, SW1_CS5, SW3_CS5},    //f4    7
    {0, SW2_CS4, SW1_CS4, SW3_CS4},    //f3    8
    {0, SW2_CS3, SW1_CS3, SW3_CS3},    //f2    9
    {0, SW2_CS2, SW1_CS2, SW3_CS2},    //f1    10
    {0, SW2_CS1, SW1_CS1, SW3_CS1},    //esc   11
    {0, SW5_CS12, SW4_CS12, SW6_CS12}, //y     12
    {0, SW5_CS11, SW4_CS11, SW6_CS11}, //t     13
    {0, SW5_CS10, SW4_CS10, SW6_CS10}, //r     14
    {0, SW5_CS9, SW4_CS9, SW6_CS9},    //e     15
    {0, SW5_CS8, SW4_CS8, SW6_CS8},    //w     16
    {0, SW5_CS7, SW4_CS7, SW6_CS7},    //q     17
    {0, SW5_CS6, SW4_CS6, SW6_CS6},    //tab   18
    {0, SW5_CS5, SW4_CS5, SW6_CS5},    //8     19
    {0, SW5_CS4, SW4_CS4, SW6_CS4},    //7     20
    {0, SW5_CS3, SW4_CS3, SW6_CS3},    //6     21
    {0, SW5_CS2, SW4_CS2, SW6_CS2},    //5     22
    {0, SW5_CS1, SW4_CS1, SW6_CS1},    //4     23
    {0, SW8_CS12, SW7_CS12, SW9_CS12}, //z     24
    {0, SW8_CS11, SW7_CS11, SW9_CS11}, //u     25
    {0, SW8_CS10, SW7_CS10, SW9_CS10}, //lshift 26
    {0, SW8_CS9, SW7_CS9, SW9_CS9},    //j     27
    {0, SW8_CS8, SW7_CS8, SW9_CS8},    //h     28
    {0, SW8_CS7, SW7_CS7, SW9_CS7},    //g     29
    {0, SW8_CS6, SW7_CS6, SW9_CS6},    //f     30
    {0, SW8_CS5, SW7_CS5, SW9_CS5},    //d     31
    {0, SW8_CS4, SW7_CS4, SW9_CS4},    //s     32
    {0, SW8_CS3, SW7_CS3, SW9_CS3},    //a     33
    {0, SW8_CS2, SW7_CS2, SW9_CS2},    //caps  34
    {0, SW11_CS10, SW10_CS10, SW12_CS10}, //spc   35
    {0, SW11_CS8, SW10_CS8, SW12_CS8},    //lalt  36
    {0, SW11_CS7, SW10_CS7, SW12_CS7},    //lwin  37
    {0, SW11_CS6, SW10_CS6, SW12_CS6},    //lctrl 38
    {0, SW11_CS5, SW10_CS5, SW12_CS5},    //n     39
    {0, SW11_CS4, SW10_CS4, SW12_CS4},    //b     40
    {0, SW11_CS3, SW10_CS3, SW12_CS3},    //v     41
    {0, SW11_CS2, SW10_CS2, SW12_CS2},    //c     42
    {0, SW11_CS1, SW10_CS1, SW12_CS1},    //x     43
    {1, SW2_CS12, SW1_CS12, SW3_CS12}, //=        44
    {1, SW2_CS11, SW1_CS11, SW3_CS11}, //-        45
    {1, SW2_CS10, SW1_CS10, SW3_CS10}, //0        46
    {1, SW2_CS9, SW1_CS9, SW3_CS9},    //9        47
    {1, SW2_CS8, SW1_CS8, SW3_CS8},    //pause    48
    {1, SW2_CS7, SW1_CS7, SW3_CS7},    //scroll   49
    {1, SW2_CS6, SW1_CS6, SW3_CS6},    //print    50
    {1, SW2_CS5, SW1_CS5, SW3_CS5},    //f12      51
    {1, SW2_CS4, SW1_CS4, SW3_CS4},    //f11      52
    {1, SW2_CS3, SW1_CS3, SW3_CS3},    //f10      53
    {1, SW2_CS2, SW1_CS2, SW3_CS2},    //f9       54
    {1, SW2_CS1, SW1_CS1, SW3_CS1},    //f8       55
    {1, SW5_CS12, SW4_CS12, SW6_CS12}, //end      56
    {1, SW5_CS11, SW4_CS11, SW6_CS11}, //del      57
    {1, SW5_CS10, SW4_CS10, SW6_CS10}, //"\"      58
    {1, SW5_CS9, SW4_CS9, SW6_CS9},    //]        59
    {1, SW5_CS8, SW4_CS8, SW6_CS8},    //[        60
    {1, SW5_CS7, SW4_CS7, SW6_CS7},    //p        61
    {1, SW5_CS6, SW4_CS6, SW6_CS6},    //o        62
    {1, SW5_CS5, SW4_CS5, SW6_CS5},    //i        63
    {1, SW5_CS4, SW4_CS4, SW6_CS4},    //pgup     64 
    {1, SW5_CS3, SW4_CS3, SW6_CS3},    //home     65
    {1, SW5_CS2, SW4_CS2, SW6_CS2},    //ins      66
    {1, SW5_CS1, SW4_CS1, SW6_CS1},    //bkspc    67
    {1, SW8_CS11, SW7_CS11, SW9_CS11}, //"/"      68
    {1, SW8_CS10, SW7_CS10, SW9_CS10}, //.        69
    {1, SW8_CS9, SW7_CS9, SW9_CS9},    //,        70
    {1, SW8_CS8, SW7_CS8, SW9_CS8},    //m        71
    {1, SW8_CS7, SW7_CS7, SW9_CS7},    //enter    72  
    {1, SW8_CS5, SW7_CS5, SW9_CS5},    //'        73
    {1, SW8_CS4, SW7_CS4, SW9_CS4},    //;        74
    {1, SW8_CS3, SW7_CS3, SW9_CS3},    //l        75
    {1, SW8_CS2, SW7_CS2, SW9_CS2},    //k        76
    {1, SW8_CS1, SW7_CS1, SW9_CS1},    //pgdn     77 
    {1, SW11_CS9, SW10_CS9, SW12_CS9},  //rt      78
    {1, SW11_CS8, SW10_CS8, SW12_CS8},  //dn      79
    {1, SW11_CS7, SW10_CS7, SW12_CS7},  //lt      80
    {1, SW11_CS6, SW10_CS6, SW12_CS6},  //rctrl   81 
    {1, SW11_CS5, SW10_CS5, SW12_CS5},  //menu    82
    {1, SW11_CS4, SW10_CS4, SW12_CS4},  //rwin    83
    {1, SW11_CS3, SW10_CS3, SW12_CS3},  //ralt    84
    {1, SW11_CS2, SW10_CS2, SW12_CS2},  //up      85
    {1, SW11_CS1, SW10_CS1, SW12_CS1},  //rshift  86  
};

led_config_t g_led_config = {
    /*
    key matrix to led position
    ┌──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┰──────────┐
    │   ESC    │    F1    │    F2    │    F3    │    F4    │    F5    │    F6    │    F7    │    F8    │    F9    │   F10    │   F11    │
    │    11    │    10    │    09    │    08    │    07    │    06    │    05    │    04    │    55    │    54    │    53    │    52    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │    ~     │    1     │    2     │    3     │    4     │    5     │    6     │    7     │    8     │    9     │    0     │    -     │
    │    03    │    02    │    01    │    00    │    23    │    22    │    21    │    20    │    19    │    47    │    46    │    45    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │   TAB    │    Q     │    W     │    E     │    R     │    T     │    Y     │    U     │    I     │    O     │    P     │    [     │
    │    18    │    17    │    16    │    15    │    14    │    13    │    12    │    25    │    63    │    62    │    61    │    60    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │   CAPS   │    A     │    S     │    D     │    F     │    G     │    H     │    J     │    K     │    L     │    ;     │    "     │
    │    34    │    33    │    32    │    31    │    30    │    29    │    28    │    27    │    76    │    75    │    74    │    73    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │  LSHIFT  │    na    │    Z     │    X     │    C     │    V     │    B     │    N     │    M     │   ","    │   "."    │    /     │
    │    26    │  NO_LED  │    24    │    43    │    42    │    41    │    40    │    39    │    71    │    70    │    69    │    68    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │   LCTRL  │   LWIN   │   LALT   │    na    │   SPC    │    na    │    na    │   RALT   │   RWIN   │    FN    │    na    │    na    │
    │    38    │    37    │    36    │  NO_LED  │    35    │  NO_LED  │  NO_LED  │    84    │    83    │    82    │  NO_LED  │  NO_LED  │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │    =     │    na    │  BKSPC   │   DEL    │   END    │   PGDN   │  RSHIFT  │    UP    │  RCTRL   │   LEFT   │   DOWN   │   RIGHT  │
    │    44    │  NO_LED  │    67    │    57    │    56    │    77    │    86    │    81    │    85    │    80    │    79    │    78    │
    ├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
    │   F12    │ PRNTSCRN │  SCROLL  │  PAUSE   │   INS    │   HOME   │   PGUP   │    ]     │    \     │    na    │   ENTER  │    na    │
    │    51    │    50    │    49    │    48    │    66    │    65    │    64    │    59    │    58    │  NO_LED  │    72    │  NO_LED  │
    └──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┘
    */
    {
        {11,     10,      9,      8,      7,      6,      5,      4,     55,     54,     53,     52}, //0
        { 3,      2,      1,      0,     23,     22,     21,     20,     19,     47,     46,     45}, //1
        {18,     17,     16,     15,     14,     13,     12,     25,     63,     62,     61,     60}, //2
        {34,     33,     32,     31,     30,     29,     28,     27,     76,     75,     74,     73}, //3
        {26, NO_LED,     24,     43,     42,     41,     40,     39,     71,     70,     69,     68}, //4
        {38,     37,     36, NO_LED,     35, NO_LED, NO_LED,     84,     83,     82, NO_LED, NO_LED}, //5
        {44, NO_LED,     67,     57,     56,     77,     86,     81,     85,     80,     79,     78}, //6
        {51,     50,     49,     48,     66,     65,     64,     59,     58, NO_LED,     72, NO_LED}  //7
    }, 
    {
        {0,0},{15,0},{30,0},{45,0},{60,0},{75,0},{90,0},{105,0},{120,0},{135,0},{150,0},{165,0},{180,0},{195,0},{210,0},{224,0},
        {0,13},{14,13},{28,13},{42,13},{56,13},{70,13},{84,13},{98,13},{112,13},{126,13},{140,13},{154,13},{168,13},{182,13},{192,13},{210,13},{224,13},
        {0,26},{14,26},{28,26},{42,26},{56,26},{70,26},{84,26},{98,26},{112,26},{126,26},{140,26},{154,26},{168,26},{182,26},{192,26},{210,26},{224,26},
        {0,39},{19,39},{38,39},{56,39},{75,39},{94,39},{112,39},{131,39},{150,39},{168,39},{187,39},{206,39},{224,39},
        {0,52},{19,52},{38,52},{56,52},{75,52},{94,52},{112,52},{131,52},{150,52},{168,52},{187,52},{206,52},{224,52},
        {0,64},{23,64},{45,64},{68,64},{90,64},{112,64},{135,64},{157,64},{180,64},{202,64},{224,64}
    }, 
    {
        1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        1,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        1, 1, 1,    4,       1, 1, 1,      
        4,    1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 
        4, 1, 1, 1, 1, 1, 1, 4, 4,    1,   
    }
};

#endif