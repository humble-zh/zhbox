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
#include <mosquitto.h>
#include <event.h>
#include <event2/util.h>

typedef enum {
    MQTT_STATE_INIT = 0,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_DISCONNECTED,
    MQTT_STATE_RECONNECTING,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_RUNING,
    MQTT_STATE_END
}mqtt_state_t;

typedef int mqtt_task_cb(void *);

#define MQTT_BASEATTRIBUTES \
    char *addr;\
    int32_t port;\
    char *clientid;\
    bool clean_session;\
    char *usr;\
    char *pwd;\
    int32_t keepalive;\
    int8_t qos;\
    struct mosquitto *mosq;\
    mqtt_state_t state;\
    int8_t isconnected;\
    struct event fsmev;\
    struct timeval taskinterval;\
    mqtt_task_cb *taskcb;\
    void *taskarg;


typedef struct {
    MQTT_BASEATTRIBUTES
    int32_t pub_cnt;
    const char *subtopic;
    const char *pubtopic;
}mqtt_t;

//callback function
extern void mosq_cb_on_connect(struct mosquitto *mosq, void *obj, int rc);
extern void mosq_cb_on_disconnect(struct mosquitto *mosq, void *obj, int rc);
extern void mosq_cb_on_sub(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
extern void mosq_cb_on_unsub(struct mosquitto *mosq, void *obj, int mid);
extern void mosq_cb_on_log(struct mosquitto *mosq, void *obj, int level, const char *str);
extern void mosq_cb_on_pub(struct mosquitto *mosq, void *obj, int mid);
extern void mosq_rcv(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

extern int mosq_pub(struct mosquitto *mosq, const char *topic, int32_t payloadlen, const char *payload, int qos);
extern void mqtt_fsm_cb(evutil_socket_t fd, short event, void *user_data);

#endif //__MQTT_H__
