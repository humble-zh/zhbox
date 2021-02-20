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
#ifndef __N_GENERAL_H__
#define __N_GENERAL_H__
#include "mqtt.h"

/*******************************************************************************
*                        General Direct Device by MQTT                        *
*******************************************************************************/
typedef enum _generaldirectdevmqtt_state_t
{
    MQTT_BASESTATE(GENERALDIRECTDEV),
    GENERALDIRECTDEVMQTT_STATE_END
}generaldirectdevmqtt_state_t;

#define GENERALDIRECTDEVMQTT_BASEATTRIBUTES \
    MQTT_BASEATTRIBUTES\

#define GENERALDIRTECTDEVMQTTGETBASESTRINGVAR(this, key, keylen, dptr) do {\
    MQTTGETBASESTRINGVAR(this, key, keylen, dptr);\
} while (0)

typedef struct _generaldirectdevmqtt_t
{
    GENERALDIRECTDEVMQTT_BASEATTRIBUTES
}generaldirectdevmqtt_t;

extern void *generaldirectdevmqtt_new(void);
extern int generaldirectdevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs);
extern int generaldirectdevmqtt_free(void *vobj);



/*******************************************************************************
*                       General Gateway Device by MQTT                        *
*******************************************************************************/
typedef enum _generalgatewaydevmqtt_state_t
{
    MQTT_BASESTATE(GENERALGATEWAYDEV),
    GENERALGATEWAYDEVMQTT_STATE_END
}generalgatewaydevmqtt_state_t;

#define GENERALGATEWAYDEVMQTT_BASEATTRIBUTES \
    MQTT_BASEATTRIBUTES\

typedef struct _generalgatewaydevmqtt_t
{
    GENERALGATEWAYDEVMQTT_BASEATTRIBUTES
}generalgatewaydevmqtt_t;

extern void *generalgatewaydevmqtt_new(void);
extern int generalgatewaydevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs);
extern int generalgatewaydevmqtt_free(void *vobj);


/*******************************************************************************
*                          General Gateway Subdevice                          *
*******************************************************************************/
typedef enum _generalgatewaysubdev_state_t
{
    MQTT_BASESTATE(GENERALGATEWAYSUBDEV),
    GENERALGATEWAYSUBDEVMQTT_STATE_END
}generalgatewaysubdev_state_t;

#define GENERALGATEWAYSUBDEV_BASEATTRIBUTES \
    GENERALDEV_BASEATTRIBUTES\
    const char *deviceid;\
    int32_t mode;\
    const char *signmethod;

typedef struct _generalgatewaysubdev_t
{
    GENERALGATEWAYDEVMQTT_BASEATTRIBUTES
}generalgatewaysubdev_t;

//extern void *generalgatewaysubdev_new(void);
//extern int generalgatewaysubdev_init(struct event_base *base, void *vobj, config_setting_t *cs);
//extern int generalgatewaysubdev_free(void *vobj);

#endif //__N_GENERAL_H__
