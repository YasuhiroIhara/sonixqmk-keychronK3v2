// !!! DO NOT ADD #pragma once !!! //

// Step 1.
// Declare custom effects using the RGB_MATRIX_EFFECT macro
// (note the lack of semicolon after the macro!)
RGB_MATRIX_EFFECT(my_raindrop)
RGB_MATRIX_EFFECT(led_canvas)
RGB_MATRIX_EFFECT(my_typewriter)


// Step 2.
// Define effects inside the `RGB_MATRIX_CUSTOM_EFFECT_IMPLS` ifdef block
#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "rgb_custom_effects.h"

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS