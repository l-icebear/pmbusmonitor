/*
 * Description: 
 * Version: 1.0
 * Author: lvwei
 * Date: 2023-07-07 11:10:31
 * LastEditors: lvwei
 * LastEditTime: 2023-07-07 18:09:40
 * FilePath: \pmbusmonitor\src\pmbus.cpp
 * Copyright (C) 2023 lvwei. All rights reserved.
 */

#include <Arduino.h>
#include <Wire.h>
#include "common.h"
#include "pmbus.h"
#include "i2c.h"

int pmbus_set_page(struct i2c_client *client, uint8_t page)
{
	// struct pmbus_data *data = (pmbus_data *) i2c_get_clientdata(client);
	struct pmbus_data *data = (pmbus_data *) client->data;
	int rv = 0;
	uint8_t newpage;

	if (page != data->currpage) {
		i2c_write_byte_data(client->I2C, client->addr, PMBUS_PAGE, page);
		newpage = i2c_read_byte_data(client->I2C, client->addr, PMBUS_PAGE);
		if (newpage != page)
			rv = -EIO;
		else
			data->currpage = page;
	}
    // logdebug("pmbus_set_page 0x");
	// logdebugln(newpage, HEX);
	// logdebugln(rv, DEC);
	return rv;
}

int16_t pmbus_read_byte_data(struct i2c_client *client, uint8_t page, uint8_t reg){
	int16_t rv;

	if (page >= 0) {
		rv = pmbus_set_page(client, page);
		if (rv < 0)
			return rv;
	}

	return i2c_read_byte_data(client->I2C, client->addr, reg);
}

int32_t pmbus_read_word_data(struct i2c_client *client, uint8_t page, uint8_t reg)
{
	int32_t rv;

	rv = pmbus_set_page(client, page);

    // logdebug("pmbus_read_word_data 0x");
	// logdebugln(reg, HEX);

	if (rv < 0)
		return rv;

	return i2c_read_word_data(client->I2C, client->addr, reg);
}

int pmbus_write_byte(struct i2c_client *client, uint8_t page, uint8_t value)
{
	int rv;

	if (page >= 0) {
		rv = pmbus_set_page(client, page);

        // printk(KERN_DEBUG "pmbus_write_byte %d!\n", rv);

		if (rv < 0)
			return rv;
	}

    // printk(KERN_DEBUG "pmbus_write_byte2-- %d!\n", value);
	//do not support single-byte commands,
	i2c_write_byte(client->I2C, client->addr, value);
	return 0;
}

int pmbus_write_word_data(struct i2c_client *client, uint8_t page, uint8_t reg, uint16_t word)
{
	int rv;

	rv = pmbus_set_page(client, page);

    // printk(KERN_DEBUG "pmbus_write_word_data %d %x %x!\n", rv, reg, word);

	if (rv < 0)
		return rv;

	i2c_write_word_data(client->I2C, client->addr, reg, word);
	return 0;
}

/*
 * _pmbus_read_byte_data() is similar to pmbus_read_byte_data(), but checks if
 * a device specific mapping function exists and calls it if necessary.
 */
static int32_t _pmbus_read_byte_data(struct i2c_client *client, int page, int reg)
{
	struct pmbus_data *data = (pmbus_data *) client->data;
	// const struct pmbus_driver_info *info = data->info;
	int32_t status;

	if (data->read_byte_data) {
		status = data->read_byte_data(client, page, reg);
		if (status != -1)
			return status;
	}
	return pmbus_read_byte_data(client, page, reg);
}

/*
 * _pmbus_read_word_data() is similar to pmbus_read_word_data(), but checks if
 * a device specific mapping function exists and calls it if necessary.
 */
static int32_t _pmbus_read_word_data(struct i2c_client *client, int page, int reg)
{
	struct pmbus_data *data = (pmbus_data *) client->data;
	//const struct pmbus_driver_info *info = data->info;
	int32_t status;

	if (data->read_word_data) {
		status = data->read_word_data(client, page, reg);

        //printk(KERN_DEBUG "_pmbus_read_word_data %d %x!\n", status, reg);

		if (status != -1)
			return status;
	}
	if (reg >= PMBUS_VIRT_BASE)
		return -ENXIO;
	return pmbus_read_word_data(client, page, reg);
}

/*
 * _pmbus_write_byte() is similar to pmbus_write_byte(), but checks if
 * a device specific mapping function exists and calls it if necessary.
 */
static int _pmbus_write_byte(struct i2c_client *client, int page, uint8_t value)
{
	struct pmbus_data *data = (pmbus_data *) client->data;
	//const struct pmbus_driver_info *info = data->info;
	int status;

	if (data->write_byte) {
		status = data->write_byte(client, page, value);

        //printk(KERN_DEBUG "_pmbus_write_byte %d!\n", status);

		if (status != -ENODATA)
			return status;
	}
	return pmbus_write_byte(client, page, value);
}

