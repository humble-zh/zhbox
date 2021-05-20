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
#include <string.h>
#include <time.h>
#include <openssl/hmac.h>
#include "aliyun.h"
#include "lib.h"
#include "task.h"

static int32_t alidirectgatewaydevmqtt_getvar(void *vpthis, const char *key, int32_t keylen, char *dptr)
{
    alidirectdevmqtt_t *pthis = vpthis;
    ALIDIRTECTDEVMQTTGETBASESTRINGVAR(pthis, key, keylen, dptr);
    return 0;
}


/*******************************************************************************
*                                   Direct                                    *
*******************************************************************************/
static void alidirectdevmqtt_your_rcvhandle(void *vpthis, char *topic, char *payload)
{
    alidirectdevmqtt_t *pthis = vpthis;
    //TODO parse the payload of yours
}

static void alidirectdevmqtt_alink_rcvhandle(void *vpthis, char *topic, char *payload)
{
    alidirectdevmqtt_t *pthis = vpthis;
    //TODO parse the payload of alink
}

static int lsproperties_fe_fn(void * data, size_t data_size, void * arg)
{
    const char *propertyname = data;
    alidirectdevmqtt_t *pthis = arg;
    ld(pthis, "%.*s", data_size, propertyname); //TODO check reference and get value of property
    return 0;
}

static int alitask_run(void *vtask, void *vpthis)
{
    task_t *atask = vtask;
    alidirectdevmqtt_t *pthis = vpthis;

    int32_t now = (int32_t)time(NULL);
    if(now - atask->lasttime < atask->interval) { return -1; }
    ld(pthis, "%s %s", atask->pubtopic, atask->payloadprefix);

    cfulist_foreach(atask->lsproperties, lsproperties_fe_fn, pthis);

    ld(pthis, "%s", atask->payloadsuffix);
    atask->lasttime = now;
    return 0;
}

