#include "rgb_custom_effects.h"
#include <math.h>   // For sqrtf and powf
#include <stdlib.h> // For rand()

// --- Configuration ---
#define MAX_WAVES 6         // Maximum number of concurrent waves
#define WAVE_WIDTH 5.5f      // How wide the ripple is (increased for smoother wave)
#define WAVE_DURATION 5000   // How long a wave lasts in ms (slightly reduced)

// --- Data Structures ---
typedef struct {
    uint8_t row;         // Center row of the wave
    uint8_t col;         // Center col of the wave
    uint16_t timestamp;  // Time the wave was created
    bool active;         // Whether this wave slot is in use
    uint8_t wave_id;     // Unique ID for consistent coloring
} wave_t;

// --- Global State ---
static wave_t waves[MAX_WAVES];
static uint8_t background_level = 0; // 0-20, where 0 is black, 20 is white
static uint8_t last_wave_index = 0;
static uint8_t next_wave_id = 0;

// --- Helper Functions ---

// Map a value from one range to another
static float map_value(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Get consistent color for a wave based on its ID and distance from center
static RGB get_wave_color(uint8_t wave_id, float distance_ratio, float intensity) {
    HSV hsv;
    
    // Use wave_id to determine if this wave is blue or white-ish
    if ((wave_id % 3) == 0) {
        // White-ish wave (33% chance)
        hsv.h = 190 + ((wave_id * 7) % 20 - 10);  // Slight cyan tint
        hsv.s = 80 + ((wave_id * 13) % 40);       // Some saturation variation
        hsv.v = (uint8_t)(intensity * 255);
    } else {
        // Blue wave (67% chance)
        hsv.h = 150 + ((wave_id * 11) % 30 - 15); // Blue with variation
        hsv.s = 255;
        hsv.v = (uint8_t)(intensity * 255);
    }
    
    return hsv_to_rgb(hsv);
}

// Calculate wave intensity based on distance from wave edge
/*static float calculate_wave_intensity(float distance, float current_radius) {
    float wave_edge_distance = fabsf(distance - current_radius);
    float half_width = WAVE_WIDTH / 2.0f;
    
    if (wave_edge_distance > half_width) {
        return 0.0f; // Outside wave
    }
    
    // Smooth falloff from center of wave to edge
    float intensity = 1.0f - (wave_edge_distance / half_width);
    
    // Apply smoothing function for more natural look
    intensity = intensity * intensity * (3.0f - 2.0f * intensity); // smoothstep
    
    return intensity;
}*/
static float calculate_wave_intensity(float distance, float current_radius) {
    // 波の最前線付近のみを光らせる
    if (distance > current_radius + WAVE_WIDTH/2.0f || distance < current_radius - WAVE_WIDTH/2.0f) {
        return 0.0f;
    }
    
    float wave_edge_distance = fabsf(distance - current_radius);
    float intensity = 1.0f - (wave_edge_distance / (WAVE_WIDTH / 2.0f));
    return intensity * intensity * (3.0f - 2.0f * intensity);
}

// --- Core Logic ---

// Called from k3.c on every key press
void process_bluewave_effect(uint8_t row, uint8_t col) {
    last_wave_index = (last_wave_index + 1) % MAX_WAVES;
    waves[last_wave_index] = (wave_t){
        .row = row,
        .col = col,
        .timestamp = timer_read(),
        .active = true,
        .wave_id = next_wave_id++
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
        next_wave_id = 0;
    }

    // Map QMK animation speed (0-255) to our desired keys/sec range
    float speed = map_value(rgb_matrix_config.speed, 0, 255, 3, 15); // Adjusted range

    HSV background_hsv = {0, 0, map_value(background_level, 0, 20, 0, 255)};
    RGB background_rgb = hsv_to_rgb(background_hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t led_row = g_led_config.matrix_co[i][0];
        uint8_t led_col = g_led_config.matrix_co[i][1];

        // Start with background color
        RGB final_color = background_rgb;
        float total_intensity = 0.0f;
        RGB accumulated_color = {0, 0, 0};

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

            // Calculate wave intensity
            float intensity = calculate_wave_intensity(distance, current_radius);
            
            if (intensity > 0.0f) {
                // Add time-based fade-out
                float age_factor = 1.0f - ((float)elapsed / WAVE_DURATION);
                intensity *= age_factor;
                
                RGB wave_color = get_wave_color(waves[w].wave_id, distance / current_radius, intensity);
                
                // Accumulate colors with proper blending
                accumulated_color.r += (uint8_t)(wave_color.r * intensity);
                accumulated_color.g += (uint8_t)(wave_color.g * intensity);
                accumulated_color.b += (uint8_t)(wave_color.b * intensity);
                total_intensity += intensity;
            }
        }

        // Blend accumulated wave colors with background
        if (total_intensity > 0.0f) {
            // Normalize accumulated color
            if (total_intensity > 1.0f) {
                accumulated_color.r = (uint8_t)(accumulated_color.r / total_intensity);
                accumulated_color.g = (uint8_t)(accumulated_color.g / total_intensity);
                accumulated_color.b = (uint8_t)(accumulated_color.b / total_intensity);
                total_intensity = 1.0f;
            }
            
            // Blend with background
            float bg_factor = 1.0f - total_intensity;
            final_color.r = (uint8_t)(accumulated_color.r + background_rgb.r * bg_factor);
            final_color.g = (uint8_t)(accumulated_color.g + background_rgb.g * bg_factor);
            final_color.b = (uint8_t)(accumulated_color.b + background_rgb.b * bg_factor);
        }

        rgb_matrix_set_color(i, final_color.r, final_color.g, final_color.b);
    }

    return rgb_matrix_check_finished_leds(led_max);
}