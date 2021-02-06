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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <mosquitto.h>
#include "mqtt.h"
#include "l.h"


void mosq_cb_on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    mqtt_t *mqttptr = (mqtt_t *)obj;
    mqttptr->isconnected = 1;
    mqttptr->state = MQTT_STATE_RUNING;
    l_d("%s MQTT connected", mqttptr->clientid);
    //should we subscribe some topics??
}

void mosq_cb_on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    mqtt_t *mqttptr = (mqtt_t *)obj;
    mqttptr->isconnected = 0;
    mqttptr->state = MQTT_STATE_DISCONNECTED;
    l_w("MQTT on_disconnect");
}

void mosq_cb_on_sub(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) { l_d("topic subed"); }

void mosq_cb_on_unsub(struct mosquitto *mosq, void *obj, int mid) { l_d("topic unsubed"); }

void mosq_cb_on_log(struct mosquitto *mosq, void *obj, int level, const char *str) { /*l_d("%s", str);*/ }

void mosq_cb_on_pub(struct mosquitto *mosq, void *obj, int mid) { l_d("msg pubed.mid:%d", mid); }

void mosq_rcv(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) { l_i("%s %s", (char *)message->topic, (char *)message->payload); }


/*
int mosq_pub(struct mosquitto *mosq, const char *topic, int32_t payloadlen, const char *payload, int qos)
{
    int32_t rc;
    const char *error_string = NULL;

    if (NULL == mosq || NULL == topic || NULL == payload || payloadlen <= 0) {
        l_e("Param invalid");
        return -1;
    }

    l_i("%s %s qos%d", topic, payload, qos);
    rc = mosquitto_publish(mosq, NULL, topic, payloadlen, payload, qos, true);
    if (MOSQ_ERR_SUCCESS != rc) {
        error_string = mosquitto_strerror(rc);
        l_e("Pub failed:%s", error_string);
        return -1;
    }

    return 0;
}
*/


void mqtt_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    struct timeval tv;
    mqtt_t *mqttptr = (mqtt_t *)user_data;

    switch (mqttptr->state) {
        case MQTT_STATE_INIT:
        case MQTT_STATE_CONNECTING:
            rc = mosquitto_connect_async(mqttptr->mosq, mqttptr->addr, mqttptr->port, mqttptr->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) {  l_d("mosquitto_connect_async() rc %d", rc); }
            else { l_w("mosquitto_connect_async() failed:rc %d, %s, reconnecting", rc, mosquitto_strerror(rc)); }
            mqttptr->state = MQTT_STATE_RECONNECTING; //connect once,then reconnect always if needed
            tv.tv_sec = 5; tv.tv_usec = 0;
            break;

        case MQTT_STATE_DISCONNECTED:
        case MQTT_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(mqttptr->mosq);
            if (MOSQ_ERR_SUCCESS == rc) { l_d("mosquitto_reconnect_async() rc %d", rc); }
            else { l_w("mosquitto_reconnect_async() failed:rc %d, %s, reconnecting", rc, mosquitto_strerror(rc)); }
            tv.tv_sec = 5; tv.tv_usec = 0;
            break;

        case MQTT_STATE_CONNECTED:
            break;

        case MQTT_STATE_RUNING:
            if(mqttptr->taskcb){ mqttptr->taskcb(mqttptr->taskarg); }
            else{ l_d("MQTT runing, do nothing"); }
            tv.tv_sec = mqttptr->taskinterval.tv_sec;
            tv.tv_usec = mqttptr->taskinterval.tv_usec;
            break;

        default:
            l_w("Unkown MQTT_STATE_STATE:%d", mqttptr->state);
            break;
    }

    event_add(&mqttptr->fsmev, &tv); //redo
}
