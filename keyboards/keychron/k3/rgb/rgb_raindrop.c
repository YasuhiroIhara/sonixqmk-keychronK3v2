#ifndef qsub8
#define qsub8(a, b) ((a) > (b) ? (a) - (b) : 0)
#endif

#include "quantum.h"               // QMKの基本関数と型定義
#include "rgb_matrix_types.h"     // effect_params_t など
#include "color.h"
#include "rgb_matrix.h"           // rgb_matrix_hsv_to_rgb を含む
#include "progmem.h"              // qsub8 を含む

#include "rgb_custom_effects.h"

// 色・彩度・明度を保持する構造体
typedef struct {
    uint8_t h;
    uint8_t s;
  } raindrop_hs_t;
  
  // 色相・彩度の情報バッファ
  static raindrop_hs_t raindrop_hs_buffer[MATRIX_ROWS][MATRIX_COLS] = {0};
  static uint16_t last_drop_time = 0;
  // 青い雨粒のようなエフェクト
  bool my_raindrop(effect_params_t* params) {
    if (!params->init) {
      uint16_t now = timer_read();
     
      // 500msごとに1個だけ光らせる
      if (timer_elapsed(last_drop_time) > 500) {
          last_drop_time = now;
  
          int index = rand() % DRIVER_LED_TOTAL;
          uint8_t row = index % MATRIX_ROWS;
          uint8_t col = index / MATRIX_ROWS;
  
          g_rgb_frame_buffer[row][col] = 255;  // 明度最大（点灯）
  
          // 色・彩度を記憶（青あたり）
          raindrop_hs_buffer[row][col].h = 115 + (rand() % 60);
          raindrop_hs_buffer[row][col].s = 200 + (rand() % 56);
      }
  
      // フレームバッファの減衰処理・描画
      RGB_MATRIX_USE_LIMITS(led_min, led_max);
      for (int i = led_min; i < led_max; i++) {
          uint8_t row = i % MATRIX_ROWS;
          uint8_t col = i / MATRIX_ROWS;
          uint8_t val = g_rgb_frame_buffer[row][col];
  
          if (val > 0) {
              HSV hsv = {
                  raindrop_hs_buffer[row][col].h,
                  raindrop_hs_buffer[row][col].s,
                  val
              };
              RGB rgb = hsv_to_rgb(hsv);
              rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
  
              g_rgb_frame_buffer[row][col] = qsub8(val, 1);  // もっとも遅い減衰速度
          } else {
              rgb_matrix_set_color(i, 0, 0, 0);
          }
      }
    }
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
  }