static void alidirectdevmqtt_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    alidirectdevmqtt_t *pthis = user_data;

    switch (pthis->state) {
        case ALIDIRECTDEVMQTT_STATE_INIT:
        case ALIDIRECTDEVMQTT_STATE_CONNECTING:
            rc = mosquitto_connect_async(pthis->mosq, pthis->addr, pthis->port, pthis->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) { ln(pthis, "mosquitto_connect_async(%s,%d) rc%d", pthis->addr, pthis->port, rc); }
            else {
                ln(pthis, "mosquitto_connect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        pthis->addr, pthis->port, rc, mosquitto_strerror(rc));
            }
            ld(pthis, "start to loop");
            mosquitto_loop_start(pthis->mosq);
            pthis->state = ALIDIRECTDEVMQTT_STATE_RECONNECTING;
            pthis->fsmtv.tv_sec = 5; pthis->fsmtv.tv_usec = 0;
            break;

        case ALIDIRECTDEVMQTT_STATE_DISCONNECTED:
        case ALIDIRECTDEVMQTT_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(pthis->mosq);
            if (MOSQ_ERR_SUCCESS == rc) { ln(pthis, "mosquitto_reconnect_async(%s,%d) rc%d", pthis->addr, pthis->port, rc); }
            else {
                ln(pthis, "mosquitto_reconnect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        pthis->addr, pthis->port, rc, mosquitto_strerror(rc));
            }
            pthis->fsmtv.tv_sec = 10; pthis->fsmtv.tv_usec = 0;
            break;

        case ALIDIRECTDEVMQTT_STATE_CONNECTED:
            if(!pthis->m_suball){ le(pthis, "pthis->m_suball() is NULL"); break; }
            pthis->m_suball(pthis);
            pthis->state = ALIDIRECTDEVMQTT_STATE_RUNING;
            pthis->fsmtv.tv_sec = 0; pthis->fsmtv.tv_usec = 1;
            break;

        case ALIDIRECTDEVMQTT_STATE_RUNING:
            //ld(pthis, "MQTT runing, check and iterate your tasksls here");
            if(pthis->m_tasksrun){ pthis->m_tasksrun((taskls_t *)&pthis->ls, pthis); }
            pthis->fsmtv.tv_sec = 0; pthis->fsmtv.tv_usec = 10;
            break;

        default:
            lw(pthis, "Unkown ALIDIRECTDEVMQTT_STATE:%d", pthis->state);
            break;
    }

    event_add(&pthis->fsmev, &pthis->fsmtv); //redo
}

void *alidirectdevmqtt_new(void)
{
    l_d("alidirectdevmqtt_new");
    alidirectdevmqtt_t *pthis = malloc(sizeof(alidirectdevmqtt_t));
    if(pthis == NULL){ l_e("malloc() '%s'", strerror(errno)); return NULL; }
    memset(pthis, 0, sizeof(alidirectdevmqtt_t));
    return pthis;
}

int alidirectdevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    l_d("alidirectdevmqtt_init");
    alidirectdevmqtt_t *pthis = vobj;

    //1. set all the methods to default
    mqtt_set_default(pthis);

    //2. set some methods to yours rewrited
    pthis->m_getvar = alidirectgatewaydevmqtt_getvar;
    //if(getfrom(cs, usealink){
        pthis->m_rcvhandle = alidirectdevmqtt_alink_rcvhandle;
    //}
    //else{ pthis->m_rcvhandle = alidirectdevmqtt_your_rcvhandle; }

    //pthis->m_onpub = your_rewrited_on_publisned_function;
    //pthis->m_show = your_rewrited_show_function;

    //3. read the config from cs
    cslksr(cs, "name", pthis->name);
    cslksr(cs, "RegionId", pthis->regionid);
    cslksr(cs, "DeviceId", pthis->deviceid);
    cslksr(cs, "ProductKey", pthis->productkey);
    pthis->devicename = pthis->name;
    cslksr(cs, "DeviceSecret", pthis->devicesecret);
    cslkir(cs, "Mode", pthis->mode);
    cslksr(cs, "SignMethod", pthis->signmethod);

    char buf[256] = {0};

    int ret = pthis->m_strexpan(pthis, "{ProductKey}.iot-as-mqtt.{RegionId}.aliyuncs.com", buf);
    if(ret < 0){ return -1; }
    pthis->addr = strdup(buf); //free(pthis->addr) in alidirectdevmqtt_free()
    if(!pthis->addr){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", pthis->addr);

    cslkir(cs, "port", pthis->port);
    cslkbr(cs, "clean_session", pthis->clean_session);

    ret = pthis->m_strexpan(pthis, "{DeviceId}|securemode={Mode},signmethod={SignMethod}|", buf);
    if(ret < 0){ return -1; }
    pthis->clientid = strdup(buf); //free(pthis->clientid) in alidirectdevmqtt_free()
    if(!pthis->clientid){ l_e("strdup() failed"); return -1; }
    l_d("clientid: '%s'", pthis->clientid);

    ret = pthis->m_strexpan(pthis, "{DeviceName}&{ProductKey}", buf);
    if(ret < 0){ return -1; }
    pthis->usr = strdup(buf); //free(pthis->usr) in alidirectdevmqtt_free()
    if(!pthis->usr){ l_e("strdup() failed"); return -1; }
    l_d("usr: '%s'", pthis->usr);

    //password
    pthis->loginmillisecond = pthis->m_getmilliseconds();
    ret = pthis->m_strexpan(pthis, "clientId{DeviceId}deviceName{DeviceName}productKey{ProductKey}", buf);
    if(ret < 0){ return -1; }
    l_d("originpassword:'%s'", buf);

    uint8_t sign[64] = {0};
    uint32_t signlen = 0;
    if(!strcmp(pthis->signmethod, "hmacmd5")){ /* TODO */ }
    else{ //hmacsha1
        if(!HMAC(EVP_sha1(), pthis->devicesecret, strlen(pthis->devicesecret), (unsigned char*)buf, strlen(buf), sign, &signlen)){
            return -1;
        }
    }
    for(int i = 0; i < signlen; ++i) { snprintf(buf + i*2, sizeof(buf), "%02x", sign[i]); }

    pthis->pwd = strdup(buf); //free(pthis->pwd) in aligatewaydevmqtt_free()
    if(!pthis->pwd){ l_e("strdup() failed"); return -1; }
    l_d("pwd: '%s'", pthis->pwd);

    cslkir(cs, "keepalive", pthis->keepalive);
    cslkir(cs, "qos", pthis->qos);

    //4. print all the params. or you can rewrite it aboved
    if(!pthis->m_show){ lw(pthis, "pthis->m_show() is NULL"); }
    else { pthis->m_show(pthis); }

    //5. init the mqtt
    mqtt_init(pthis);

    //6. topics list to subscribe
    pthis->cssubtopicsls = config_setting_lookup(cs, "subtopics");
    if(pthis->cssubtopicsls == NULL) { le(pthis, "config_setting_lookup(subtopics) failed"); return -1; }

    //7. tasks list to run after mqtt-connected TODO
    config_setting_t *cstasksls = config_setting_lookup(cs, "tasksls");
    if(!cstasksls) { le(pthis, "config_setting_lookup(tasksls) failed"); return -1; }
    taskls_t *taskls = (taskls_t *)&pthis->ls;
    if(taskls_init(taskls, cstasksls, alitask_run) < 0){ return -1; }

    //8. FSM for pthis(mqttobject)
    pthis->fsmtv.tv_sec = 1; pthis->fsmtv.tv_usec = 0;
    event_assign(&pthis->fsmev, base, -1, EV_TIMEOUT, alidirectdevmqtt_fsm_cb, pthis);
    event_add(&pthis->fsmev, &pthis->fsmtv);

    return 0;
//TODO take some error label here??? return -1;
}

int alidirectdevmqtt_free(void *vobj)
{
    l_d("alidirectdevmqtt_free");
    alidirectdevmqtt_t *pthis = vobj;
    mosquitto_loop_stop(pthis->mosq, false);
    mosquitto_destroy(pthis->mosq);
    //if(pthis->anythingelse){ free(pthis->anythingelse); }
    if(pthis->pwd){ free((void *)pthis->pwd); }
    if(pthis->usr){ free((void *)pthis->usr); }
    if(pthis->clientid){ free((void *)pthis->clientid); }
    if(pthis->addr){ free((void *)pthis->addr); }
    if(pthis){ free(pthis); }
    return 0;
}


void* aligateway_new(void)
{
    printf("aligateway_new\n");
    return 0;
}
int aligateway_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    printf("aligateway_init\n");
    return 0;
}
int aligateway_free(void *vobj)
{
    printf("aligateway_free\n");
    return 0;
}

void* alisubdevice_new(void)
{
    printf("alisubdevice_new\n");
    return 0;
}
int alisubdevice_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    printf("alisubdevice_init\n");
    return 0;
}
int alisubdevice_free(void *vobj)
{
    printf("alisubdevice_free\n");
    return 0;
}
