/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-03 14:23:42
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 09:48:50
 * FilePath: \pmbusmonitor\include\i2c.h
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#ifndef I2C_H
#define I2C_H

#include <Arduino.h>
#include <Wire.h>

#define I2C_NAME_SIZE 20

struct i2c_device_id {
	char name[I2C_NAME_SIZE];
	uint32_t driver_data	/* Data private to the driver */
			__attribute__((aligned(sizeof(uint32_t))));
};

struct i2c_client{
    uint8_t flag;
    uint8_t addr;
    char name[I2C_NAME_SIZE];
    TwoWire *I2C;
    void *data;
    //struct i2c_driver *driver;
    //LIST_HEAD detected;
	LIST_ENTRY(i2c_client) list_entry;
};

typedef LIST_HEAD(i2c_client_list_head, i2c_client) i2c_client_list_head;

extern int i2c_check_address(TwoWire *I2C, uint8_t address);
extern void i2c_write_byte_data(TwoWire *I2C, uint8_t address,uint8_t reg, uint8_t value);
extern void i2c_write_byte(TwoWire *I2C, uint8_t address,uint8_t data);
extern void i2c_write_word_data(TwoWire *I2C, uint8_t address,uint8_t reg, uint16_t value);
extern int16_t i2c_read_byte_data(TwoWire *I2C, uint8_t address, uint8_t reg);
extern int32_t i2c_read_word_data(TwoWire *I2C, uint8_t address, uint8_t reg);
extern int i2c_read_string(TwoWire *I2C, uint8_t address, uint8_t reg,int bytes,uint8_t *buffer);
#endif
