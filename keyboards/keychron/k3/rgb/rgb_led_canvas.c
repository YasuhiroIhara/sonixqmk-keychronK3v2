#include "rgb_custom_effects.h"
#include "rgb_matrix.h"
#include "matrix.h"
#include <stdlib.h> // For rand()
#include "timer.h"  // For wait_ms()

// 各キーの押下回数を保存するための配列
static uint8_t key_count[MATRIX_ROWS][MATRIX_COLS] = {{0}};

// k3.cから呼び出され、キーの押下回数をカウントする
void process_rgb_led_canvas(uint8_t row, uint8_t col) {
    key_count[row][col]++;
    // 10回を超えたらリセット
    if (key_count[row][col] > 10) {
        key_count[row][col] = 0;
    }
}

// ledの物理的なindexを探す
static int find_led_index(uint8_t row, uint8_t col) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (g_led_config.matrix_co[i][0] == row && g_led_config.matrix_co[i][1] == col) {
            return i;
        }
    }
    return -1; // 見つからなかった
}

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
        case 10: hsv = (HSV){0,   0,   150}; break; // 白
        default: hsv = (HSV){0,   0,   0};   break; // 消灯
    }
    return hsv;
}

// カスタム効果の実装
bool led_canvas(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        // カウントをリセット
        memset(key_count, 0, sizeof(key_count));

        // 開始時に3回白く点滅
        for (int i = 0; i < 3; i++) {
            rgb_matrix_set_color_all(255, 255, 255);
            rgb_matrix_update_pwm_buffers(); // 色を即時反映
            wait_ms(500); // 0.5秒待機
            rgb_matrix_set_color_all(0, 0, 0);
            rgb_matrix_update_pwm_buffers(); // 色を即時反映
            wait_ms(500); // 0.5秒待機
        }
        return false;
    }

    // 全てのキーをスキャンし、押下回数に基づいて色を決定する
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            int led_index = find_led_index(row, col);
            if (led_index != -1 && led_index >= led_min && led_index < led_max) {
                uint8_t count = key_count[row][col];
                
                if (count > 0) {
                    HSV hsv = get_hsv_from_count(count);

                    // 色相にランダムな値を加える (-10 から +10)
                    int random_hue_offset = (rand() % 21) - 10;
                    hsv.h += random_hue_offset;

                    RGB rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
                } else {
                    // カウントが0の場合は消灯
                    rgb_matrix_set_color(led_index, 0, 0, 0);
                }
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}