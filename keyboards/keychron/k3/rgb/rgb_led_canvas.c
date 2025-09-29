#include "rgb_custom_effects.h"
#include "matrix.h"
#include <stdlib.h> // For rand()
#include "timer.h"  // For wait_ms()
#include "config.h"

// 各キーの押下回数を保存するための配列
static uint8_t key_count[DRIVER_LED_TOTAL] = {0};

// 押下回数に応じたHSVカラーを取得するヘルパー関数
static HSV get_hsv_from_count(uint8_t count) {
    HSV hsv;
    switch (count) {
        case 1:  hsv = (HSV){170, 255, 50};  break; // 暗い青
        case 2:  hsv = (HSV){170, 255, 150}; break; // 青
        case 3:  hsv = (HSV){130, 255, 150}; break; // 水色
        case 4:  hsv = (HSV){105, 255, 150}; break; // 青緑
        case 5:  hsv = (HSV){85,  255, 150}; break; // 緑
        case 6:  hsv = (HSV){42,  255, 150}; break; // 黄色
        case 7:  hsv = (HSV){21,  255, 150}; break; // オレンジ
        case 8:  hsv = (HSV){0,   255, 150}; break; // 赤
        case 9:  hsv = (HSV){210, 255, 150}; break; // 紫
        case 10: hsv = (HSV){0,   0,   0}; break;   // 消灯
        default: hsv = (HSV){0,   0,   0};   break; // 消灯
    }
    return hsv;
}

// カスタム効果の実装
bool led_canvas(effect_params_t* params) {
    static uint8_t prev_led = 255; // 255: 無効値
    static RGB prev_rgb = {0, 0, 0};

    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // キーが押された場合に限り、押下回数を更新し、LEDの色を設定
    // LED色を設定すると、次のキーが押されるまでその色が維持される
    if (g_last_hit_tracker.count > 0) {
        // 最後に押されたキーの物理座標を取得
        uint8_t last_hit_index = g_last_hit_tracker.count - 1;
        int16_t center_x = g_last_hit_tracker.x[last_hit_index];
        int16_t center_y = g_last_hit_tracker.y[last_hit_index];
        
        // 最も近いLEDを見つける
        uint8_t closest_led = 0;
        uint16_t min_distance = UINT16_MAX;
        
        for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
            int16_t dx = g_led_config.point[i].x - center_x;
            int16_t dy = g_led_config.point[i].y - center_y;
            uint16_t distance = dx * dx + dy * dy;
            
            if (distance < min_distance) {
                min_distance = distance;
                closest_led = i;
            }
        }
        
        // 新しいキーが押された場合のみ色を更新
        if (closest_led != prev_led) {
            // 押下回数を更新
            key_count[closest_led]++;
            if (key_count[closest_led] > 10) {
                key_count[closest_led] = 0;
            }
            // LEDの色を設定
            HSV hsv = get_hsv_from_count(key_count[closest_led]);
            int random_hue_offset = (rand() % 64);
            hsv.h = (hsv.h + random_hue_offset) % 256;
            hsv.s = (hsv.s * 0.7);
            RGB rgb = hsv_to_rgb(hsv);
            prev_rgb = rgb;
            rgb_matrix_set_color(closest_led, prev_rgb.r, prev_rgb.g, prev_rgb.b);
            prev_led = closest_led;
        } 

        /* 全LEDを押下回数に応じた色で更新する場合のコード
        for (uint8_t i = led_min; i < led_max; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            
            uint8_t count = key_count[i];
            if (count > 0) {
                HSV hsv = get_hsv_from_count(count);
                
                // ランダムな色相オフセット
                int random_hue_offset = (rand() % 64);
                hsv.h = (hsv.h + random_hue_offset) % 256;
                // 彩度を落として淡い色合いにする
                hsv.s = (hsv.s * 0.7);  // 彩度を70%に減少

                RGB rgb = hsv_to_rgb(hsv);
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            } else {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }*/
    }

    return rgb_matrix_check_finished_leds(led_max);
}