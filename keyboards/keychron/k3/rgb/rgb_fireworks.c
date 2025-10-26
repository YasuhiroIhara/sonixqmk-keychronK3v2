#include "rgb_custom_effects.h"
#include <math.h>

#define MAX_FIREWORKS 10
#define MAX_HITS 10
#define SPREAD_SPEED 5 // keys per second
#define SPARKLE_DURATION 1000 // ms
#define SPARKLE_INTERVAL 100 // ms
#define SPARKLE_EDGE_WIDTH 1.0f // 外周の幅
#define FADE_DURATION 2000 // ms
#define INNER_FADE_DURATION 300 // ms 

typedef enum {
    INACTIVE,
    SPREADING,
    SPARKLING,
    FADING
} firework_state_t;

typedef struct {
    firework_state_t state;
    uint8_t center_x;
    uint8_t center_y;
    uint8_t hue;
    uint8_t sat;
    uint8_t max_radius;
    uint32_t start_time;
} firework_t;

static firework_t fireww[MAX_FIREWORKS];
static uint8_t next_firework_index = 0; // 次に書き込む位置
static uint8_t prev_hit_index[MAX_HITS]; // 前回のg_last_hit_tracker.indexのコピー
static uint8_t prev_hit_count = 0; // 前回のg_last_hit_tracker.count
static bool initialized = false; // 初期化フラグ

// 花火データを初期化する関数
static void clear_firework(int index) {
    fireww[index].state = INACTIVE;
    fireww[index].center_x = 0;
    fireww[index].center_y = 0;
    fireww[index].hue = 0;
    fireww[index].sat = 0;
    fireww[index].max_radius = 0;
    fireww[index].start_time = 0;
}

// 全ての花火データをクリアする関数
static void clear_all_fireworks(void) {
    for (int i = 0; i < MAX_FIREWORKS; i++) {
        clear_firework(i);
    }
    next_firework_index = 0;
}

// 初期化関数
static void init_fireworks(void) {
    clear_all_fireworks();
    prev_hit_count = 0;
    for (uint8_t i = 0; i < MAX_HITS; ++i) {
        prev_hit_index[i] = NO_LED;
    }
    initialized = true;
}

// アクティブな花火が存在するか確認
static bool is_any_firework_active(void) {
    for (int i = 0; i < MAX_FIREWORKS; i++) {
        if (fireww[i].state != INACTIVE) {
            return true;
        }
    }
    return false;
}

// 古いものから順に上書きする方式で花火を発射
static void launch_firework(uint8_t x, uint8_t y) {
    int index = next_firework_index;
    
    // 新しい花火を設定
    fireww[index].state = SPREADING;
    fireww[index].center_x = x;
    fireww[index].center_y = y;
    fireww[index].hue = rand() % 256;
    fireww[index].sat = 255;
    fireww[index].max_radius = (rand() % 6) + 3; // 3-8 keys
    fireww[index].start_time = timer_read32();
    
    // 次のインデックスに進む（循環バッファ）
    next_firework_index = (next_firework_index + 1) % MAX_FIREWORKS;
}

