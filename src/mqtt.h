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
#ifndef __MQTT_H__
#define __MQTT_H__
#include <libconfig.h>
#include <mosquitto.h>
#include <event.h>
#include "l.h"
#include "obj.h"
#include "task.h"

#define MQTT_BASESTATE(PREFIX) \
    OBJ_BASESTATE(PREFIX##MQTT),\
    PREFIX##MQTT##_STATE_CONNECTING,\
    PREFIX##MQTT##_STATE_DISCONNECTED,\
    PREFIX##MQTT##_STATE_CONNECTED,\
    PREFIX##MQTT##_STATE_RECONNECTING,\
    PREFIX##MQTT##_STATE_RUNING

typedef enum _mqtt_state_t
{
    MQTT_BASESTATE(),
    MQTT_STATE_END
}mqtt_state_t;


typedef void mqtt_t;

typedef int (*method_subscribe_t)(void *vmqttptr, int *mid, const char *sub, int qos);
typedef int (*method_publish_t)(void *vmqttptr, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain);
typedef void (*method_subscribeall_t)(void *vmqttptr);
typedef int (*method_tasksrun_t)(void *vmqttptr);
typedef void (*method_rcvhandle_t)(void *vmqttptr, char *topic, char *payload);
typedef void (*method_onpub_t)(void *vmqttptr, int mid);

#define MQTT_BASEATTRIBUTES \
    OBJ_BASEATTRIBUTES\
    TASK_BASEATTRIBUTES\
    const char *addr;\
    int32_t port;\
    const char *clientid;\
    int32_t clean_session;\
    const char *usr;\
    const char *pwd;\
    int32_t keepalive;\
    int32_t qos;\
    struct mosquitto *mosq;\
    int32_t state;\
    int8_t isconnected;\
    int32_t count;\
    struct timeval fsmtv;\
    struct event fsmev;\
    config_setting_t *cssubtopicsls;\
    method_subscribe_t    m_sub;\
    method_publish_t      m_pub;\
    method_subscribeall_t m_suball;\
    method_tasksrun_t     m_tasksrun;\
    method_rcvhandle_t    m_rcvhandle;\
    method_onpub_t        m_onpub;

#define MQTTGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    OBJGETBASESTRINGVAR(this, key, keylen, dptr);\
    if(!strncmp("addr", key, keylen)){ return sprintf(dptr, "%s", this->addr); }\
    if(!strncmp("port", key, keylen)){ return sprintf(dptr, "%d", this->port); }\
    if(!strncmp("clientid", key, keylen)){ return sprintf(dptr, "%s", this->clientid); }\
    if(!strncmp("usr", key, keylen)){ return sprintf(dptr, "%s", this->usr); }\
    if(!strncmp("pwd", key, keylen)){ return sprintf(dptr, "%s", this->pwd); }\
    if(!strncmp("count", key, keylen)){ return sprintf(dptr, "%d", this->count); }\
} while (0)

struct _mqtt_t
{
    MQTT_BASEATTRIBUTES
};


#define mqttle(fmt, arg...) do\
{\
    objle("'%s' " fmt, this->clientid, ## arg);\
} while (0)

#define mqttlw(fmt, arg...) do\
{\
    objlw("'%s' " fmt, this->clientid, ## arg);\
} while (0)

#define mqttln(fmt, arg...) do\
{\
    objln("'%s' " fmt, this->clientid, ## arg);\
} while (0)

#define mqttli(fmt, arg...) do\
{\
    objli("'%s' " fmt, this->clientid, ## arg);\
} while (0)

#define mqttld(fmt, arg...) do\
{\
    objld("'%s' " fmt, this->clientid, ## arg);\
} while (0)

extern int mqtt_tasksinit(void *vmqttptr);
extern void mqtt_set_default(void *vmqttptr);
extern void mqtt_init(void *vmqttptr);

#endif //__MQTT_H__
