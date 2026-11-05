#pragma once

#include "quantum.h"

/*
 * QK_KB_0..2 are taken by the VIA keymap's USER00 range.
 */
enum k530_keycodes {
    K530_BT_PAIR = QK_KB_8, /* tap: reconnect, hold 3 s: pair a new host */
};
