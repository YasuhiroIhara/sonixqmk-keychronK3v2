/* Copyright 2020 Adam Honse <calcprogrammer1@gmail.com>
 * Copyright 2020 Dimitris Mantzouranis <d3xter93@gmail.com>
 * Copyright 2022 Harrison Chan (Xelus)
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

#include "jis.h"

#define __ NO_LED

/* JIS keymap LEDs */

led_config_t g_led_config = {
  {
      // Key Matrix to LED Index
      { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15 },
      { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, __, 31 },
      { 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, __, __, 45 },
      { 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, __, 60 },
      { 61, __, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 84, 74 },
      { 75, 76, 77, 78, __, __, 79, 29, __, 80, 81, 82, __, 83, 85, 86 }
  },
  {
      // LED Index to Physical Position
      {0,0},  {15,0},  {30,0},  {45,0},  {60,0},  {75,0},  {90,0},   {105,0},  {119,0},  {134,0},  {149,0},  {164,0},  {179,0},  {194,0},  {209,0},  {224,0},
      {0,13}, {15,13}, {30,13}, {45,13}, {60,13}, {75,13}, {90,13},  {105,13}, {119,13}, {134,13}, {149,13}, {164,13}, {179,13}, {194,13}, {209,13}, {224,13},
      {4,26}, {22,26}, {37,26}, {52,26}, {67,26}, {82,26}, {97,26},  {112,26}, {127,26}, {142,26}, {157,26}, {172,26}, {187,26},                     {224,26},
      {6,38}, {26,38}, {41,38}, {56,38}, {71,38}, {86,38}, {101,38}, {116,38}, {131,38}, {146,38}, {161,38}, {175,38}, {190,38}, {207,32},           {224,38},
      {9,51},          {34,51}, {49,51}, {64,51}, {78,51}, {93,51},  {108,51}, {123,51}, {138,51}, {153,51}, {168,51}, {183,51}, {203,51},           {224,51},
      {0,64}, {15,64}, {30,64}, {45,64},                   {90,64},                      {134,64}, {149,64}, {164,64}, {179,64}, {194,64}, {209,64}, {224,64},
  },
  {
      // RGB LED Index to Flag
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      4, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1,
      1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,       1,
      8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,    1,
      4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,    1,
      1, 1, 1, 1,       4,       1, 1, 1, 1, 1, 1, 1
  }
};