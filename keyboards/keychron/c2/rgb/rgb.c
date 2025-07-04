/* Copyright 2024 NetUserGet, SiphonedAnomaly
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

#include "quantum.h"

bool mode_leds_show = true;

#ifdef DIP_SWITCH_ENABLE
    static void mode_leds_update(void){
        if (!mode_leds_show) {
            gpio_write_pin_low(LED_MAC_PIN);
            gpio_write_pin_low(LED_WIN_PIN);
        }
        else {
            if (layer_state_is(WIN_BASE)) {
                gpio_write_pin_high(LED_WIN_PIN) && gpio_write_pin_low(LED_MAC_PIN);
            } 
            if (layer_state_is(MAC_BASE)) {
                gpio_write_pin_high(LED_MAC_PIN) && gpio_write_pin_low(LED_WIN_PIN);
            }
        }
    }
    bool dip_switch_update_kb(uint8_t index, bool active) {
        if (!dip_switch_update_user(index, active)) {
            return false;
        }
        if (index == 0) {
            if (active) {
                default_layer_state_set_kb(1 << MAC_BASE); /* set layer 2 to be on */
            }
        }
        mode_leds_update();
        return true;
    }
#endif // DIP_SWITCH_ENABLE


void keyboard_pre_init_kb(void) {
    // Setup Win & Mac LED Pins as output
    gpio_set_pin_output(LED_WIN_PIN);
    gpio_set_pin_output(LED_MAC_PIN);
}

void keyboard_post_init_kb(void) {
    // Setup Default Keymap.
    // If you chose to not have the dipswich enabled change this WIN_BASE to be your default keymap.
    // Eg: MAC_BASE
    default_layer_state_set_kb(1 << WIN_BASE); /* set layer 0 to be on */
}

#ifdef RGB_MATRIX_SLEEP
    void suspend_power_down_kb(void) {
        // Turn leds off
        mode_leds_show = false;
        mode_leds_update();

        rgb_matrix_set_suspend_state(true);

    }

    void suspend_wakeup_init_kb(void) {
        mode_leds_show = true;
        mode_leds_update();
        
        rgb_matrix_set_suspend_state(false);

    }
#endif // RGB_MATRIX_SLEEP