/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-05 09:29:43
 * LastEditors: lvwei
 * LastEditTime: 2023-07-05 10:24:08
 * FilePath: \pmbusmonitor\include\common.h
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#ifndef COMMON_H
#define COMMON_H
#include <arduino.h>

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({              \         
const typeof( ((type *)0)->member ) *__mptr = (ptr);    \         
(type *)( (char *)__mptr - offsetof(type,member) );})

#define DEBUG

#define DebugSerial    Serial

#ifdef  DEBUG
#define logdebug(message, ...)   DebugSerial.print(message, ##__VA_ARGS__)
#define logdebugln(message, ...)   DebugSerial.println(message, ##__VA_ARGS__)
#else
#define logdebug(message, ...)
#define logdebugln(message, ...)
#endif
#endif