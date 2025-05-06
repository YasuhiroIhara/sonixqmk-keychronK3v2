#include "rgb_custom_effects.h"
#include "rgb_matrix.h" // QMKのeffect_params_tを使うため
#include "matrix.h"

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix_get_row(row) & (1 << col)) != 0;
}

// 各キーの押下回数を保存するための配列を定義
uint8_t key_count[MATRIX_ROWS][MATRIX_COLS] = {{0}};

int find_led_index(uint8_t row, uint8_t col) {
    for (int i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (g_led_config.matrix_co[i][0] == row &&
            g_led_config.matrix_co[i][1] == col) {
            return i;
        }
    }
    return -1;  // 見つからなかった
}

// カスタム効果実装
bool led_canvas(effect_params_t* params) {
    // 初期化時は何もしない
    if (params->init) {
        return false;
    }
    
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    
    // 全てのLEDを消灯
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }
    
    // キーが押された時のみ処理
    for (uint8_t i = led_min; i < led_max; i++) {
        // キーの行・列を取得
        uint8_t row = g_led_config.matrix_co[i][0];
        uint8_t col = g_led_config.matrix_co[i][1];

        // 押されていればカウント増加
        if (matrix_is_on(row, col)) {
            key_count[row][col]++;
            
            // 10回を超えたら一度リセット
            if (key_count[row][col] > 10) {
                key_count[row][col] = 1;
            }
                    
            // 押下回数に応じてHSV値を決定
            HSV hsv;
            
            switch (key_count[row][col]) {
                case 1:  // 暗い青
                    hsv = (HSV){ .h = 170, .s = 255, .v = 50 };
                    break;
                case 2:  // 青
                    hsv = (HSV){ .h = 170, .s = 255, .v = 150 };
                    break;
                case 3:  // 水色
                    hsv = (HSV){ .h = 130, .s = 255, .v = 150 };
                    break;
                case 4:  // 青緑
                    hsv = (HSV){ .h = 105, .s = 255, .v = 150 };
                    break;
                case 5:  // 緑
                    hsv = (HSV){ .h = 85, .s = 255, .v = 150 };
                    break;
                case 6:  // 黄色
                    hsv = (HSV){ .h = 42, .s = 255, .v = 150 };
                    break;
                case 7:  // オレンジ
                    hsv = (HSV){ .h = 21, .s = 255, .v = 150 };
                    break;
                case 8:  // 赤
                    hsv = (HSV){ .h = 0, .s = 255, .v = 150 };
                    break;
                case 9:  // 紫
                    hsv = (HSV){ .h = 210, .s = 255, .v = 150 };
                    break;
                case 10: // 白
                    hsv = (HSV){ .h = 0, .s = 0, .v = 150 };
                    break;
                default:
                    hsv = (HSV){ .h = 0, .s = 0, .v = 0 };
                    break;
            }
            
            // HSVからRGBに変換して色をセット
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    
    // 既に点灯しているLEDも含めて全部のキーをスキャン
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (key_count[row][col] > 0) {
                int led_index = find_led_index(row, col);
                if (led_index >= 0) {
                    // 押下回数に応じて色を表示
                    HSV hsv;
                    switch (key_count[row][col]) {
                        case 1:  // 暗い青
                            hsv = (HSV){ .h = 170, .s = 255, .v = 50 };
                            break;
                        case 2:  // 青
                            hsv = (HSV){ .h = 170, .s = 255, .v = 150 };
                            break;
                        case 3:  // 水色
                            hsv = (HSV){ .h = 130, .s = 255, .v = 150 };
                            break;
                        case 4:  // 青緑
                            hsv = (HSV){ .h = 105, .s = 255, .v = 150 };
                            break;
                        case 5:  // 緑
                            hsv = (HSV){ .h = 85, .s = 255, .v = 150 };
                            break;
                        case 6:  // 黄色
                            hsv = (HSV){ .h = 42, .s = 255, .v = 150 };
                            break;
                        case 7:  // オレンジ
                            hsv = (HSV){ .h = 21, .s = 255, .v = 150 };
                            break;
                        case 8:  // 赤
                            hsv = (HSV){ .h = 0, .s = 255, .v = 150 };
                            break;
                        case 9:  // 紫
                            hsv = (HSV){ .h = 210, .s = 255, .v = 150 };
                            break;
                        case 10: // 白
                            hsv = (HSV){ .h = 0, .s = 0, .v = 150 };
                            break;
                        default:
                            hsv = (HSV){ .h = 0, .s = 0, .v = 0 };
                            break;
                    }
                    RGB rgb = hsv_to_rgb(hsv);
                    rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
                }
            }
        }
    }
    
    return rgb_matrix_check_finished_leds(led_max);
}
