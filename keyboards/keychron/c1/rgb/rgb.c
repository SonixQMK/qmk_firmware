/* Copyright 2025 @ Keychron (https://www.keychron.com)
 * Copyright 2025 vjdato21 <vjdato21@outlook.com>
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
#include "config.h"

#ifdef DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0) {
        default_layer_set(1UL << (active ? 2 : 0));
    }
    return true;
}
#endif

void keyboard_post_init_kb(void) {
    gpio_set_pin_output_push_pull(LED_MAC_PIN);
    gpio_set_pin_output_push_pull(LED_WIN_PIN);
    gpio_write_pin(LED_MAC_PIN, !LED_OS_PIN_ON_STATE);
    gpio_write_pin(LED_WIN_PIN, !LED_OS_PIN_ON_STATE);

    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    if (default_layer_state == (1U << _MAC_BASE)) {
        gpio_write_pin(LED_MAC_PIN, LED_OS_PIN_ON_STATE);
        gpio_write_pin(LED_WIN_PIN, !LED_OS_PIN_ON_STATE);
    }
    if (default_layer_state == (1U << _WIN_BASE)) {
        gpio_write_pin(LED_MAC_PIN, !LED_OS_PIN_ON_STATE);
        gpio_write_pin(LED_WIN_PIN, LED_OS_PIN_ON_STATE);
    }
}

void suspend_power_down_kb(void) {
    gpio_write_pin(LED_WIN_PIN, !LED_OS_PIN_ON_STATE);
    gpio_write_pin(LED_MAC_PIN, !LED_OS_PIN_ON_STATE);

#ifdef RGB_MATRIX_SLEEP
    rgb_matrix_set_suspend_state(true);
#endif

    suspend_power_down_user();
}
