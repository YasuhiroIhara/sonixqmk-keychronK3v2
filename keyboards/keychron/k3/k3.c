/* Copyright 2020 Adam Honse <calcprogrammer1@gmail.com>
 * Copyright 2020 Dimitris Mantzouranis <d3xter93@gmail.com>
 * Copyright 2021 Harrison Chan (Xelus)
 * Copyright 2022 Pablo Ramirez <jp.ramangulo@gmail.com>
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

#include "k3.h"

#include "quantum.h"
#include "rgb_custom_effects.h"

#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_TYPING_HEATMAP)

void matrix_scan_kb(void) {
    // Scan the matrix as usual
    matrix_scan_user();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
        if (record->event.pressed) {
        // Pass key press position to animations that need it
        process_my_typewriter_effect(record->event.key.row, record->event.key.col);
        process_rgb_led_canvas(record->event.key.row, record->event.key.col);
        process_bluewave_effect(record->event.key.row, record->event.key.col);

        // Check for Right Shift to cycle bluewave background
        if (keycode == KC_RSFT) {
            cycle_bluewave_background();
        }
    }
    
    // Don't forget to call the user's process_record function
    return process_record_user(keycode, record);
}

#endif