#include "rgb_custom_effects.h"
#include <math.h>   // For sqrtf and powf
#include <stdlib.h> // For rand()

// --- Configuration ---
#define MAX_WAVES 6         // Maximum number of concurrent waves
#define WAVE_WIDTH 40.0f    // How wide the ripple is (物理座標単位) - 波の厚み
#define WAVE_DURATION 8000  // How long a wave lasts in ms (8 seconds)
#define WAVE_SPEED 5.0f     // Wave speed in keys per second

// --- Data Structures ---
typedef struct {
    int16_t center_x;    // 物理座標のX（LEDの実際の位置）
    int16_t center_y;    // 物理座標のY（LEDの実際の位置）
    uint16_t timestamp;  // Time the wave was created
    bool active;         // Whether this wave slot is in use
    uint8_t wave_id;     // Unique ID for consistent coloring
} wave_t;

// --- Global State ---
static wave_t waves[MAX_WAVES];
static uint8_t background_level = 0; // 0-20, where 0 is black, 20 is white
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

// Calculate wave intensity based on distance from wave center
static float calculate_wave_intensity(float distance, float current_radius, float elapsed_time) {
    // 波がまだ到達していない場合は0
    if (distance > current_radius + WAVE_WIDTH) {
        return 0.0f;
    }
    
    // 時間による全体的な減衰（8秒で0になる）
    float time_factor = 1.0f - (elapsed_time / WAVE_DURATION);
    if (time_factor <= 0.0f) {
        return 0.0f;
    }
    
    // 距離による強度計算
    float intensity = 0.0f;
    
    if (distance <= current_radius) {
        // 波が通過した領域：中心からの距離に基づいて減衰
        float distance_factor = 1.0f - (distance / (current_radius + WAVE_WIDTH));
        distance_factor = fmaxf(0.0f, distance_factor);
        intensity = distance_factor;
    } else {
        // 波の前線部分：波の幅内であれば光る
        float front_distance = distance - current_radius;
        if (front_distance <= WAVE_WIDTH) {
            float front_factor = 1.0f - (front_distance / WAVE_WIDTH);
            intensity = front_factor;
        }
    }
    
    // 全体の強度に時間減衰を適用
    intensity *= time_factor;
    
    // スムーズな減衰カーブ
    intensity = intensity * intensity * (3.0f - 2.0f * intensity);
    
    return intensity;
}

// --- Core Logic ---

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
    
    // キーが押されている場合、新しい波を作成
    if (g_last_hit_tracker.count > 0) {
        // Get data for the last key press by using g_last_hit_tracker
        uint8_t last_hit_index = g_last_hit_tracker.count - 1;
        int16_t center_x = g_last_hit_tracker.x[last_hit_index];
        int16_t center_y = g_last_hit_tracker.y[last_hit_index];

        // 新しい波として追加（既存の波と同じ位置でも重複を許可）
        bool should_create_wave = true;
        
        // 同じ位置に最近作られた波があるかチェック（重複防止）
        for (int w = 0; w < MAX_WAVES; w++) {
            if (waves[w].active && 
                waves[w].center_x == center_x && 
                waves[w].center_y == center_y &&
                timer_elapsed(waves[w].timestamp) < 100) { // 100ms以内の重複は防ぐ
                should_create_wave = false;
                break;
            }
        }
        
        if (should_create_wave) {
            // 空いているスロットを探す、なければ最古のものを上書き
            int target_slot = -1;
            uint16_t oldest_time = 0;
            
            for (int w = 0; w < MAX_WAVES; w++) {
                if (!waves[w].active) {
                    target_slot = w;
                    break;
                }
                uint16_t elapsed = timer_elapsed(waves[w].timestamp);
                if (elapsed > oldest_time) {
                    oldest_time = elapsed;
                    target_slot = w;
                }
            }
            
            waves[target_slot] = (wave_t){
                .center_x = center_x,
                .center_y = center_y,
                .timestamp = timer_read(),
                .active = true,
                .wave_id = next_wave_id++
            };
        }
    }

    // Map QMK animation speed (0-255) to wave speed multiplier (0.5x to 2.0x)
    float speed_multiplier = map_value(rgb_matrix_config.speed, 0, 255, 0.5f, 2.0f);
    float actual_wave_speed = WAVE_SPEED * speed_multiplier * 25.0f; // より高速でキーボード端まで到達

    HSV background_hsv = {0, 0, map_value(background_level, 0, 20, 0, 255)};
    RGB background_rgb = hsv_to_rgb(background_hsv);

    for (uint8_t i = led_min; i < led_max; i++) {
        // LEDの物理座標を取得
        int16_t led_x = g_led_config.point[i].x;
        int16_t led_y = g_led_config.point[i].y;

        // Start with background color
        RGB final_color = background_rgb;
        float total_intensity = 0.0f;
        RGB accumulated_color = {0, 0, 0};

        // Check against all active waves
        for (int w = 0; w < MAX_WAVES; w++) {
            if (!waves[w].active) continue;

            uint16_t elapsed = timer_elapsed(waves[w].timestamp);
            if (elapsed >= WAVE_DURATION) {  // >= を使用して確実に8秒で消去
                waves[w].active = false;
                continue;
            }

            // Calculate current wave radius (時間経過に基づいて半径を拡大)
            float current_radius = (elapsed / 1000.0f) * actual_wave_speed;

            // Calculate distance from LED to wave center (物理座標で計算)
            float dx = led_x - waves[w].center_x;
            float dy = led_y - waves[w].center_y;
            float distance = sqrtf(dx * dx + dy * dy);

            // Calculate wave intensity
            float intensity = calculate_wave_intensity(distance, current_radius, (float)elapsed);
            
            if (intensity > 0.01f) { // 最小閾値を下げる
                RGB wave_color = get_wave_color(waves[w].wave_id, distance / (current_radius + 1.0f), intensity);
                
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