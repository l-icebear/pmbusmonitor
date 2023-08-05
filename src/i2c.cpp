/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-03 14:22:12
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 09:48:39
 * FilePath: \pmbusmonitor\src\i2c.cpp
 * Copyright (C) 2023 lvwei. All rights reserved.
 */
#include <Arduino.h>
#include <Wire.h>
#include "i2c.h"

int i2c_check_address(TwoWire *I2C, uint8_t address){
  int error = 0;
  I2C->beginTransmission(address);
  error = I2C->endTransmission();
  I2C->flush();
  return error;
}

void i2c_write_byte(TwoWire *I2C, uint8_t address, uint8_t data) {

  I2C->beginTransmission(address);
  I2C->write(data);
  I2C->endTransmission(true);

  return;
}

void i2c_write_byte_data(TwoWire *I2C, uint8_t address, uint8_t reg, uint8_t value) {

  I2C->beginTransmission(address);
  I2C->write(reg);
  I2C->write(value);
  I2C->endTransmission(true);

  return;
}

void i2c_write_word_data(TwoWire *I2C, uint8_t address, uint8_t reg, uint16_t value) {

  I2C->beginTransmission(address);
  I2C->write(reg);
  I2C->write(0xFF & value);
  I2C->write(value>>8);
  I2C->endTransmission(true);

  return;
}
/*
 *
 */

int16_t i2c_read_byte_data(TwoWire *I2C, uint8_t address, uint8_t reg) {

  I2C->beginTransmission(address);
  I2C->write(reg);
  I2C->endTransmission(false);
  
  I2C->requestFrom(address,(uint8_t) 1,(uint8_t) true);

  return I2C->read();
}

/*
 *
 */

int32_t i2c_read_word_data(TwoWire *I2C, uint8_t address, uint8_t reg) {

  int temp = -1;
  int32_t value = -1;

  I2C->beginTransmission(address);
  I2C->write(reg);
  I2C->endTransmission(false);
  
  I2C->requestFrom(address,(uint8_t) 2,(uint8_t) true);

  value = I2C->read();
  temp = I2C->read();
  if(temp == -1 || value == -1)
    return -1;
  value |= temp << 8;

  return value;
}

/*
 *
 */

static void read_block(TwoWire *I2C, uint8_t address, uint8_t reg,int bytes, uint8_t *buffer) {

  int i;

  I2C->beginTransmission(address);
  I2C->write(reg);
  I2C->endTransmission(false);
  
  I2C->requestFrom(address,(uint8_t) bytes,(uint8_t) true);

  for (i = 0; i < bytes; ++i) {

    buffer[i] = I2C->read();
  }

  return;
}

/*
 *
 */

int i2c_read_string(TwoWire *I2C, uint8_t address, uint8_t reg,int bytes,uint8_t *buffer) {

  int len = 0;

  len = (int) i2c_read_byte_data(I2C, address, reg);

  if ((len)&&(len < bytes)) {

    read_block(I2C, address, reg, len, buffer);
  }

  return len;
}

void* i2c_get_clientdata(const struct i2c_client *client){
  return client->data;
}

void i2c_set_clientdata(struct i2c_client *client, void *data){
  client->data = data;
}