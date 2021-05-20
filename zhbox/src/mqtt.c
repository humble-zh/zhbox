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
    struct _mqtt_t *pthis = vmqttptr;
    MQTTGETBASESTRINGVAR(pthis, key, keylen, dptr);
    return 0;
}

static void mqtt_show(void *vmqttptr)
{
    struct _mqtt_t *pthis = vmqttptr;
    ld(pthis, "{");
    ld(pthis, "  addr: '%s'", pthis->addr);
    ld(pthis, "  port: %d", pthis->port);
    ld(pthis, "  clientid: '%s'", pthis->clientid);
    ld(pthis, "  clean_session: %d", pthis->clean_session);
    ld(pthis, "  usr: '%s'", pthis->usr);
    ld(pthis, "  pwd: '%s'", pthis->pwd);
    ld(pthis, "  keepalive: %d", pthis->keepalive);
    ld(pthis, "  qos: %d", pthis->qos);
    ld(pthis, "}");
    return ;
}

static int mqtt_subscribe(void *vmqttptr, int *mid, const char *sub, int qos)
{
    struct _mqtt_t *pthis = vmqttptr;
    int32_t rc = mosquitto_subscribe(pthis->mosq, mid, sub, qos);
    if (MOSQ_ERR_SUCCESS != rc) { le(pthis, "mosquitto_subscribe() failed:rc%d.'%s'", rc, mosquitto_strerror(rc)); return rc; }
    li(pthis, "sub '%s' qos%d rc%d", sub, qos, rc);
    return rc;
}

static int mqtt_publish(void *vmqttptr, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain)
{
    struct _mqtt_t *pthis = vmqttptr;
    int32_t rc = mosquitto_publish(pthis->mosq, mid, topic, payloadlen, payload, qos, retain);
    if (MOSQ_ERR_SUCCESS != rc) { le(pthis, "mosquitto_publish() failed:rc%d.'%s'", rc, mosquitto_strerror(rc)); return rc; }
    li(pthis, "pub '%s' '%.*s' qos%d rc%d", topic, payloadlen, (char *)payload, qos, rc);
    pthis->count++;
    return rc;
}

static void mqtt_suballtopic(void *vmqttptr)
{
    struct _mqtt_t *pthis = vmqttptr;
    char subtopic[1024] = {0};
    config_setting_t *cssubtopicsls = pthis->cssubtopicsls;
    for(int i = 0; i < config_setting_length(cssubtopicsls); ++i){
        const char *origin = config_setting_get_string_elem(cssubtopicsls, i);
        if(origin == NULL) { le(pthis, "cannot found cssubtopicsls[%u]", i); break; }

        if(!pthis->m_strexpan){ le(pthis, "pthis->m_strexpan() is NULL"); break; }
        int len = pthis->m_strexpan(pthis, origin, subtopic);
        if(len <= 0){ le(pthis, "wrong subtopic'%s'", origin); exit(1); }

        if(!pthis->m_sub){ le(pthis, "pthis->m_sub() is NULL"); break; }
        pthis->m_sub(pthis, NULL, subtopic, 2);
    }
}

static void mqtt_cb_on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    ld(pthis, "MQTT connected");
    pthis->isconnected = 1;
    pthis->state = MQTT_STATE_CONNECTED;
}

static void mqtt_cb_on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    pthis->isconnected = 0;
    pthis->state = MQTT_STATE_DISCONNECTED;
    lw(pthis, "MQTT on_disconnect");
}

static void mqtt_cb_on_sub(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    ld(pthis, "topic subed");
}

static void mqtt_cb_on_unsub(struct mosquitto *mosq, void *obj, int mid)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    ld(pthis, "topic unsubed");
}

static void mqtt_cb_on_pub(struct mosquitto *mosq, void *obj, int mid)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    ld(pthis, "MQTT Message published. mid:%d, count:%d", mid, pthis->count);
    if(!pthis->m_onpub){ return; }
    return pthis->m_onpub(pthis, mid);
}

static void mqtt_cb_on_log(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    /*struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
     ld(pthis, "'%s'", str);*/
}

static void mqtt_cb_on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    struct _mqtt_t *pthis = obj;
    if(pthis->mosq != mosq){ le(pthis, "pthis->mosq != mosq"); return ; }
    //ld(pthis, "'%s' '%s'", (char *)message->topic, (char *)message->payload);
    if(!pthis->m_rcvhandle){ ln(pthis, "pthis->m_rcvhandle() is NULL"); return; }
    return pthis->m_rcvhandle(pthis, (char *)message->topic, (char *)message->payload);
}

void mqtt_set_default(void *vmqttptr)
{
    struct _mqtt_t *pthis = vmqttptr;
    obj_set_default(pthis);
    pthis->m_getvar = mqtt_getvar;
    pthis->m_show = mqtt_show;
    pthis->m_sub = mqtt_subscribe;
    pthis->m_pub = mqtt_publish;
    pthis->m_suball = mqtt_suballtopic;

    pthis->state = MQTT_STATE_INIT;
}

void mqtt_init(void *vmqttptr)
{
    struct _mqtt_t *pthis = vmqttptr;
    pthis->mosq = mosquitto_new(pthis->clientid, pthis->clean_session, pthis);
    if (NULL == pthis->mosq) { le(pthis, "mosquitto_new() failed"); exit(1); }
    mosquitto_username_pw_set(pthis->mosq, pthis->usr, pthis->pwd);
    mosquitto_connect_callback_set(pthis->mosq, mqtt_cb_on_connect);
    mosquitto_disconnect_callback_set(pthis->mosq, mqtt_cb_on_disconnect);
    mosquitto_subscribe_callback_set(pthis->mosq, mqtt_cb_on_sub);
    mosquitto_unsubscribe_callback_set(pthis->mosq, mqtt_cb_on_unsub);
    mosquitto_publish_callback_set(pthis->mosq, mqtt_cb_on_pub);
    mosquitto_log_callback_set(pthis->mosq, mqtt_cb_on_log);
    mosquitto_message_callback_set(pthis->mosq, mqtt_cb_on_message);
    //mosquitto_will_set(mosq, willtopic, strlen(willpayloadlen), willpayload, 1, false);
}
