/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-07 11:10:31
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 18:09:40
 * FilePath: \pmbusmonitor\src\lvgl_ports.cpp
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#include "lvgl_ports.h"
// TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
// TFT_eSPI tft = TFT_eSPI(screenHeight,screenWidth); /* TFT instance */
TFT_eSPI tft = TFT_eSPI();

lv_obj_t *contanier1 = NULL;
lv_obj_t *contanier2 = NULL;
lv_obj_t *dev_info = NULL;
lv_obj_t *voltage_in = NULL;
lv_obj_t *current_in = NULL;
lv_obj_t *power_in = NULL;
lv_obj_t *voltage_out = NULL;
lv_obj_t *current_out = NULL;
lv_obj_t *power_out = NULL;
lv_obj_t *fan_1 = NULL;
lv_obj_t *temperature_1 = NULL;
lv_obj_t *efficiency = NULL;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

//   tft.setSwapBytes(true);
//   tft.pushImageDMA(area->x1, area->y1, w, h,(uint16_t *)&color_p->full);

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// #if LV_USE_LOG != 0
// void my_print(const char *buf) { Serial.printf("%s \r\n", buf); }
// #endif

void lvgl_ui_init(void){
  //design ui
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE_GREY), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);

  // lv_style_t my_style;
  // lv_style_init(&my_style);
  // lv_style_set_radius(&my_style, 0);
  // lv_style_set_border_width(&my_style, 0);
  // lv_style_set_outline_width(&my_style, 0);
  // lv_style_set_outline_pad(&my_style, 0);
  // lv_style_set_shadow_width(&my_style, 0);
  // lv_style_set_bg_color(&my_style, lv_color_hex(0xFFFFFFF));
  // lv_style_set_pad_all(&my_style, 0);
  contanier1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_radius(contanier1, 0, 0);
  lv_obj_set_style_border_width(contanier1, 0, 0);
  lv_obj_set_style_pad_all(contanier1, 0, 0);
  lv_obj_set_size(contanier1, 160, 80);
  lv_obj_set_style_bg_color(contanier1, lv_color_make(0x0, 0x0, 0x0), 0);
  // lv_obj_add_style(contanier1, &my_style, 0);
  contanier2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_radius(contanier2, 0, 0);
  lv_obj_set_style_border_width(contanier2, 0, 0);
  lv_obj_set_style_pad_all(contanier2, 0, 0);
  lv_obj_set_size(contanier2, 160, 80);
  lv_obj_set_style_bg_color(contanier2, lv_color_make(0x0, 0x0, 0x0), 0);

  //test, for now
  lv_obj_add_flag(contanier1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(contanier2, LV_OBJ_FLAG_HIDDEN);

  /*Create a style for the shadow*/
  static lv_style_t style_shadow;
  lv_style_init(&style_shadow);
  lv_style_set_text_opa(&style_shadow, LV_OPA_70);
  lv_style_set_text_color(&style_shadow, lv_color_black());
  lv_style_set_text_font(&style_shadow, &lv_font_montserrat_20);
  lv_style_set_text_align(&style_shadow, LV_TEXT_ALIGN_CENTER);

  /*Create a label for the shadow first (it's in the background)*/
  lv_obj_t * shadow_label = lv_label_create(contanier2);
  lv_obj_add_style(shadow_label, &style_shadow, 0);
  lv_obj_set_size(shadow_label, 150, 40);

  lv_obj_t *startimage = lv_label_create(contanier2);
  lv_obj_set_size(startimage, 150, 40);
  lv_obj_align(startimage, LV_ALIGN_CENTER, 0, -5);
  lv_obj_set_style_text_align(startimage, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(startimage, &lv_font_montserrat_20, 0);
  lv_label_set_recolor(startimage, true);
  lv_label_set_text(startimage, "#FFFF00 PMBUS#\n#FFFF00 MONITOR#");

  /*Set the same text for the shadow label*/
  lv_label_set_recolor(shadow_label, true);
  lv_label_set_text(shadow_label, lv_label_get_text(startimage));

  /*Shift the second label down and to the right by 2 pixel*/
  lv_obj_align_to(shadow_label, startimage, LV_ALIGN_TOP_LEFT, 2, 2);

  // lv_obj_t *dev_info = lv_label_create(lv_scr_act());
  dev_info = lv_label_create(contanier1);
  lv_obj_set_size(dev_info, 158, 14);
  //lv_obj_set_pos(dev_info, 0, 0);
  lv_obj_align(dev_info, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_label_set_text_fmt(dev_info, "PMBUSMONITOR");
  lv_obj_set_style_text_align(dev_info, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_set_style_bg_color(dev_info, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(dev_info, 100, LV_STATE_DEFAULT);

  voltage_in = lv_label_create(contanier1);
  lv_obj_set_size(voltage_in, 52, 14);
  lv_label_set_text_fmt(voltage_in, "V: ");
  lv_obj_align_to(voltage_in, dev_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 1);
  // lv_obj_set_style_bg_color(voltage_in, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(voltage_in, 100, LV_STATE_DEFAULT);

  current_in = lv_label_create(contanier1);
  lv_obj_set_size(current_in, 52, 14);
  lv_label_set_text_fmt(current_in, "I: ");
  lv_obj_align_to(current_in, voltage_in, LV_ALIGN_OUT_RIGHT_TOP, 1, 0);
  // lv_obj_set_style_bg_color(current_in, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(current_in, 100, LV_STATE_DEFAULT);

  power_in = lv_label_create(contanier1);
  lv_obj_set_size(power_in, 52, 14);
  lv_label_set_text_fmt(power_in, "P: ");
  lv_obj_align_to(power_in, current_in, LV_ALIGN_OUT_RIGHT_TOP, 1, 0);
  // lv_obj_set_style_bg_color(power_in, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(power_in, 100, LV_STATE_DEFAULT);

  voltage_out = lv_label_create(contanier1);
  lv_obj_set_size(voltage_out, 52, 14);
  lv_label_set_text_fmt(voltage_out, "V: ");
  lv_obj_align_to(voltage_out, voltage_in, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 1);
  // lv_obj_set_style_bg_color(voltage_out, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(voltage_out, 100, LV_STATE_DEFAULT);

  current_out = lv_label_create(contanier1);
  lv_obj_set_size(current_out, 52, 14);
  lv_label_set_text_fmt(current_out, "I: ");
  lv_obj_align_to(current_out, voltage_out, LV_ALIGN_OUT_RIGHT_TOP, 1, 0);
  // lv_obj_set_style_bg_color(current_out, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(current_out, 100, LV_STATE_DEFAULT);

  power_out = lv_label_create(contanier1);
  lv_obj_set_size(power_out, 52, 14);
  lv_label_set_text_fmt(power_out, "P: ");
  lv_obj_align_to(power_out, current_out, LV_ALIGN_OUT_RIGHT_TOP, 1, 0);
  // lv_obj_set_style_bg_color(power_out, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(power_out, 100, LV_STATE_DEFAULT);

  efficiency = lv_label_create(contanier1);
  lv_obj_set_size(efficiency, 158, 14);
  lv_label_set_text_fmt(efficiency, "EFF: ");
  lv_obj_align_to(efficiency, voltage_out, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 1);
  // lv_obj_set_style_bg_color(efficiency, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(efficiency, 100, LV_STATE_DEFAULT);

  temperature_1 = lv_label_create(contanier1);
  lv_obj_set_size(temperature_1, 52, 14);
  lv_label_set_text_fmt(temperature_1, "T: ");
  lv_obj_align_to(temperature_1, efficiency, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 1);
  // lv_obj_set_style_bg_color(temperature_1, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(temperature_1, 100, LV_STATE_DEFAULT);

  fan_1 = lv_label_create(contanier1);
  lv_obj_set_size(fan_1, 105, 14);
  lv_label_set_text_fmt(fan_1, "FAN: ");
  lv_obj_align_to(fan_1, temperature_1, LV_ALIGN_OUT_RIGHT_TOP, 1, 0);
  // lv_obj_set_style_bg_color(fan_1, lv_color_hex(0xff00000), LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_opa(fan_1, 100, LV_STATE_DEFAULT);
}
void my_disp_init(void) {
// TFT驱动初始化
//   tft.begin(); /* TFT init */
//   tft.initDMA();
  tft.init();
  // tft.initDMA();
  tft.setRotation(3); /* Landscape orientation, flipped */

  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t buf_2_1[TFT_HEIGHT * 80]; /*A buffer for 10 rows*/
  // static lv_color_t buf_2_2[TFT_HEIGHT * 80]; /*An other buffer for 10rows*/
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf_2_1, NULL,
                      TFT_HEIGHT * 80); /*Initialize
                      the display buffer*/

  // 设置LVGL显示设备
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = TFT_HEIGHT;
  disp_drv.ver_res = TFT_WIDTH;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

