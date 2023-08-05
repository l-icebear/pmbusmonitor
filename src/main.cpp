#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "common.h"
#include "i2c.h"
#include "pmbus.h"
#include "lvgl_ports.h"
#include "key.h"

//lvgl benchmark
// #include <demos/lv_demos.h>>
// #include <examples/lv_examples.h>>

// TFT_eSPI tft = TFT_eSPI();

// pmbus_dev_list_head pmbus_dev_head;
uint8_t addresslist[1] = {0x58};
i2c_client_list_head pmbus_dev_list_head;

void rescan(void){
  struct i2c_client *pmbusdev = NULL;
  struct pmbus_data *pmbusdevdata = NULL;

  if(!LIST_EMPTY(&pmbus_dev_list_head)){
    LIST_FOREACH(pmbusdev, &pmbus_dev_list_head, list_entry){
      LIST_REMOVE(pmbusdev, list_entry);
    }
  }

  lv_obj_t * mbox = lv_msgbox_create(contanier1, NULL, "scanning ...", NULL, NULL);
  lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);

  pmbus_scan(&Wire, addresslist, sizeof(addresslist)/sizeof(addresslist[0]), &pmbus_dev_list_head);
  delay(3000);
  
  lv_msgbox_close(mbox);
}

struct one_key key_center{
  .pin = 4,
  .state = 0,
  .long_press_call_back = rescan, 
};

void setup() {
  // put your setup code here, to run once:
  //Wire.begin(27,26,100000);//esp32
  Wire.begin(0,1,100000);//c3
  DebugSerial.begin(115200);

  my_disp_init();
  /*获取LVGL版本信息*/
  String LVGL_Arduino = "Hello LVGL! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch(); //版本
  // lv_obj_t *label = lv_label_create(lv_scr_act());
  // lv_label_set_text(label, LVGL_Arduino.c_str());
  // lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); //居中显示

  //lvgl benchmark demo
  // lv_demo_benchmark();
  lvgl_ui_init();
  
  logdebugln("\r\npmbusmonitor " + String(__DATE__) + "\r\n");
  logdebugln("I2C scanning ......");

  byte error;
  byte address;
  char message[128];

  LIST_INIT(&pmbus_dev_list_head);
  
  pmbus_scan(&Wire, addresslist, sizeof(addresslist)/sizeof(addresslist[0]), &pmbus_dev_list_head);

  key_init(&key_center);
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_timer_handler(); /* let the GUI do its work */
  static bool on = false;

  if(!on){
    //lv_obj_add_flag(contanier1, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_clear_flag(contanier2, LV_OBJ_FLAG_HIDDEN);
    delay(3000);
    lv_obj_add_flag(contanier2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(contanier1, LV_OBJ_FLAG_HIDDEN);
    on = true;
  }

  struct i2c_client *pmbusdev = NULL;
  struct pmbus_data *pmbusdevdata = NULL;
  unsigned long last_update_time = 0;
  if(!LIST_EMPTY(&pmbus_dev_list_head)){
    LIST_FOREACH(pmbusdev, &pmbus_dev_list_head, list_entry){
      pmbusdevdata = (pmbus_data *) pmbusdev->data;
      pmbusdevdata->update_sensor(pmbusdev, pmbusdevdata);
    }
    pmbusdev = pmbus_dev_list_head.lh_first,
    pmbusdevdata = (pmbus_data *)pmbusdev->data;
  }
  
  if(pmbusdev != NULL && pmbusdevdata != NULL){
    if(last_update_time != pmbusdevdata->last_updated){
      lv_label_set_text_fmt(dev_info, "0x%2x:%s", pmbusdev->addr, pmbusdev->name);
      lv_label_set_text_fmt(voltage_in, "V: %d", (int)pmbus_linear2float(pmbusdevdata->sensor[pmbusdevdata->currpage][PSC_VOLTAGE_IN]));
      lv_label_set_text_fmt(current_in, "I: %0.3f", pmbus_linear2float(pmbusdevdata->sensor[pmbusdevdata->currpage][PSC_CURRENT_IN]));
      lv_label_set_text_fmt(power_in, "P: %0.1f", pmbus_linear2float(pmbusdevdata->sensor[pmbusdevdata->currpage][PSC_POWER_IN]));
      lv_label_set_text_fmt(temperature_1, "T: %0.1f", pmbus_linear2float(pmbusdevdata->sensor[pmbusdevdata->currpage][PSC_TEMPERATURE]));
      lv_label_set_text_fmt(fan_1, "FAN: %d", (int)pmbus_linear2float(pmbusdevdata->sensor[pmbusdevdata->currpage][PSC_FAN12]));
      last_update_time = pmbusdevdata->last_updated;
    }
  }
  
  delay(100);
}
