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
#include "l.h"
#include "lib.h"
#include "mqttgeneral.h"

static void mqttgeneral_your_rcvhandle(void *vmqttptr, char *topic, char *payload)
{
    mqttgeneral_t *this = vmqttptr;
    //TODO parse the alink payload
}

static void mqttgeneral_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    mqttgeneral_t *this = (mqttgeneral_t *)user_data;

    switch (this->state) {
        case MQTTGENERAL_STATE_INIT:
        case MQTTGENERAL_STATE_CONNECTING:
            rc = mosquitto_connect_async(this->mosq, this->addr, this->port, this->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) {
                mqttln("mosquitto_connect_async(%s,%d) rc%d", this->addr, this->port, rc);
            }
            else {
                mqttln("mosquitto_connect_async(%s,%d) failed:rc%d.%s, reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            mqttld("start to loop");
            mosquitto_loop_start(this->mosq);
            this->state = MQTTGENERAL_STATE_RECONNECTING;
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case MQTTGENERAL_STATE_DISCONNECTED:
        case MQTTGENERAL_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(this->mosq);
            if (MOSQ_ERR_SUCCESS == rc) {
                mqttln("mosquitto_reconnect_async(%s,%d) rc%d", this->addr, this->port, rc);
            }
            else {
                mqttln("mosquitto_reconnect_async(%s,%d) failed:rc%d.%s, reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case MQTTGENERAL_STATE_CONNECTED:
            if(!this->m_suball){ mqttle("this->m_suball() is NULL"); break; }
            this->m_suball(this);
            this->state = MQTTGENERAL_STATE_RUNING;
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 1;
            break;

        case MQTTGENERAL_STATE_RUNING:
            //mqttld("MQTT runing, check and iterate your tasksls here");
            if(this->m_tasksrun){ this->m_tasksrun(this); }
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 10;
            break;

        default:
            mqttlw("Unkown MQTTGENERAL_STATE:%d", this->state);
            break;
    }

    event_add(&this->fsmev, &this->fsmtv); //redo
}

mqtt_t *mqttgeneral_new(void) //new an mqttgeneral_t object
{
    mqttgeneral_t *this = malloc(sizeof(mqttgeneral_t));
    if(this == NULL){ l_e("malloc() %s", strerror(errno)); return NULL; }
    memset(this, 0, sizeof(mqttgeneral_t));
    return (mqtt_t *)this;
}

int mqttgeneral_init(struct event_base *base, mqtt_t *mqttptr, config_setting_t *csmqtt)
{
    mqttgeneral_t *this = (mqttgeneral_t *)mqttptr;
    //1. read the config from csmqtt
    getsetstr(csmqtt, "name", this->name);
    getsetstr(csmqtt, "addr", this->addr);
    getsetint(csmqtt, "port", this->port);
    getsetbool(csmqtt, "clean_session", this->clean_session);
    getsetstr(csmqtt, "clientid", this->clientid);
    getsetstr(csmqtt, "usr", this->usr);
    getsetstr(csmqtt, "pwd", this->pwd);
    getsetint(csmqtt, "keepalive", this->keepalive);
    getsetint(csmqtt, "qos", this->qos);

    //2. set all the methods to default
    mqtt_set_default(mqttptr);

    //3. set some methods to yours rewrited
    this->m_rcvhandle = mqttgeneral_your_rcvhandle;
    //this->m_onpub = your_rewrited_on_publisned_function;
    //this->m_show = your_rewrited_show_function;

    //4. print all the params. or you can rewrite it aboved
    if(!this->m_show){ mqttlw("this->m_show() is NULL"); }
    else { this->m_show(mqttptr); }

    //5. topics list to subscribe
    this->cssubtopicsls = config_setting_lookup(csmqtt, "subtopics");
    if(this->cssubtopicsls == NULL) { mqttle("config_setting_lookup(subtopics) failed"); return -1; }

    //6. tasks list to run after mqtt-connected
    this->cstasksls = config_setting_lookup(csmqtt, "tasksls");
    if(this->cstasksls == NULL) { mqttle("config_setting_lookup(tasksls) failed"); return -1; }
    if(mqtt_tasksinit(this) < 0){ return -1; }

    //7. FSM for this(mqttobject)
    this->fsmtv.tv_sec = 1; this->fsmtv.tv_usec = 0;
    event_assign(&this->fsmev, base, -1, EV_TIMEOUT, mqttgeneral_fsm_cb, this);
    event_add(&this->fsmev, &this->fsmtv);
    return 0;
}

int mqttgeneral_free(mqtt_t *mqttptr) //free the object
{
    mqttgeneral_t *this = (mqttgeneral_t *)mqttptr;
    mosquitto_loop_stop(this->mosq, false);
    mosquitto_destroy(this->mosq);
    //if(this->anythingelse){ free(this->anythingelse); }
    if(this){ free(this); }
    return 0;
}