static int pmbus_check_status_cml(struct i2c_client *client)
{
	struct pmbus_data *data = (pmbus_data *) client->data;
	int status, status2;

	status = _pmbus_read_byte_data(client, -1, data->status_register);
	if (status < 0 || (status & PB_STATUS_CML)) {
		status2 = _pmbus_read_byte_data(client, -1, PMBUS_STATUS_CML);
		if (status2 < 0 || (status2 & PB_CML_FAULT_INVALID_COMMAND))
			return -EIO;
	}
	return 0;
}

static void pmbus_clear_fault_page(struct i2c_client *client, int page)
{
	_pmbus_write_byte(client, page, PMBUS_CLEAR_FAULTS);
}

static bool pmbus_check_register(struct i2c_client *client,
				 int32_t (*func)(struct i2c_client *client,
					     int page, int reg),
				 int page, int reg)
{
	int rv;
	struct pmbus_data *data = (pmbus_data *) client->data;

	rv = func(client, page, reg);
	if (rv >= 0 && !(data->flags & PMBUS_SKIP_STATUS_CHECK))
		rv = pmbus_check_status_cml(client);
	//pmbus_clear_fault_page(client, -1);
	return rv >= 0;
}

bool pmbus_check_byte_register(struct i2c_client *client, int page, int reg)
{
	return pmbus_check_register(client, _pmbus_read_byte_data, page, reg);
}

bool pmbus_check_word_register(struct i2c_client *client, int page, int reg)
{
	return pmbus_check_register(client, _pmbus_read_word_data, page, reg);
}

void pmbus_update_sensor(struct i2c_client *client, struct pmbus_data *data){
	int page;
	unsigned long time;

	time = millis();
	if(time > (data->last_updated + 1000)){
		for(page = 0; page < data->pages; page++){
			if(data->func[page] & PMBUS_HAVE_VIN)
				data->sensor[page][PSC_VOLTAGE_IN] = data->read_word_data(client, page, PMBUS_READ_VIN);
			if(data->func[page] & PMBUS_HAVE_VCAP)
				data->sensor[page][PSC_VOLTAGE_CAP] = data->read_word_data(client, page, PMBUS_READ_VCAP);
			if(data->func[page] & PMBUS_HAVE_VOUT)
				data->sensor[page][PSC_VOLTAGE_OUT] = data->read_word_data(client, page, PMBUS_READ_VOUT);
			if(data->func[page] & PMBUS_HAVE_IIN)
				data->sensor[page][PSC_CURRENT_IN] = data->read_word_data(client, page, PMBUS_READ_IIN);
			if(data->func[page] & PMBUS_HAVE_IOUT)
				data->sensor[page][PSC_CURRENT_OUT] = data->read_word_data(client, page, PMBUS_READ_IOUT);
			if(data->func[page] & PMBUS_HAVE_PIN)
				data->sensor[page][PSC_POWER_IN] = data->read_word_data(client, page, PMBUS_READ_PIN);
			if(data->func[page] & PMBUS_HAVE_POUT)
				data->sensor[page][PSC_POWER_OUT] = data->read_word_data(client, page, PMBUS_READ_POUT);
			if(data->func[page] & PMBUS_HAVE_FAN12)
				data->sensor[page][PSC_FAN12] = data->read_word_data(client, page, PMBUS_READ_FAN_SPEED_1);
			if(data->func[page] & PMBUS_HAVE_FAN34)
				data->sensor[page][PSC_FAN34] = data->read_word_data(client, page, PMBUS_READ_FAN_SPEED_3);
			if(data->func[page] & PMBUS_HAVE_TEMP)
				data->sensor[page][PSC_TEMPERATURE] = data->read_word_data(client, page, PMBUS_READ_TEMPERATURE_1);
			if(data->func[page] & PMBUS_HAVE_TEMP2)
				data->sensor[page][PSC_TEMPERATURE2] = data->read_word_data(client, page, PMBUS_READ_TEMPERATURE_2);
			if(data->func[page] & PMBUS_HAVE_TEMP3)
				data->sensor[page][PSC_TEMPERATURE3] = data->read_word_data(client, page, PMBUS_READ_TEMPERATURE_3);
		}
		data->last_updated = millis();
	}
}
/*
 * Find sensor groups and status registers on each page.
 */
static void pmbus_find_sensor_groups(struct i2c_client *client,
				     struct pmbus_data *info)
{
	int page;

	logdebug("pmbus have flag0 ");
	logdebugln(info->func[page], HEX);

