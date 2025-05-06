#pragma once

#include "quantum.h"
#include "rgb_matrix.h"
#include "rgb_matrix_types.h"
#include "color.h"       // For RGB/HSV structures and functions
#include "led_tables.h"

#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS)

void process_my_typewriter_effect(uint8_t row, uint8_t col);
bool my_typewriter(effect_params_t* params);
bool my_raindrop(effect_params_t* params);
bool led_canvas(effect_params_t* params);

#endif