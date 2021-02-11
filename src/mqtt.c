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
#include <ctype.h>
#include "l.h"
#include "lib.h"
#include "mqtt.h"

static int32_t mqtt_getvar(void *vmqttptr, const char *key, int32_t keylen, char *dptr)
{
    mqtt_t *this = vmqttptr;
    MQTTGETBASESTRINGVAR(this, key, keylen, dptr);
    return 0;
}

static int32_t mqtt_stringexpansion(void *vmqttptr, const char *str, char *dptr)
{
    mqtt_t *this = vmqttptr;
    int finallen = 0;
    if((!this) || (!str) || (!dptr)){ mqttle("Argv is NULL"); return 0; }
    if(!this->m_getvar){ mqttle("this->m_getvar() is NULL"); return 0; }

    while(*str != '\0'){
        if(str[0] == '{' && isalpha(str[1])){
            str++;
            int32_t keylen = strchr(str, '}') - str -1;
            int32_t len = this->m_getvar(this, str, keylen, &dptr[finallen]);
            if(len < 0){ mqttle("sprintf() failed"); return 0; }
            finallen += len;
            str += (keylen + 2);
        }
        dptr[finallen++] = *str++;
    }
    dptr[finallen++] = '\0';
    return finallen;
}

static void mqtt_show(void *vmqttptr)
{
    mqtt_t *this = vmqttptr;
    mqttld("{");
    mqttld("  cloud: %d", this->cloudid);
    mqttld("  name: '%s'", this->name);
    mqttld("  addr: '%s'", this->addr);
    mqttld("  port: %d", this->port);
    mqttld("  clientid: '%s'", this->clientid);
    mqttld("  clean_session: %d", this->clean_session);
    mqttld("  usr: '%s'", this->usr);
    mqttld("  pwd: '%s'", this->pwd);
    mqttld("  keepalive: %d", this->keepalive);
    mqttld("  qos: %d", this->qos);
    mqttld("}");
    return ;
}

static int mqtt_subscribe(void *vmqttptr, int *mid, const char *sub, int qos)
{
    mqtt_t *this = vmqttptr;
    int32_t rc = mosquitto_subscribe(this->mosq, mid, sub, qos);
    if (MOSQ_ERR_SUCCESS != rc) { mqttle("mosquitto_subscribe() failed:rc%d.'%s'", rc, mosquitto_strerror(rc)); return rc; }
    mqttli("sub '%s' qos%d rc%d", sub, qos, rc);
    return rc;
}

static int mqtt_publish(void *vmqttptr, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain)
{
    mqtt_t *this = vmqttptr;
    int32_t rc = mosquitto_publish(this->mosq, mid, topic, payloadlen, payload, qos, retain);
    if (MOSQ_ERR_SUCCESS != rc) { mqttle("mosquitto_publish() failed:rc%d.'%s'", rc, mosquitto_strerror(rc)); return rc; }
    mqttli("pub '%s' '%.*s' qos%d rc%d", topic, payloadlen, (char *)payload, qos, rc);
    return rc;
}

static void mqtt_suballtopic(void *vmqttptr)
{
    mqtt_t *this = vmqttptr;
    char subtopic[1024] = {0};
    config_setting_t *cssubtopicsls = this->cssubtopicsls;
    for(int i = 0; i < config_setting_length(cssubtopicsls); ++i){
        const char *origin = config_setting_get_string_elem(cssubtopicsls, i);
        if(origin == NULL) { mqttle("cannot found cssubtopicsls[%u]", i); break; }

        if(!this->m_strexpan){ mqttle("this->m_strexpan() is NULL"); break; }
        int len = this->m_strexpan(this, origin, subtopic);
        if(len <= 0){ mqttle("wrong subtopic'%s'", origin); exit(1); }

        if(!this->m_sub){ mqttle("this->m_sub() is NULL"); break; }
        this->m_sub(this, NULL, subtopic, 2);
    }
}

static int mqtt_tasksrun(void *vmqttptr)
{
    mqtt_t *this = vmqttptr;
    int32_t now = time(NULL);
    config_setting_t *cstasksls = this->cstasksls;
    for(int i = 0; i < this->taskstot; ++i){
        if(now - this->taskslasttimels[i] < this->tasksintervalls[i]){ continue; }

        config_setting_t *cstask = config_setting_get_elem(this->cstasksls, i);
        if(!cstask){ mqttle("config_setting_get_elem() failed"); return -1; }

        //topic
        const char *origin = NULL;
        config_setting_lookup_string(cstask, "pubtopic", &origin);
        if(!origin){ mqttle("config_setting_lookup_string(pubtopic)"); continue; }

        char pubtopic[1024] = {0};
        if(!this->m_strexpan){ mqttle("this->m_strexpan() is NULL"); break; }
        int topiclen = this->m_strexpan(this, origin, pubtopic);
        if(topiclen <= 0){ mqttle("wrong pubtopic'%s'", origin); exit(1); }

        //payload
        const char *payloadfmt = NULL;
        config_setting_lookup_string(cstask, "payloadfmt", &payloadfmt);
        if(!payloadfmt) { mqttle("config_setting_lookup_string(payloadfmt)"); continue; }

        char payload[4096000] = {0};
        if(!this->m_strexpan){ mqttle("this->m_strexpan() is NULL"); break; }
        int payloadlen = this->m_strexpan(this, payloadfmt, payload);
        if(payloadlen <= 0){ mqttle("wrong payloadfmt'%s'", payloadfmt); exit(1); }

        if(!this->m_pub){ mqttle("this->m_pub() is NULL"); break; }
        this->m_pub(this, NULL, pubtopic, payloadlen, payload, 2, false);
        this->taskslasttimels[i] = now;
    }
    return 0;
}

