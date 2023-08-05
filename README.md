# Pmbus monitor
说明：\
这个库是用esp32+lcd做的一个pmbus监控，主要参考了linux源码中的``drivers/hwmon/pmbus/pmbus.h``,``drivers/hwmon/pmbus/pmbus.c``,``drivers/hwmon/pmbus/pmbus_core.c``  
效果可以参考``https://www.bilibili.com/video/BV1s8411D7dM/``

硬件：
```
电源：DPS-400AB-23
开发板：合宙EPS32C3 + LCD扩展板
```
开发平台：
```
vscode + platformIO + arduino
```
依赖库：
```
lib_deps = 
	bodmer/TFT_eSPI@^2.5.31
	lvgl/lvgl@^8.3.7
```

使用方法：  
IO0:sda  
IO1:scl  
接5V电源，开机后会自动搜索pmbus设备，长按LCD扩展板上的按钮可以清空已搜索到的设备并重新搜索  
目前的代码仅支持地址为``0x58``的``DPS-400AB-23 ``，如果要增加支持电源型号需要修改以下代码  
pmbus.cpp 415行
```C
/*
 * Use driver_data to set the number of pages supported by the chip.
 */
static const struct i2c_device_id pmbus_id[] = {
	{"DPS-400AB-23 ", 1},
	{}
};
```
main.cpp 17行
```C
uint8_t addresslist[1] = {0x58};
```