/*******************************************************************************
MIT License

Copyright (c) 2021 Zhihong Li <humble_zh@163.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#ifndef __MQTTALI_H__
#define __MQTTALI_H__
#include "mqtt.h"

typedef enum _mqttali_state_t{
    MQTT_BASESTATE(ALI),
    MQTTALI_STATE_END
}mqttali_state_t;

typedef struct _mqttali_t{
    MQTT_BASEATTRIBUTES
    const char *regionid;
    const char *deviceid;
    const char *productkey;
    const char *devicename;
    const char *devicesecret;
    int32_t mode;
    const char *signmethod;
}mqttali_t;


extern mqtt_t *mqttali_new(void);
extern int mqttali_init(struct event_base *base, mqtt_t *mqttptr, config_setting_t *csmqtt);
extern int mqttali_free(mqtt_t *mqttptr);
#endif //__MQTTALI_H__
