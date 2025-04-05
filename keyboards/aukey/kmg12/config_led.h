/*
Copyright 2022 Luca Mazzilli <setmaat@gmail.com>

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

#pragma once


// Backlight configuration
#define BACKLIGHT_LEVELS 8


//C0, C1, C3,    RBG row 1 (keys)
//C4, C5, C6,    RBG row 2 (keys)
//C7, C8, C9     RBG row 3 (keys)
//C10, C11, C12, RBG row 4 (keys and 1 underglow - led DL23 on PCB)
//B6, B7, B8,    RBG row 5 (keys)
//B9, B10, B11,  RBG row 6 (keys and 7 underglow - led DL37 to DL42, and DL24 on PCB)
//D4, B15, D3,   RBG row 7 (21 underglow - led DL1, DL25 to DL36, DL43 to DL50 on PCB)
//D5, A15, D6    RBG row 8 (21 underglow - led DL2 to DL24 on PCB)

#define LED_MATRIX_ROWS 8
#define LED_MATRIX_ROW_CHANNELS 3
#define LED_MATRIX_ROWS_HW (LED_MATRIX_ROWS * LED_MATRIX_ROW_CHANNELS)

#define LED_MATRIX_ROW_PINS { C0, C1, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, B6, B7, B8, B9, B10, B11, D4, B15, D3 , D5, A15, D6  }

#define LED_MATRIX_COLS 21
#define LED_MATRIX_COL_PINS MATRIX_COL_PINS

#define DRIVER_LED_TOTAL (155) //105 keys 1 underglow in row 4 + 7 underglow in row 6 + 21 underglow in row 7 and 21 underglow in row 8
