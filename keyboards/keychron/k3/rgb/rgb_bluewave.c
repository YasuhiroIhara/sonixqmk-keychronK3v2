#include "rgb_custom_effects.h"
#include <math.h>   // For sqrtf and powf
#include <stdlib.h> // For rand()

// --- Configuration ---
#define MAX_WAVES 10         // Maximum number of concurrent waves
#define WAVE_WIDTH 2.0f      // How wide the ripple is
#define WAVE_DURATION 4000   // How long a wave lasts in ms

// --- Data Structures ---
typedef struct {
    uint8_t row;         // Center row of the wave
    uint8_t col;         // Center col of the wave
    uint16_t timestamp;  // Time the wave was created
    bool active;         // Whether this wave slot is in use
} wave_t;

// --- Global State ---
static wave_t waves[MAX_WAVES];
static uint8_t background_level = 0; // 0-20, where 0 is black, 20 is white
static uint8_t last_wave_index = 0;

// --- Helper Functions ---

// Find the physical LED index for a matrix position
static int find_led_index(uint8_t row, uint8_t col) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (g_led_config.matrix_co[i][0] == row && g_led_config.matrix_co[i][1] == col) {
            return i;
        }
    }
    return -1;
}

// Map a value from one range to another
static float map_value(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- Core Logic ---

// Called from k3.c on every key press
void process_bluewave_effect(uint8_t row, uint8_t col) {
    last_wave_index = (last_wave_index + 1) % MAX_WAVES;
    waves[last_wave_index] = (wave_t){
        .row = row,
        .col = col,
        .timestamp = timer_read(),
        .active = true
    };
}

// Called from k3.c when Right Shift is pressed
void cycle_bluewave_background(void) {
    // This function will only be called when bluewave is active, checked in the main function
    background_level = (background_level + 1) % 21;
}

// The main animation function
bool bluewave(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        // Initialize state when effect starts
        memset(waves, 0, sizeof(waves));
        background_level = 0;
    }

    // Map QMK animation speed (0-255) to our desired keys/sec range (e.g., 2-20)
    // Default QMK speed is 127. We want this to be ~8 keys/sec.
    float speed = map_value(rgb_matrix_config.speed, 0, 255, 2, 20); // keys per second

    HSV background_hsv = {0, 0, map_value(background_level, 0, 20, 0, 255)};
    RGB background_rgb = hsv_to_rgb(background_hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        // Set background color first
        rgb_matrix_set_color(i, background_rgb.r, background_rgb.g, background_rgb.b);

        uint8_t led_row = g_led_config.matrix_co[i][0];
        uint8_t led_col = g_led_config.matrix_co[i][1];

        // Check against all active waves
        for (int w = 0; w < MAX_WAVES; w++) {
            if (!waves[w].active) continue;

            uint16_t elapsed = timer_elapsed(waves[w].timestamp);
            if (elapsed > WAVE_DURATION) {
                waves[w].active = false;
                continue;
            }

            // Calculate current wave radius
            float current_radius = (elapsed / 1000.0f) * speed;

            // Calculate distance from LED to wave center
            float distance = sqrtf(powf(led_row - waves[w].row, 2) + powf(led_col - waves[w].col, 2));

            // If the LED is part of the wave, color it
            if (distance >= current_radius - (WAVE_WIDTH / 2.0f) && distance <= current_radius + (WAVE_WIDTH / 2.0f)) {
                // Randomly choose between blue and white with variations
                HSV hsv;
                if (rand() % 10 > 3) { // 70% chance of blue
                    hsv = (HSV){.h = 150 + (rand() % 20 - 10), .s = 255, .v = 255};
                } else { // 30% chance of white-ish blue
                    hsv = (HSV){.h = 160 + (rand() % 20 - 10), .s = 100 + (rand() % 50), .v = 255};
                }

                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}
