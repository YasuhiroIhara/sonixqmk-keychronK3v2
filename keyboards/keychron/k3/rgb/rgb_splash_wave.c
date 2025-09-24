#include "quantum.h"
#include "rgb_matrix.h"
#include <math.h>

#define MAX_SPLASHES 8
#define WAVE_WIDTH 30

// 状態管理用の構造体
typedef struct {
    bool     enabled;
    uint8_t  key_index;
    uint16_t start_tick;
    uint8_t  center_x;
    uint8_t  center_y;
} splash_wave_t;

static splash_wave_t splashes[MAX_SPLASHES];
static uint8_t bg_brightness_step = 0; // 0-20

// Find the key index for a given row and col
static uint8_t get_key_index(uint8_t row, uint8_t col) {
    // This mapping depends on the specific keyboard wiring.
    // The default implementation of `g_led_config.matrix_coors` might not be populated.
    // A direct mapping or a more robust search might be needed if this fails.
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (g_led_config.matrix_coors[i][0] == col && g_led_config.matrix_coors[i][1] == row) {
            return i;
        }
    }
    // Fallback for keyboards without matrix_coors mapping
    if (row < MATRIX_ROWS && col < MATRIX_COLS) {
        return g_led_config.matrix_map[row][col];
    }
    return 0xFF; // Not found
}

// Key press handler
void process_splash_wave(uint8_t row, uint8_t col) {
    uint8_t key_index = get_key_index(row, col);
    if (key_index >= DRIVER_LED_TOTAL) return;

    for (int i = 0; i < MAX_SPLASHES; i++) {
        if (!splashes[i].enabled) {
            splashes[i].enabled    = true;
            splashes[i].key_index  = key_index;
            splashes[i].start_tick = timer_read();
            splashes[i].center_x   = g_led_config.point[key_index].x;
            splashes[i].center_y   = g_led_config.point[key_index].y;
            return;
        }
    }
}

// Background brightness handler
void splash_wave_increase_bg(void) {
    bg_brightness_step = (bg_brightness_step + 1) % 21;
}

bool SPLASH_WAVE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // 1. Set background
    uint8_t bg_val = (bg_brightness_step * 255) / 20;
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, bg_val, bg_val, bg_val);
    }

    // 2. Draw waves
    uint32_t time_now = timer_read();
    // Speed: 1(slow) to 8(fast). Default speed is 3 keys/sec.
    // QMK speed is 0-255. Let's map it. Base speed + scaled value.
    float speed = 1.5f + (rgb_matrix_config.speed / 32.0f); 

    for (int s = 0; s < MAX_SPLASHES; s++) {
        if (!splashes[s].enabled) continue;

        uint32_t elapsed = time_now - splashes[s].start_tick;
        float wave_radius = elapsed * speed / 10.0f;

        if (wave_radius > 255) { // Wave dissipates
            splashes[s].enabled = false;
            continue;
        }

        for (uint8_t i = led_min; i < led_max; i++) {
            float dx = g_led_config.point[i].x - splashes[s].center_x;
            float dy = g_led_config.point[i].y - splashes[s].center_y;
            float dist = hypotf(dx, dy);

            float dist_from_wave = fabsf(dist - wave_radius);

            if (dist_from_wave < WAVE_WIDTH) {
                float intensity = 1.0f - (dist_from_wave / WAVE_WIDTH);

                // Randomize color for splash effect
                uint8_t hue = HSV_BLUE + (rand() % 40 - 20);
                uint8_t sat = 255 - (rand() % 50);
                uint8_t val = intensity * 255;

                HSV hsv = {hue, sat, val};
                RGB rgb = hsv_to_rgb(hsv);
                
                // Blend with background
                uint8_t bg_val_current = (bg_brightness_step * 255) / 20;
                rgb.r = MAX(rgb.r, bg_val_current);
                rgb.g = MAX(rgb.g, bg_val_current);
                rgb.b = MAX(rgb.b, bg_val_current);

                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}
