#include "rgb_custom_effects.h"
#include <math.h>   // For sqrtf and powf
#include <stdlib.h> // For rand()

// --- Configuration ---
#define MAX_WAVES 6         // Maximum number of concurrent waves
#define WAVE_WIDTH 8.0f     // 波の厚み
#define WAVE_DURATION 1500  // 各波は1.5秒で消える
#define WAVE_SPEED 10.0f    // 速度を上げて素早く広がる
#define GLOBAL_FADEOUT 3000 // 最後のキー押下から3秒で全消灯

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
static uint16_t last_keypress_time = 0; // 最後のキー押下時刻
static int16_t last_hit_x = -1;         // 前回のヒット位置X
static int16_t last_hit_y = -1;         // 前回のヒット位置Y
static uint16_t last_processed_time = 0; // 前回処理した時刻

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
    // 時間による全体的な減衰（急峻なカーブ）
    float time_factor = 1.0f - (elapsed_time / WAVE_DURATION);
    if (time_factor <= 0.0f) {
        return 0.0f;
    }
    
    // 4乗カーブでより急速に減衰
    time_factor = time_factor * time_factor * time_factor * time_factor;
    
    // 距離による強度計算
    float intensity = 0.0f;
    
    // 波の前線からの距離
    float distance_from_front = fabsf(distance - current_radius);
    
    if (distance_from_front <= WAVE_WIDTH) {
        // 波の幅内にいる場合のみ光る
        float position_in_wave = distance_from_front / WAVE_WIDTH;
        
        // ガウス曲線的な減衰（中心が最も明るい）
        intensity = expf(-5.0f * position_in_wave * position_in_wave);
        
        // 波が通過した後は急速に減衰
        if (distance < current_radius) {
            float trail_factor = fminf(1.0f, (current_radius - distance) / (WAVE_WIDTH * 1.5f));
            intensity *= (1.0f - trail_factor * 0.9f); // 通過後は10%まで減衰
        }
    }
    
    // 全体の強度に時間減衰を適用
    intensity *= time_factor;
    
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
        last_keypress_time = 0;
        last_hit_x = -1;
        last_hit_y = -1;
        last_processed_time = 0;
    }
    
    // 新しいキー押下を検出
    if (g_last_hit_tracker.count > 0) {
        uint8_t last_hit_index = g_last_hit_tracker.count - 1;
        int16_t center_x = g_last_hit_tracker.x[last_hit_index];
        int16_t center_y = g_last_hit_tracker.y[last_hit_index];
        uint16_t current_time = timer_read();
        
        // 位置が変わった場合のみ、または同じ位置でも100ms以上経過した場合のみ
        bool is_new_position = (center_x != last_hit_x || center_y != last_hit_y);
        bool enough_time_passed = (last_processed_time == 0) || (timer_elapsed(last_processed_time) >= 100);
        
        if (is_new_position && enough_time_passed) {
            // 最後のキー押下時刻を更新
            last_keypress_time = current_time;
            last_processed_time = current_time;
            last_hit_x = center_x;
            last_hit_y = center_y;

            // 新しい波として追加
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
            
            if (target_slot >= 0) {
                waves[target_slot] = (wave_t){
                    .center_x = center_x,
                    .center_y = center_y,
                    .timestamp = current_time,
                    .active = true,
                    .wave_id = next_wave_id++
                };
            }
        }
    }

    // Map QMK animation speed (0-255) to wave speed multiplier (0.5x to 2.0x)
    float speed_multiplier = map_value(rgb_matrix_config.speed, 0, 255, 0.5f, 2.0f);
    float actual_wave_speed = WAVE_SPEED * speed_multiplier * 25.0f;

    // 最後のキー押下からの経過時間に基づくグローバルフェード
    float global_fade = 1.0f;
    if (last_keypress_time > 0) {
        uint16_t time_since_last_key = timer_elapsed(last_keypress_time);
        if (time_since_last_key >= GLOBAL_FADEOUT) {
            global_fade = 0.0f;
            // 全ての波を無効化
            for (int w = 0; w < MAX_WAVES; w++) {
                waves[w].active = false;
            }
        } else {
            // 3秒かけて徐々にフェードアウト（3乗カーブ）
            float fade_progress = (float)time_since_last_key / GLOBAL_FADEOUT;
            fade_progress = fade_progress * fade_progress * fade_progress; // 3乗
            global_fade = 1.0f - fade_progress;
        }
    }

    // グローバルフェードが0なら全て消灯
    if (global_fade <= 0.0f) {
        for (uint8_t i = led_min; i < led_max; i++) {
            HSV background_hsv = {0, 0, map_value(background_level, 0, 20, 0, 255)};
            RGB background_rgb = hsv_to_rgb(background_hsv);
            rgb_matrix_set_color(i, background_rgb.r, background_rgb.g, background_rgb.b);
        }
        return rgb_matrix_check_finished_leds(led_max);
    }

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
            if (elapsed >= WAVE_DURATION) {
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
            
            if (intensity > 0.001f) {
                RGB wave_color = get_wave_color(waves[w].wave_id, distance / (current_radius + 1.0f), intensity);
                
                // Accumulate colors with proper blending
                accumulated_color.r += (uint8_t)(wave_color.r * intensity);
                accumulated_color.g += (uint8_t)(wave_color.g * intensity);
                accumulated_color.b += (uint8_t)(wave_color.b * intensity);
                total_intensity += intensity;
            }
        }

        // グローバルフェードを適用
        total_intensity *= global_fade;

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