	/* Sensors detected on page 0 only */
	if (pmbus_check_word_register(client, 0, PMBUS_READ_VIN))
		info->func[0] |= PMBUS_HAVE_VIN;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_VCAP))
		info->func[0] |= PMBUS_HAVE_VCAP;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_IIN))
		info->func[0] |= PMBUS_HAVE_IIN;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_PIN))
		info->func[0] |= PMBUS_HAVE_PIN;
	if (info->func[0]
	    && pmbus_check_byte_register(client, 0, PMBUS_STATUS_INPUT))
		info->func[0] |= PMBUS_HAVE_STATUS_INPUT;
	if (pmbus_check_byte_register(client, 0, PMBUS_FAN_CONFIG_12) &&
	    pmbus_check_word_register(client, 0, PMBUS_READ_FAN_SPEED_1)) {
		info->func[0] |= PMBUS_HAVE_FAN12;
		if (pmbus_check_byte_register(client, 0, PMBUS_STATUS_FAN_12))
			info->func[0] |= PMBUS_HAVE_STATUS_FAN12;
	}
	if (pmbus_check_byte_register(client, 0, PMBUS_FAN_CONFIG_34) &&
	    pmbus_check_word_register(client, 0, PMBUS_READ_FAN_SPEED_3)) {
		info->func[0] |= PMBUS_HAVE_FAN34;
		if (pmbus_check_byte_register(client, 0, PMBUS_STATUS_FAN_34))
			info->func[0] |= PMBUS_HAVE_STATUS_FAN34;
	}
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_1))
		info->func[0] |= PMBUS_HAVE_TEMP;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_2))
		info->func[0] |= PMBUS_HAVE_TEMP2;
	if (pmbus_check_word_register(client, 0, PMBUS_READ_TEMPERATURE_3))
		info->func[0] |= PMBUS_HAVE_TEMP3;
	if (info->func[0] & (PMBUS_HAVE_TEMP | PMBUS_HAVE_TEMP2
			     | PMBUS_HAVE_TEMP3)
	    && pmbus_check_byte_register(client, 0,
					 PMBUS_STATUS_TEMPERATURE))
			info->func[0] |= PMBUS_HAVE_STATUS_TEMP;

	/* Sensors detected on all pages */
	for (page = 0; page < info->pages; page++) {
		if (pmbus_check_word_register(client, page, PMBUS_READ_VOUT)) {
			info->func[page] |= PMBUS_HAVE_VOUT;
			if (pmbus_check_byte_register(client, page,
						      PMBUS_STATUS_VOUT))
				info->func[page] |= PMBUS_HAVE_STATUS_VOUT;
		}
		if (pmbus_check_word_register(client, page, PMBUS_READ_IOUT)) {
			info->func[page] |= PMBUS_HAVE_IOUT;
			if (pmbus_check_byte_register(client, 0,
						      PMBUS_STATUS_IOUT))
				info->func[page] |= PMBUS_HAVE_STATUS_IOUT;
		}
		if (pmbus_check_word_register(client, page, PMBUS_READ_POUT))
			info->func[page] |= PMBUS_HAVE_POUT;
		
		logdebug("pmbus have flag ");
		logdebugln(info->func[page], HEX);
	}
}

/*
 * Identify chip parameters.
 */
static int pmbus_identify(struct i2c_client *client,
			  struct pmbus_data *info)
{
	int ret = 0;

	if (!info->pages) {
		/*
		 * Check if the PAGE command is supported. If it is,
		 * keep setting the page number until it fails or until the
		 * maximum number of pages has been reached. Assume that
		 * this is the number of pages supported by the chip.
		 */
		if (pmbus_check_byte_register(client, 0, PMBUS_PAGE)) {
			uint8_t page;

			for (page = 1; page < PMBUS_PAGES; page++) {
				if (pmbus_set_page(client, page) < 0)
					break;
			}
			pmbus_set_page(client, 0);
			info->pages = page;
		} else {
			info->pages = 1;
		}
	}

 	if (pmbus_check_byte_register(client, 0, PMBUS_VOUT_MODE)) {
		int vout_mode;

		vout_mode = pmbus_read_byte_data(client, 0, PMBUS_VOUT_MODE);
		// logdebug("vout_mode 0x");
		// logdebugln(vout_mode, HEX);
		if (vout_mode >= 0 && vout_mode != 0xff) {
			switch (vout_mode >> 5) {
			case 0:
				break;
			case 1:
				info->format[PSC_VOLTAGE_OUT] = vid;
				break;
			case 2:
				info->format[PSC_VOLTAGE_OUT] = direct;
				break;
			default:
				ret = -ENODEV;
				goto abort;
			}
		}
		// logdebugln(info->format[PSC_VOLTAGE_OUT], HEX);
 	}

