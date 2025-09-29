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

#if defined(RGB_MATRIX_CUSTOM_USER)

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // Check current animation mode and call the appropriate handler
        switch (rgb_matrix_get_mode()) {
            case RGB_MATRIX_CUSTOM_my_typewriter:
                process_my_typewriter_effect(record->event.key.row, record->event.key.col);
                break;
            case RGB_MATRIX_CUSTOM_bluewave:
                if (keycode == KC_RSFT) {
                    cycle_bluewave_background();
                }
            case RGB_MATRIX_CUSTOM_splash_wave:
                if (keycode == KC_RSFT) {
                    splash_wave_increase_bg();
                }
                break;
            default:
                break;
        }
    }
    return process_record_user(keycode, record);
}

#endif