/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-07 11:10:31
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 18:09:40
 * FilePath: \pmbusmonitor\src\key.cpp
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#include "key.h"
#include "common.h"

#define KEY_CENTER 4

void key_scan(struct one_key* key){
    uint32_t io_num;
    bool ret = 0;
    int key_value = digitalRead(key->pin);
    static int key_value_last = 1;
    static int key_press_time = 0;

    switch(key->state){
        case 0:
            if(key_value == 0){
                if(key_value_last){
                    key_value_last = key_value;
                    key_press_time = esp_timer_get_time();
                    key->state = 1;
                }
            }else{
                if(key_value_last == 0){
                    key->state = 3;
                }
                key_value_last = key_value;
            }
            break;
        case 1:
            logdebugln("press");
            if(key_value == 0){
                key->state = 2;
            }else{
                key_value_last = key_value;
                key->state = 3;
            }
            break;
        case 2:
            //logdebugln("long press");
            if(key_value == 0){
                int delta = esp_timer_get_time() - key_press_time;
                if(delta > 3000000){
                    key->long_press_call_back();
                    logdebugln("long press");
                    if(key_value){
                        key_value_last = key_value;
                        key->state = 3;
                    }else{
                        key->state = 0;
                    }
                }
            }else{
                key_value_last = key_value;
                key->state = 3;
            }
            break;
        case 3:
            logdebugln("release");
            key->state = 0;
            break;
    }
}

void gpio_key_task(void *arg){
    struct one_key *key = (struct one_key*) arg; 
    while(1){
        vTaskDelay(1);
        key_scan(key);
    }
}

void key_init(struct one_key* key){
    pinMode(key->pin, INPUT_PULLUP);
    xTaskCreate(gpio_key_task, "key_tesk", 2048, (void *)key, 1, NULL);
}