bool fireworks(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // 初回起動時の初期化
    if (!initialized) {
        init_fireworks();
    }

    // 新しいキー入力があった場合の処理
    if (g_last_hit_tracker.count > 0) {
        // 現在のg_last_hit_tracker.indexと前回のものを比較
        for (uint8_t i = 0; i < g_last_hit_tracker.count && i < MAX_HITS; ++i) {
            uint8_t led_index = g_last_hit_tracker.index[i];
            
            if (led_index != NO_LED && led_index < DRIVER_LED_TOTAL) {
                // 前回の配列に同じLEDインデックスが存在するかチェック
                bool already_processed = false;
                
                if (prev_hit_count > 0) {
                    for (uint8_t j = 0; j < prev_hit_count && j < MAX_HITS; ++j) {
                        if (prev_hit_index[j] == led_index) {
                            already_processed = true;
                            break;
                        }
                    }
                }
                
                // 未処理のキーなら花火を発射
                if (!already_processed) {
                    launch_firework(g_led_config.point[led_index].x, g_led_config.point[led_index].y);
                }
            }
        }

        // 現在のキー情報を保存（配列全体をコピー）
        prev_hit_count = g_last_hit_tracker.count;
        for (uint8_t i = 0; i < MAX_HITS; ++i) {
            if (i < g_last_hit_tracker.count) {
                prev_hit_index[i] = g_last_hit_tracker.index[i];
            } else {
                prev_hit_index[i] = NO_LED; // 無効値
            }
        }
    } else {
        // キーが押されていない状態 - リセット
        if (prev_hit_count > 0) {
            prev_hit_count = 0;
            for (uint8_t i = 0; i < MAX_HITS; ++i) {
                prev_hit_index[i] = NO_LED;
            }
        }
    }

    // 全ての花火が終了した場合、データをクリアして次に備える
    if (!is_any_firework_active() && prev_hit_count == 0) {
        clear_all_fireworks();
        return rgb_matrix_check_finished_leds(led_max);
    }

    uint32_t now = timer_read32();
    uint8_t v_buffer[DRIVER_LED_TOTAL] = {0};
    uint8_t h_buffer[DRIVER_LED_TOTAL] = {0};
    uint8_t s_buffer[DRIVER_LED_TOTAL] = {0};
    bool needs_update = false;

    for (int i = 0; i < MAX_FIREWORKS; i++) {
        if (fireww[i].state == INACTIVE) continue;

        needs_update = true;
        uint32_t elapsed = now - fireww[i].start_time;

        switch (fireww[i].state) {
            case SPREADING: {
                float current_radius = (float)elapsed / 1000.0f * SPREAD_SPEED;
                if (current_radius > fireww[i].max_radius) {
                    fireww[i].state = SPARKLING;
                    fireww[i].start_time = now;
                    elapsed = 0;
                }

                for (uint8_t j = 0; j < DRIVER_LED_TOTAL; j++) {
                    float dx = g_led_config.point[j].x - fireww[i].center_x;
                    float dy = g_led_config.point[j].y - fireww[i].center_y;
                    float distance = sqrtf(dx * dx + dy * dy) / 8.0f;

                    if (distance <= current_radius) {
                        float distance_from_edge = current_radius - distance;
                        
                        float edge_brightness = 1.0f;
                        if (distance_from_edge > 0.5f) {
                            edge_brightness = 1.0f - ((distance_from_edge - 0.5f) / fireww[i].max_radius);
                            if (edge_brightness < 0.0f) edge_brightness = 0.0f;
                        }
                        
                        float inner_fade = 1.0f;
                        if (distance < current_radius - 0.5f) {
                            float time_since_passed = (current_radius - distance) / SPREAD_SPEED * 1000.0f;
                            if (time_since_passed > INNER_FADE_DURATION) {
                                inner_fade = 0.0f;
                            } else {
                                inner_fade = 1.0f - (time_since_passed / INNER_FADE_DURATION);
                            }
                        }
                        
                        uint8_t v = 255 * edge_brightness * inner_fade;
                        
                        if (v > v_buffer[j]) {
                           v_buffer[j] = v;
                           h_buffer[j] = fireww[i].hue;
                           s_buffer[j] = fireww[i].sat;
                        }
                    }
                }
                break;
            }
            case SPARKLING: {
                if (elapsed > SPARKLE_DURATION) {
                    fireww[i].state = FADING;
                    fireww[i].start_time = now;
                    elapsed = 0;
                }

                bool sparkle_on = (elapsed / SPARKLE_INTERVAL) % 2 == 0;
                
                for (uint8_t j = 0; j < DRIVER_LED_TOTAL; j++) {
                    float dx = g_led_config.point[j].x - fireww[i].center_x;
                    float dy = g_led_config.point[j].y - fireww[i].center_y;
                    float distance = sqrtf(dx * dx + dy * dy) / 8.0f;

                    float distance_from_edge = fabsf(distance - fireww[i].max_radius);
                    
                    if (distance_from_edge <= SPARKLE_EDGE_WIDTH) {
                        if (sparkle_on) {
                            v_buffer[j] = 255;
                            h_buffer[j] = fireww[i].hue;
                            s_buffer[j] = fireww[i].sat;
                        } else {
                            v_buffer[j] = 255;
                            h_buffer[j] = 0;
                            s_buffer[j] = 0;
                        }
                    }
                }
                break;
            }
            case FADING: {
                if (elapsed > FADE_DURATION) {
                    fireww[i].state = INACTIVE;
                    for (uint8_t j = 0; j < DRIVER_LED_TOTAL; j++) {
                        float dx = g_led_config.point[j].x - fireww[i].center_x;
                        float dy = g_led_config.point[j].y - fireww[i].center_y;
                        float distance = sqrtf(dx * dx + dy * dy) / 8.0f;

                        if (distance <= fireww[i].max_radius + SPARKLE_EDGE_WIDTH) {
                            rgb_matrix_set_color(j, 0, 0, 0);
                        }
                    }
                    clear_firework(i);
                    continue;
                }
                float fade_progress = (float)elapsed / FADE_DURATION;
                uint8_t v = 255 * (1.0f - fade_progress);

                for (uint8_t j = 0; j < DRIVER_LED_TOTAL; j++) {
                    float dx = g_led_config.point[j].x - fireww[i].center_x;
                    float dy = g_led_config.point[j].y - fireww[i].center_y;
                    float distance = sqrtf(dx * dx + dy * dy) / 8.0f;

                    float distance_from_edge = fabsf(distance - fireww[i].max_radius);
                    
                    if (distance_from_edge <= SPARKLE_EDGE_WIDTH) {
                        if (v > v_buffer[j]) {
                            v_buffer[j] = v;
                            h_buffer[j] = fireww[i].hue;
                            s_buffer[j] = fireww[i].sat;
                        }
                    }
                }
                break;
            }
            case INACTIVE:
                break;
        }
    }

    if (needs_update) {
        for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
            if (v_buffer[i] > 0) {
                HSV hsv = {h_buffer[i], s_buffer[i], v_buffer[i]};
                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}