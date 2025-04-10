
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

#pragma once

#include "quantum.h"

#define XXX KC_NO

#define LAYOUT_86_jis( \
  k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e, k0f, \
  k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d,      k1f, \
  k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c,           k2f, \
  k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b, k3c, k3d,      k3f, \
  k40,      k42, k43, k44, k45, k46, k47, k48, k49, k4a, k4b, k4c, k4d, k4e, k4f, \
  k50, k51, k52, k53,           k56, k57,      k59, k5a, k5b,      k5d, k5e, k5f  \
) \
{ \
  {k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e, k0f}, \
  {k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d, XXX, k1f}, \
  {k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c, XXX, XXX, k2f}, \
  {k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b, k3c, k3d, XXX, k3f}, \
  {k40, XXX, k42, k43, k44, k45, k46, k47, k48, k49, k4a, k4b, k4c, k4d, k4e, k4f}, \
  {k50, k51, k52, k53, XXX, XXX, k56, k57, XXX, k59, k5a, k5b, XXX, k5d, k5e, k5f}  \
}

/*#define LAYOUT_75_ansi( \
  k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e, k0f, \
  k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d,      k1f, \
  k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c, k2d,      k2f, \
  k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b,      k3d,      k3f, \
  k40,      k42, k43, k44, k45, k46, k47, k48, k49, k4a, k4b,      k4d, k4e, k4f, \
  k50, k51, k52,                k56,                k5a, k5b, k5c, k5d, k5e, k5f  \
) \
{ \
  {k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e, k0f}, \
  {k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d, XXX, k1f}, \
  {k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c, k2d, XXX, k2f}, \
  {k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b, XXX, k3d, XXX, k3f}, \
  {k40, XXX, k42, k43, k44, k45, k46, k47, k48, k49, k4a, k4b, XXX, k4d, k4e, k4f}, \
  {k50, k51, k52, XXX, XXX, XXX, k56, XXX, XXX, XXX, k5a, k5b, k5c, k5d, k5e, k5f}  \
} */
