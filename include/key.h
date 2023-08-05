/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-07 11:10:31
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 18:09:40
 * FilePath: \pmbusmonitor\include\key.h
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#ifndef KEY_H
#define KEY_H

#include <arduino.h>

struct one_key{
    uint8_t pin;
    uint8_t state;
    void (*long_press_call_back)();
};

void key_init(struct one_key* key);
// void key_scan(void);
#endif