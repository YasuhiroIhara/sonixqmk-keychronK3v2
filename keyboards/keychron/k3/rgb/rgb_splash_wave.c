#include "rgb_custom_effects.h"
#include "lib/lib8tion/lib8tion.h" // For qadd8, scale16by8
#include <math.h>

// This file implements a custom RGB Matrix effect.
// It is not a standard reactive effect runner to allow for a custom background.

#define WAVE_WIDTH 25 // Width of the wave ripple
#define BG_STEPS 20   // Number of background brightness steps

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static uint8_t bg_brightness_step = 0; // 0-20

// Function to be called by a keymap to cycle background brightness
void splash_wave_increase_bg(void) {
    bg_brightness_step = (bg_brightness_step + 1) % (BG_STEPS + 1);
}

// The main animation function
bool splash_wave(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // 1. Set background color
    uint8_t bg_val = (bg_brightness_step * 255) / BG_STEPS;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, bg_val, bg_val, bg_val);
    }

    // 2. Draw wave from the last key press
    // We use g_last_hit_tracker like the reference SPLASH effect, but manually
    // render to support a background. We only use the most recent key press.
    if (g_last_hit_tracker.count == 0) {
        return rgb_matrix_check_finished_leds(led_max);
    }

    // Get data for the last key press
    uint8_t last_hit_index = g_last_hit_tracker.count - 1;
    int16_t  center_x   = g_last_hit_tracker.x[last_hit_index];
    int16_t  center_y   = g_last_hit_tracker.y[last_hit_index];
    
    // Scale tick by animation speed setting. The faster the setting, the faster the wave.
    // qadd8(rgb_matrix_config.speed, 1) ensures speed is not zero.
    // The division by 8 is a scaling factor to get a pleasant speed.
    uint16_t tick = scale16by8(g_last_hit_tracker.tick[last_hit_index], qadd8(rgb_matrix_config.speed, 1)) / 8 + 10;

    float wave_radius = tick;

    // If wave is huge, it has dissipated.
    // The tick will eventually wrap around, so this check is important.
    if (wave_radius > 255) {
        return rgb_matrix_check_finished_leds(led_max);
    }

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();

        float dx = g_led_config.point[i].x - center_x;
        float dy = g_led_config.point[i].y - center_y;
        float dist = hypotf(dx, dy);

        float dist_from_wave = fabsf(dist - wave_radius);

        if (dist_from_wave < WAVE_WIDTH) {
            // LED is part of the wave
            float intensity = 1.0f - (dist_from_wave / WAVE_WIDTH);

            // Randomize color for splash effect (blue/white)
            uint8_t hue = 170 + (rand() % 20 - 10); // +/- 10 from blue
            uint8_t sat = 255 - (rand() % 80);          // 175-255, tending towards white
            uint8_t val = intensity * 255;

            HSV hsv = {hue, sat, val};
            RGB rgb = hsv_to_rgb(hsv);

            // Blend with background using MAX to ensure wave is always brighter
            rgb.r = MAX(rgb.r, bg_val);
            rgb.g = MAX(rgb.g, bg_val);
            rgb.b = MAX(rgb.b, bg_val);

            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}