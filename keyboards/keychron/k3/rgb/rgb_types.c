#include "rgb_custom_effects.h"
#include "rgb_matrix_types.h"
#include "rgb_matrix.h"
#include "color.h"       // For RGB/HSV structures and conversion functions
#include "led_tables.h"  // For brightness scaling functions
#include "quantum.h"     // For timer functions

// Include this for the qadd8 and qsub8 functions
#include "lib/lib8tion/lib8tion.h"

#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_TYPING_HEATMAP)

# ifndef RGB_MATRIX_TYPING_HEATMAP_DECREASE_DELAY_MS
# define RGB_MATRIX_TYPING_HEATMAP_DECREASE_DELAY_MS 1000  // orginal:25
# endif

void process_my_typewriter_effect(uint8_t row, uint8_t col) {
    // Only light up the pressed key, no spreading to adjacent keys
    // k3.c から呼び出される
    g_rgb_frame_buffer[row][col] = qadd8(g_rgb_frame_buffer[row][col], 32);
    // 最大値を超えたらリセット
    if (g_rgb_frame_buffer[row][col] > 220) {
        g_rgb_frame_buffer[row][col] = 0;
    }
}

// A timer to track the last time we decremented all heatmap values.
static uint16_t heatmap_decrease_timer;
// Whether we should decrement the heatmap values during the next update.
static bool decrease_heatmap_values;

bool my_typewriter(effect_params_t* params) {
    // Modified version of RGB_MATRIX_USE_LIMITS to work off of matrix row / col size
    uint8_t led_min = RGB_MATRIX_LED_PROCESS_LIMIT * params->iter;
    uint8_t led_max = led_min + RGB_MATRIX_LED_PROCESS_LIMIT;
    if (led_max > sizeof(g_rgb_frame_buffer)) led_max = sizeof(g_rgb_frame_buffer);

    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);
        memset(g_rgb_frame_buffer, 0, sizeof g_rgb_frame_buffer);
    }

    // アニメーション速度の制御。decayのみ
    if (params->iter == 0) {
        decrease_heatmap_values = timer_elapsed(heatmap_decrease_timer) >= RGB_MATRIX_TYPING_HEATMAP_DECREASE_DELAY_MS;
        // Restart the timer if we are going to decrease the heatmap this frame.時間が経過したらタイマーをリセット
        if (decrease_heatmap_values) {
            heatmap_decrease_timer = timer_read();
        }
    }

    // Render key lighting & decrease
    for (int i = led_min; i < led_max; i++) {
        uint8_t row = i % MATRIX_ROWS;
        uint8_t col = i / MATRIX_ROWS;
        uint8_t val = g_rgb_frame_buffer[row][col];

        if (val > 0) {  // Only process keys with some brightness value
            // set the pixel colour
            uint8_t led[LED_HITS_TO_REMEMBER];
            uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);
            
            for (uint8_t j = 0; j < led_count; ++j) {
                if (!HAS_ANY_FLAGS(g_led_config.flags[led[j]], params->flags)) continue;
                
                // Simplified color scheme - just use the value directly for brightness
                HSV hsv = {rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, scale8(val, rgb_matrix_config.hsv.v)};
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(led[j], rgb.r, rgb.g, rgb.b);
            }
        }

        if (decrease_heatmap_values) {
            g_rgb_frame_buffer[row][col] = qsub8(val, 1);
        }
    }

    return led_max < sizeof(g_rgb_frame_buffer);
}

#endif // defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_TYPING_HEATMAP)