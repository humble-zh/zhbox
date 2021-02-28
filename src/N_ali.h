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
#ifndef __N_ALI_H__
#define __N_ALI_H__
#include "mqtt.h"

/*******************************************************************************
*                                 Ali Device                                  *
*******************************************************************************/
#define ALIDEV_BASEATTRIBUTES \
    const char *productkey;\
    const char *devicename;\
    const char *devicesecret;\
    int64_t loginmillisecond;

#define ALIDEVGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    OBJGETBASESTRINGVAR(this, key, keylen, dptr);\
    if(!strncmp("ProductKey", key, keylen)){ return sprintf(dptr, "%s", this->productkey); }\
    if(!strncmp("DeviceName", key, keylen)){ return sprintf(dptr, "%s", this->devicename); }\
    if(!strncmp("DeviceSecret", key, keylen)){ return sprintf(dptr, "%s", this->devicesecret); }\
    if(!strncmp("LoginMilliSecond", key, keylen)){ return sprintf(dptr, "%" PRId64, this->loginmillisecond); }\
} while (0)

/*******************************************************************************
*                          Ali Direct Device by MQTT                          *
*******************************************************************************/
typedef enum _alidirectdevmqtt_state_t
{
    MQTT_BASESTATE(ALIDIRECTDEV),
    ALIDIRECTDEVMQTT_STATE_END
}alidirectdevmqtt_state_t;

#define ALIDIRECTDEVMQTT_BASEATTRIBUTES \
    MQTT_BASEATTRIBUTES\
    ALIDEV_BASEATTRIBUTES\
    const char *regionid;\
    const char *deviceid;\
    int32_t mode;\
    const char *signmethod;

#define ALIDIRTECTDEVMQTTGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    MQTTGETBASESTRINGVAR(this, key, keylen, dptr);\
    ALIDEVGETBASESTRINGVAR(this, key, keylen, dptr);\
    if(!strncmp("RegionId", key, keylen)){ return sprintf(dptr, "%s", this->regionid); }\
    if(!strncmp("DeviceId", key, keylen)){ return sprintf(dptr, "%s", this->deviceid); }\
    if(!strncmp("Mode", key, keylen)){ return sprintf(dptr, "%d", this->mode); }\
    if(!strncmp("SignMethod", key, keylen)){ return sprintf(dptr, "%s", this->signmethod); }\
} while (0)

typedef struct _alidirectdevmqtt_t
{
    ALIDIRECTDEVMQTT_BASEATTRIBUTES
}alidirectdevmqtt_t;

extern void *alidirectdevmqtt_new(void);
extern int alidirectdevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs);
extern int alidirectdevmqtt_free(void *vobj);


/*******************************************************************************
*                         Ali Gateway Device by MQTT                          *
*******************************************************************************/
typedef enum _aligatewaydevmqtt_state_t
{
    MQTT_BASESTATE(ALIGATEWAYDEV),
    ALIGATEWAYDEVMQTT_STATE_TOPOGET,
    ALIGATEWAYDEVMQTT_STATE_END
}aligatewaydevmqtt_state_t;

#define ALIGATEWAYDEVMQTT_BASEATTRIBUTES \
    MQTT_BASEATTRIBUTES\
    ALIDEV_BASEATTRIBUTES\
    const char *regionid;\
    const char *deviceid;\
    int32_t mode;\
    const char *signmethod;\
    /* SOUTH LIST */\
    int32_t southlstot;\
    void **southarray;

typedef struct _aligatewaydevmqtt_t
{
    ALIGATEWAYDEVMQTT_BASEATTRIBUTES
}aligatewaydevmqtt_t;

extern void *aligatewaydevmqtt_new(void);
extern int aligatewaydevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs);
extern int aligatewaydevmqtt_free(void *vobj);

#endif //__N_ALI_H__
