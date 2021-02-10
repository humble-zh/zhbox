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
#include "mqttali.h"

static int32_t mqttali_getvar(void *vmqttptr, const char *key, int32_t keylen, char *dptr)
{
    mqttali_t *this = vmqttptr;
    MQTTGETBASESTRINGVAR(this, key, keylen, dptr);
    if(!strncmp("RegionId", key, keylen)){ return sprintf(dptr, "%s", this->regionid); }
    if(!strncmp("DeviceId", key, keylen)){ return sprintf(dptr, "%s", this->deviceid); }
    if(!strncmp("ProductKey", key, keylen)){ return sprintf(dptr, "%s", this->productkey); }
    if(!strncmp("DeviceName", key, keylen)){ return sprintf(dptr, "%s", this->devicename); }
    if(!strncmp("DeviceSecret", key, keylen)){ return sprintf(dptr, "%s", this->devicesecret); }
    if(!strncmp("Mode", key, keylen)){ return sprintf(dptr, "%d", this->mode); }
    if(!strncmp("SignMethod", key, keylen)){ return sprintf(dptr, "%s", this->signmethod); }
    return 0;
}

static void mqttali_your_rcvhandle(void *vmqttptr, char *topic, char *payload)
{
    mqttali_t *this = vmqttptr;
    //TODO parse the payload of yours
}

static void mqttali_alink_rcvhandle(void *vmqttptr, char *topic, char *payload)
{
    mqttali_t *this = vmqttptr;
    //TODO parse the payload of alink
}

static void mqttali_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    mqttali_t *this = (mqttali_t *)user_data;

    switch (this->state) {
        case MQTTALI_STATE_INIT:
        case MQTTALI_STATE_CONNECTING:
            rc = mosquitto_connect_async(this->mosq, this->addr, this->port, this->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_connect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_connect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            mqttld("start to loop");
            mosquitto_loop_start(this->mosq);
            this->state = MQTTALI_STATE_RECONNECTING;
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case MQTTALI_STATE_DISCONNECTED:
        case MQTTALI_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(this->mosq);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_reconnect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_reconnect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case MQTTALI_STATE_CONNECTED:
            if(!this->m_suball){ mqttle("this->m_suball() is NULL"); break; }
            this->m_suball(this);
            this->state = MQTTALI_STATE_RUNING;
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 1;
            break;

        case MQTTALI_STATE_RUNING:
            //mqttld("MQTT runing, check and iterate your tasksls here");
            if(this->m_tasksrun){ this->m_tasksrun(this); }
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 10;
            break;

        default:
            mqttlw("Unkown MQTTALI_STATE:%d", this->state);
            break;
    }

    event_add(&this->fsmev, &this->fsmtv); //redo
}

mqtt_t *mqttali_new(void) //new an mqttali_t object
{
    mqttali_t *this = malloc(sizeof(mqttali_t));
    if(this == NULL){ l_e("malloc() '%s'", strerror(errno)); return NULL; }
    memset(this, 0, sizeof(mqttali_t));
    return (mqtt_t *)this;
}

int mqttali_init(struct event_base *base, mqtt_t *mqttptr, config_setting_t *csmqtt)
{
    mqttali_t *this = (mqttali_t *)mqttptr;

    //1. read the config from csmqtt
    getsetstr(csmqtt, "name", this->name);
    getsetstr(csmqtt, "RegionId", this->regionid);
    getsetstr(csmqtt, "DeviceId", this->deviceid);
    getsetstr(csmqtt, "ProductKey", this->productkey);
    this->devicename = this->name;
    getsetstr(csmqtt, "DeviceSecret", this->devicesecret);
    //getsetint(csmqtt, "Mode", this->mode);
    getsetstr(csmqtt, "SignMethod", this->signmethod);

    char buf[256] = {0};

    snprintf(buf, sizeof(buf), "%s.iot-as-mqtt.%s.aliyuncs.com", this->productkey, this->regionid);
    this->addr = strdup(buf); //free(this->addr) in mqttali_free()
    if(!this->addr){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", this->addr);

    getsetint(csmqtt, "port", this->port);
    getsetbool(csmqtt, "clean_session", this->clean_session);

    snprintf(buf, sizeof(buf), "%s|securemode=3,signmethod=%s|", this->deviceid, this->signmethod);
    this->clientid = strdup(buf); //free(this->clientid) in mqttali_free()
    if(!this->clientid){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", this->clientid);

    snprintf(buf, sizeof(buf), "%s&%s", this->devicename, this->productkey);
    this->usr = strdup(buf); //free(this->usr) in mqttali_free()
    if(!this->usr){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", this->usr);

    getsetstr(csmqtt, "pwd", this->pwd);
    getsetint(csmqtt, "keepalive", this->keepalive);
    getsetint(csmqtt, "qos", this->qos);

    //2. set all the methods to default
    mqtt_set_default(mqttptr);

    //3. set some methods to yours rewrited
    this->m_getvar = mqttali_getvar;
    //if(getfrom(csmqtt, usealink){
        this->m_rcvhandle = mqttali_alink_rcvhandle;
    //}
    //else{ this->m_rcvhandle = mqttali_your_rcvhandle; }

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
    event_assign(&this->fsmev, base, -1, EV_TIMEOUT, mqttali_fsm_cb, this);
    event_add(&this->fsmev, &this->fsmtv);
    return 0;
}

int mqttali_free(mqtt_t *mqttptr) //free the object
{
    mqttali_t *this = (mqttali_t *)mqttptr;
    mosquitto_loop_stop(this->mosq, false);
    mosquitto_destroy(this->mosq);
    //if(this->anythingelse){ free(this->anythingelse); }
    //if(this->pwd){ free(this->pwd); }
    if(this->usr){ free((void *)this->usr); }
    if(this->clientid){ free((void *)this->clientid); }
    if(this->addr){ free((void *)this->addr); }
    if(this){ free(this); }
    return 0;
}
