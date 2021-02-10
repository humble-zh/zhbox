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

#define MQTT_BASESTATE(CLOUD) \
    MQTT##CLOUD##_STATE_INIT = 0,\
    MQTT##CLOUD##_STATE_CONNECTING,\
    MQTT##CLOUD##_STATE_DISCONNECTED,\
    MQTT##CLOUD##_STATE_CONNECTED,\
    MQTT##CLOUD##_STATE_RECONNECTING,\
    MQTT##CLOUD##_STATE_RUNING

typedef enum _mqtt_state_t{
    MQTT_BASESTATE(),
    MQTT_STATE_END
}mqtt_state_t;


typedef struct _mqtt_t mqtt_t;

#define MQTTGETBASESTRINGVAR(this, key, keylen, dptr) do {\
    if(!strncmp("name", key, keylen)){ return sprintf(dptr, "%s", this->name); }\
    if(!strncmp("addr", key, keylen)){ return sprintf(dptr, "%s", this->addr); }\
    if(!strncmp("port", key, keylen)){ return sprintf(dptr, "%d", this->port); }\
    if(!strncmp("clientid", key, keylen)){ return sprintf(dptr, "%s", this->clientid); }\
    if(!strncmp("usr", key, keylen)){ return sprintf(dptr, "%s", this->usr); }\
    if(!strncmp("pwd", key, keylen)){ return sprintf(dptr, "%s", this->pwd); }\
    if(!strncmp("pubcnt", key, keylen)){ return sprintf(dptr, "%d", this->pubcnt); }\
} while (0)

typedef int (*method_getvar_t)(void *vmqttptr, const char *key, int32_t keylen, char *dptr);
typedef int (*method_topicexpansion_t)(void *vmqttptr, const char *str, char *dptr);
typedef void (*method_show_t)(void *vmqttptr);
typedef int (*method_subscribe_t)(void *vmqttptr, int *mid, const char *sub, int qos);
typedef int (*method_publish_t)(void *vmqttptr, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain);
typedef void (*method_subscribeall_t)(void *vmqttptr);
typedef int (*method_tasksrun_t)(void *vmqttptr);
typedef void (*method_rcvhandle_t)(void *vmqttptr, char *topic, char *payload);
typedef void (*method_onpub_t)(void *vmqttptr, int mid);

#define MQTT_BASEATTRIBUTES \
    int32_t cloudid;\
    const char *name;\
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
    int32_t pubcnt;\
    struct timeval fsmtv;\
    struct event fsmev;\
    config_setting_t *cssubtopicsls;\
    config_setting_t *cstasksls;\
    int32_t taskstot;\
    int32_t *taskslasttimels;\
    int32_t *tasksintervalls;\
    method_getvar_t           m_getvar;\
    method_topicexpansion_t   m_strexpan;\
    method_show_t             m_show;\
    method_subscribe_t        m_sub;\
    method_publish_t          m_pub;\
    method_subscribeall_t     m_suball;\
    method_tasksrun_t         m_tasksrun;\
    method_rcvhandle_t        m_rcvhandle;\
    method_onpub_t            m_onpub;

struct _mqtt_t{
    MQTT_BASEATTRIBUTES
};


#define mqttle(fmt, arg...) do {\
    fprintf(stderr, "%s:%d: '%s' " fmt "\n", __FUNCTION__, __LINE__, this->clientid, ## arg);\
    syslog(LOG_ERR, "%s:%d: '%s' " fmt, __FUNCTION__, __LINE__, this->clientid, ## arg);\
} while (0)

#define mqttlw(fmt, arg...) do {\
    printf("%s:%d: '%s' " fmt "\n", __FUNCTION__, __LINE__, this->clientid, ## arg);\
    syslog(LOG_WARNING, "%s:%d: '%s' " fmt, __FUNCTION__, __LINE__, this->clientid, ## arg);\
} while (0)

#define mqttln(fmt, arg...) do {\
    printf("%s:%d: '%s' " fmt "\n", __FUNCTION__, __LINE__, this->clientid, ## arg);\
    syslog(LOG_NOTICE, "%s:%d: '%s' " fmt, __FUNCTION__, __LINE__, this->clientid, ## arg);\
} while (0)

#define mqttli(fmt, arg...) do {\
    printf("%s:%d: '%s' " fmt "\n", __FUNCTION__, __LINE__, this->clientid, ## arg);\
    syslog(LOG_INFO, "%s:%d: '%s' " fmt, __FUNCTION__, __LINE__, this->clientid, ## arg);\
} while (0)

#define mqttld(fmt, arg...) do {\
    printf("%s:%d: '%s' " fmt "\n", __FUNCTION__, __LINE__, this->clientid, ## arg);\
    syslog(LOG_DEBUG, "%s:%d: '%s' " fmt, __FUNCTION__, __LINE__, this->clientid, ## arg);\
} while (0)

extern int mqtt_tasksinit(void *vmqttptr);
extern void mqtt_set_default(mqtt_t *mqttptr);

#endif //__MQTT_H__
