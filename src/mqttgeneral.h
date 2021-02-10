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
#ifndef __MQTTGENERAL_H__
#define __MQTTGENERAL_H__
#include "mqtt.h"

typedef enum _mqttgeneral_state_t{
    MQTT_BASESTATE(GENERAL),
    MQTT_GENERAL_STATE_END
}mqttgeneral_state_t;

typedef struct _mqttgeneral_t{
    MQTT_BASEATTRIBUTES
}mqttgeneral_t;


extern mqtt_t *mqttgeneral_new(void);
extern int mqttgeneral_init(struct event_base *base, mqtt_t *mqttptr, config_setting_t *csmqtt);
extern int mqttgeneral_free(mqtt_t *mqttptr);
#endif //__MQTTGENERAL_H__