static void mqtt_cb_on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    mqttld("MQTT connected");
    this->isconnected = 1;
    this->state = MQTT_STATE_CONNECTED;
}

static void mqtt_cb_on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    this->isconnected = 0;
    this->state = MQTT_STATE_DISCONNECTED;
    mqttlw("MQTT on_disconnect");
}

static void mqtt_cb_on_sub(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    mqttld("topic subed");
}

static void mqtt_cb_on_unsub(struct mosquitto *mosq, void *obj, int mid)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    mqttld("topic unsubed");
}

static void mqtt_cb_on_pub(struct mosquitto *mosq, void *obj, int mid)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    mqttld("MQTT Message published. mid:%d, pubcnt:%d", mid, this->pubcnt);
    if(!this->m_onpub){ return; }
    return this->m_onpub(this, mid);
}

static void mqtt_cb_on_log(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    /*mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
     mqttld("'%s'", str);*/
}

static void mqtt_cb_on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    mqtt_t *this = (mqtt_t *)obj;
    if(this->mosq != mosq){ mqttle("this->mosq != mosq"); return ; }
    mqttld("'%s' '%s'", (char *)message->topic, (char *)message->payload);
    if(!this->m_rcvhandle){ mqttln("this->m_rcvhandle() is NULL"); return; }
    return this->m_rcvhandle(this, (char *)message->topic, (char *)message->payload);
}

int mqtt_tasksinit(void *vmqttptr)
{
    mqtt_t *this = vmqttptr;
    this->taskstot = config_setting_length(this->cstasksls);
    if(this->taskstot){
        this->taskslasttimels = malloc(sizeof(int32_t) * this->taskstot);
        if(!this->taskslasttimels){ mqttle("malloc() %s", strerror(errno)); return -1; }

        this->tasksintervalls = malloc(sizeof(int32_t) * this->taskstot);
        if(!this->tasksintervalls){ mqttle("malloc() %s", strerror(errno)); return -1; }
    }

    memset(this->taskslasttimels, 0, sizeof(int32_t) * this->taskstot);
    for(int i = 0; i < this->taskstot; ++i){
        config_setting_t *cstask = config_setting_get_elem(this->cstasksls, i);
        if(!cstask){ mqttle("config_setting_get_elem(%u) failed", i); return -1; }
        config_setting_lookup_int(cstask, "interval", &this->tasksintervalls[i]);
        mqttld("taskls[%d].interval: %d", i, this->tasksintervalls[i]);
    }
    return 0;
}

void mqtt_set_default(mqtt_t *this)
{
    this->m_getvar = mqtt_getvar;
    this->m_strexpan = mqtt_stringexpansion;
    this->m_show = mqtt_show;
    this->m_sub = mqtt_subscribe;
    this->m_pub = mqtt_publish;
    this->m_suball = mqtt_suballtopic;
    this->m_tasksrun = mqtt_tasksrun;

    this->mosq = mosquitto_new(this->clientid, this->clean_session, this);
    if (NULL == this->mosq) { mqttle("mosquitto_new() failed"); exit(1); }
    mosquitto_username_pw_set(this->mosq, this->usr, this->pwd);
    mosquitto_connect_callback_set(this->mosq, mqtt_cb_on_connect);
    mosquitto_disconnect_callback_set(this->mosq, mqtt_cb_on_disconnect);
    mosquitto_subscribe_callback_set(this->mosq, mqtt_cb_on_sub);
    mosquitto_unsubscribe_callback_set(this->mosq, mqtt_cb_on_unsub);
    mosquitto_publish_callback_set(this->mosq, mqtt_cb_on_pub);
    mosquitto_log_callback_set(this->mosq, mqtt_cb_on_log);
    mosquitto_message_callback_set(this->mosq, mqtt_cb_on_message);
    //mosquitto_will_set(mosq, willtopic, strlen(willpayloadlen), willpayload, 1, false);

    this->state = MQTT_STATE_INIT;
}