	/*
	 * We should check if the COEFFICIENTS register is supported.
	 * If it is, and the chip is configured for direct mode, we can read
	 * the coefficients from the chip, one set per group of sensor
	 * registers.
	 *
	 * To do this, we will need access to a chip which actually supports the
	 * COEFFICIENTS command, since the command is too complex to implement
	 * without testing it. Until then, abort if a chip configured for direct
	 * mode was detected.
	 */
	if (info->format[PSC_VOLTAGE_OUT] == direct) {
		ret = -ENODEV;
		goto abort;
	}

	/* Try to find sensor groups  */
	pmbus_find_sensor_groups(client, info);
abort:
 	return ret;
}

static int pmbus_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	struct pmbus_data *data;

	//data =(pmbus_data*) malloc(sizeof(pmbus_data));
	data = (pmbus_data*) calloc(1, sizeof(pmbus_data));
	if (!data)
		return -ENOMEM;

    logdebugln("pmbus_probe!");

	data->flags = PMBUS_SKIP_STATUS_CHECK;
	data->pages = id->driver_data;
	data->identify = pmbus_identify;
	data->read_byte_data = pmbus_read_byte_data;
	data->read_word_data = pmbus_read_word_data;
	data->write_byte = pmbus_write_byte;
	data->write_word_data = pmbus_write_word_data;
	data->update_sensor = pmbus_update_sensor;
	data->currpage = 0;
	client->data = data;

	// LIST_INSERT_HEAD(&pmbus_dev_head, data, list_entry);
	//data->identify(client, data);
	pmbus_identify(client, data);

	//return pmbus_do_probe(client, id, data);
}

/*
 * Use driver_data to set the number of pages supported by the chip.
 */
static const struct i2c_device_id pmbus_id[] = {
	{"DPS-400AB-23 ", 1},
	{}
};

bool pmbus_detect(TwoWire *wire, uint8_t address, i2c_client_list_head *head){
	char mfr_model[21] = {0};
	uint8_t i = 0;

	i2c_read_string(wire, address, 0x9a, sizeof(mfr_model) - 1, (uint8_t *) mfr_model);
	// logdebugln(&mfr_model[1]);
	// logdebugln(sizeof(mfr_model));
	// logdebugln(sizeof(pmbus_id[i].name));
	// logdebugln(pmbus_id[i].name);
	while(pmbus_id[i].name[0]){
		if(strcmp(&mfr_model[1], pmbus_id[i].name) == 0){
			logdebugln("found pmbus device : " + String(&mfr_model[1]));
			struct i2c_client *client = (i2c_client *) malloc(sizeof(i2c_client));
			client->addr = address;
			client->I2C = wire;
			memcpy(client->name, &mfr_model[1], (sizeof(mfr_model) - 1));
			LIST_INSERT_HEAD(head, client, list_entry);
			pmbus_probe(client, &pmbus_id[i]);
		}
		i++;
	}
	return 0;
}

//void pmbus_scan(TwoWire *wire, uint8_t *address, uint8_t len, i2c_client_list_head *head, i2c_client *clients){
void pmbus_scan(TwoWire *wire, uint8_t *address, uint8_t len, i2c_client_list_head *head){
	uint8_t i = 0;
	int error;

	// uint8_t add = 0;
	// for(add = 1; add < 127; add++){
	// 	error = i2c_check_address(wire, address[i]);

	// 	if(error == 0){
	// 		logdebug("found i2c device at 0x");
	// 		logdebugln(add, HEX);
	// 	} else if(error == 4){
	// 		logdebug("Unknow error at 0x");
	// 		logdebugln(add, HEX);
	// 	}
	// }

	for(i = 0; i < len; i++){
		if(address[i] < 128){
			error = i2c_check_address(wire, address[i]);

			if(error == 0){
				logdebug("found i2c device at 0x");
				logdebugln(address[i], HEX);
				pmbus_detect(wire, address[i], head);
			} else if(error == 4){
				logdebug("Unknow error at 0x");
				logdebugln(address[i], HEX);
			}
		}
  	}
}

float pmbus_linear2float(uint16_t data){
	float val = 0;
	int8_t exponent;
	int16_t mantissa;

	exponent = data >> 11;
	// mantissa = ((int16_t)((data & 0x07ff) << 5)) >> 5;
	mantissa = data & 0x07ff;

	if(exponent > 0x0f) {
		exponent |= 0xe0;
	}
	if(mantissa > 0x03ff) {
		mantissa |= 0xf800;
	}

	val = mantissa;
	if(exponent >=0){
		val *= (1 << exponent);
	}else{
		val /= (1 << -exponent);
	}
	
	return (float)val;
}
