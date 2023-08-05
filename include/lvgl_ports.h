/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-07 11:08:33
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 11:15:39
 * FilePath: \pmbusmonitor\include\lvgl_ports.h
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#ifndef LVGL_PORTS_H
#define LVGL_PORTS_H
#include <TFT_eSPI.h>
#include <lvgl.h>

// /*Change to your screen resolution*/
// const uint16_t screenWidth = 80;
// const uint16_t screenHeight = 160;
extern lv_obj_t *contanier1;
extern lv_obj_t *contanier2;
extern lv_obj_t *dev_info;
extern lv_obj_t *voltage_in;
extern lv_obj_t *current_in;
extern lv_obj_t *power_in;
extern lv_obj_t *voltage_out;
extern lv_obj_t *current_out;
extern lv_obj_t *power_out;
extern lv_obj_t *fan_1;
extern lv_obj_t *temperature_1;
extern lv_obj_t *efficiency;

void my_disp_init(void); // 挂载lvgl接口，设置buffer
void lvgl_ui_init(void);
#endif
