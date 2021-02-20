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
#include <openssl/hmac.h>
#include "l.h"
#include "lib.h"
#include "N_ali.h"
#include "S_opcua.h"

static int32_t alidirectgatewaydevmqtt_getvar(void *vthis, const char *key, int32_t keylen, char *dptr)
{
    alidirectdevmqtt_t *this = vthis;
    ALIDIRTECTDEVMQTTGETBASESTRINGVAR(this, key, keylen, dptr);
    return 0;
}


/*******************************************************************************
*                                   Direct                                    *
*******************************************************************************/
static void alidirectdevmqtt_your_rcvhandle(void *vthis, char *topic, char *payload)
{
    alidirectdevmqtt_t *this = vthis;
    //TODO parse the payload of yours
}

static void alidirectdevmqtt_alink_rcvhandle(void *vthis, char *topic, char *payload)
{
    alidirectdevmqtt_t *this = vthis;
    //TODO parse the payload of alink
}

static void alidirectdevmqtt_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    alidirectdevmqtt_t *this = user_data;

    switch (this->state) {
        case ALIDIRECTDEVMQTT_STATE_INIT:
        case ALIDIRECTDEVMQTT_STATE_CONNECTING:
            rc = mosquitto_connect_async(this->mosq, this->addr, this->port, this->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_connect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_connect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            mqttld("start to loop");
            mosquitto_loop_start(this->mosq);
            this->state = ALIDIRECTDEVMQTT_STATE_RECONNECTING;
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case ALIDIRECTDEVMQTT_STATE_DISCONNECTED:
        case ALIDIRECTDEVMQTT_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(this->mosq);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_reconnect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_reconnect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            this->fsmtv.tv_sec = 10; this->fsmtv.tv_usec = 0;
            break;

        case ALIDIRECTDEVMQTT_STATE_CONNECTED:
            if(!this->m_suball){ mqttle("this->m_suball() is NULL"); break; }
            this->m_suball(this);
            this->state = ALIDIRECTDEVMQTT_STATE_RUNING;
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 1;
            break;

        case ALIDIRECTDEVMQTT_STATE_RUNING:
            //mqttld("MQTT runing, check and iterate your tasksls here");
            if(this->m_tasksrun){ this->m_tasksrun(this); }
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 10;
            break;

        default:
            mqttlw("Unkown ALIDIRECTDEVMQTT_STATE:%d", this->state);
            break;
    }

    event_add(&this->fsmev, &this->fsmtv); //redo
}


void *alidirectdevmqtt_new(void)
{
    alidirectdevmqtt_t *this = malloc(sizeof(alidirectdevmqtt_t));
    if(this == NULL){ l_e("malloc() '%s'", strerror(errno)); return NULL; }
    memset(this, 0, sizeof(alidirectdevmqtt_t));
    return this;
}

int alidirectdevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    alidirectdevmqtt_t *this = vobj;

    //1. set all the methods to default
    mqtt_set_default(this);

    //2. set some methods to yours rewrited
    this->m_getvar = alidirectgatewaydevmqtt_getvar;
    //if(getfrom(cs, usealink){
        this->m_rcvhandle = alidirectdevmqtt_alink_rcvhandle;
    //}
    //else{ this->m_rcvhandle = alidirectdevmqtt_your_rcvhandle; }

    //this->m_onpub = your_rewrited_on_publisned_function;
    //this->m_show = your_rewrited_show_function;

    //3. read the config from cs
    getsetstr(cs, "name", this->name);
    getsetstr(cs, "RegionId", this->regionid);
    getsetstr(cs, "DeviceId", this->deviceid);
    getsetstr(cs, "ProductKey", this->productkey);
    this->devicename = this->name;
    getsetstr(cs, "DeviceSecret", this->devicesecret);
    getsetint(cs, "Mode", this->mode);
    getsetstr(cs, "SignMethod", this->signmethod);

    char buf[256] = {0};

    int ret = this->m_strexpan(this, "{ProductKey}.iot-as-mqtt.{RegionId}.aliyuncs.com", buf);
    if(ret < 0){ return -1; }
    this->addr = strdup(buf); //free(this->addr) in alidirectdevmqtt_free()
    if(!this->addr){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", this->addr);

    getsetint(cs, "port", this->port);
    getsetbool(cs, "clean_session", this->clean_session);

    ret = this->m_strexpan(this, "{DeviceId}|securemode={Mode},signmethod={SignMethod}|", buf);
    if(ret < 0){ return -1; }
    this->clientid = strdup(buf); //free(this->clientid) in alidirectdevmqtt_free()
    if(!this->clientid){ l_e("strdup() failed"); return -1; }
    l_d("clientid: '%s'", this->clientid);

    ret = this->m_strexpan(this, "{DeviceName}&{ProductKey}", buf);
    if(ret < 0){ return -1; }
    this->usr = strdup(buf); //free(this->usr) in alidirectdevmqtt_free()
    if(!this->usr){ l_e("strdup() failed"); return -1; }
    l_d("usr: '%s'", this->usr);

    //password
    this->loginmillisecond = this->m_getmilliseconds();
    ret = this->m_strexpan(this, "clientId{DeviceId}deviceName{DeviceName}productKey{ProductKey}", buf);
    if(ret < 0){ return -1; }
    l_d("originpassword:'%s'", buf);

    uint8_t sign[64] = {0};
    uint32_t signlen = 0;
    if(!strcmp(this->signmethod, "hmacmd5")){ /* TODO */ }
    else{ //hmacsha1
        if(!HMAC(EVP_sha1(), this->devicesecret, strlen(this->devicesecret), (unsigned char*)buf, strlen(buf), sign, &signlen)){
            return -1;
        }
    }
    for(int i = 0; i < signlen; ++i) { snprintf(buf + i*2, sizeof(buf), "%02x", sign[i]); }

    this->pwd = strdup(buf); //free(this->pwd) in aligatewaydevmqtt_free()
    if(!this->pwd){ l_e("strdup() failed"); return -1; }
    l_d("pwd: '%s'", this->pwd);

    getsetint(cs, "keepalive", this->keepalive);
    getsetint(cs, "qos", this->qos);

    //4. print all the params. or you can rewrite it aboved
    if(!this->m_show){ mqttlw("this->m_show() is NULL"); }
    else { this->m_show(this); }

    //5. init the mqtt
    mqtt_init(this);

    //6. topics list to subscribe
    this->cssubtopicsls = config_setting_lookup(cs, "subtopics");
    if(this->cssubtopicsls == NULL) { mqttle("config_setting_lookup(subtopics) failed"); return -1; }

    //7. tasks list to run after mqtt-connected
    this->cstasksls = config_setting_lookup(cs, "tasksls");
    if(this->cstasksls == NULL) { mqttle("config_setting_lookup(tasksls) failed"); return -1; }
    if(mqtt_tasksinit(this) < 0){ return -1; }

    //8. FSM for this(mqttobject)
    this->fsmtv.tv_sec = 1; this->fsmtv.tv_usec = 0;
    event_assign(&this->fsmev, base, -1, EV_TIMEOUT, alidirectdevmqtt_fsm_cb, this);
    event_add(&this->fsmev, &this->fsmtv);
    return 0;
}

int alidirectdevmqtt_free(void *vobj)
{
    alidirectdevmqtt_t *this = vobj;
    mosquitto_loop_stop(this->mosq, false);
    mosquitto_destroy(this->mosq);
    //if(this->anythingelse){ free(this->anythingelse); }
    if(this->pwd){ free((void *)this->pwd); }
    if(this->usr){ free((void *)this->usr); }
    if(this->clientid){ free((void *)this->clientid); }
    if(this->addr){ free((void *)this->addr); }
    if(this){ free(this); }
    return 0;
}


/*******************************************************************************
*                                  Subdevice                                  *
*******************************************************************************/
typedef enum _aligatewaysubdevopcua_state_t
{
    OPCUA_BASESTATE(ALIGATEWAYSUBDEV),
    ALIGATEWAYSUBDEVOPCUA_STATE_TOPOADDING,
    ALIGATEWAYSUBDEVOPCUA_STATE_LOGOUTED,
    ALIGATEWAYSUBDEVOPCUA_STATE_LOGINED,
    ALIGATEWAYSUBDEVOPCUA_STATE_END
}aligatewaysubdevopcua_state_t;

#define ALIGATEWAYSUBDEVOPCUA_BASEATTRIBUTES \
    OPCUA_BASEATTRIBUTES\
    ALIDEV_BASEATTRIBUTES\
    config_setting_t *cssubtopicsls;\
    const char *signmethod;\
    const char *clientid;\
    char sign[64];

#define ALIGATEWAYSUBDEVOPCUAGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    OPCUAGETBASESTRINGVAR(this, key, keylen, dptr);\
    ALIDEVGETBASESTRINGVAR(this, key, keylen, dptr);\
    if(!strncmp("SignMethod", key, keylen)){ return sprintf(dptr, "%s", this->signmethod); }\
    if(!strncmp("clientId", key, keylen)){ return sprintf(dptr, "%s", this->clientid); }\
    if(!strncmp("sign", key, keylen)){ return sprintf(dptr, "%s", this->sign); }\
} while (0)

typedef struct _aligatewaysubdevopcua_t
{
    ALIGATEWAYSUBDEVOPCUA_BASEATTRIBUTES
}aligatewaysubdevopcua_t;

static int32_t alidirectgatewaysubdevopcua_getvar(void *vthis, const char *key, int32_t keylen, char *dptr)
{
    aligatewaysubdevopcua_t *this = vthis;
    ALIGATEWAYSUBDEVOPCUAGETBASESTRINGVAR(this, key, keylen, dptr);
    return alidirectgatewaydevmqtt_getvar(this->vnorthptr, key, keylen, dptr);
}

static void aligatewaysubdevopcua_show(void *vthis)
{
    aligatewaysubdevopcua_t *this = vthis;
    opcuald("{");
    opcuald("  objtypestr: '%s'", this->objtypestr);
    opcuald("  name: '%s'", this->name);
    opcuald("  addr: '%s'", this->addr);
    opcuald("  port: %d", this->port);
    //opcuald("  usr: '%s'", this->usr);
    //opcuald("  pwd: '%s'", this->pwd);
    opcuald("}");
    return ;
}

static void aligatewaysubdevopcua_your_rcvhandle(void *vthis, char *topic, char *payload)
{
    aligatewaysubdevopcua_t *this = vthis;
    //TODO parse the payload of yours
}

static void aligatewaysubdevopcua_alink_rcvhandle(void *vthis, char *topic, char *payload)
{
    aligatewaysubdevopcua_t *this = vthis;
    //TODO parse the payload of alink
}

static void aligatewaysubdevopcua_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    aligatewaysubdevopcua_t *this = user_data;

    switch (this->state) {
        case ALIGATEWAYSUBDEVOPCUA_STATE_INIT:
            opcuald("trying to add topo");//TODO
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case ALIGATEWAYSUBDEVOPCUA_STATE_LOGOUTED:
            opcuald("trying to login");
            //订阅/发布子设备登录
            aligatewaydevmqtt_t *thisnorth = this->vnorthptr;
            int ret = -1;
            int mid = 0;
            char topic[256] = {0};
            //响应Topic：/ext/session/${productKey}/${deviceName}/combine/login_reply
            ret = thisnorth->m_strexpan(thisnorth, "/ext/session/{ProductKey}/{DeviceName}/combine/login_reply", topic);
            if(ret < 0){ break; }
            ret = thisnorth->m_sub(thisnorth, &mid, topic, 2);
            if(ret < 0){ break; }

            //请求Topic：/ext/session/${productKey}/${deviceName}/combine/login
            ret = thisnorth->m_strexpan(thisnorth, "/ext/session/{ProductKey}/{DeviceName}/combine/login", topic);
            if(ret < 0){ break; }

            char buf[4096] = {0};
            uint32_t signlen = 0;
            uint8_t sign[128] = {0};

            this->loginmillisecond = this->m_getmilliseconds();
            //hmac_md5(deviceSecret, clientIdal12345****&device1234deviceNamedevice1234productKeyal12345****timestamp1581417203000)
            ret = this->m_strexpan(this, "clientId{clientId}deviceName{DeviceName}productKey{ProductKey}timestamp{LoginMilliSecond}", buf);
            if(ret < 0){ break; }
            l_d("originpassword:'%s'", buf);

            if(!strcmp(this->signmethod, "hmacmd5")){ /* TODO */ }
            else{ //hmacsha1
                if(!HMAC(EVP_sha1(), this->devicesecret, strlen(this->devicesecret), (unsigned char*)buf, strlen(buf), sign, &signlen)){
                    break;
                }
            }
            for(int i = 0; i < signlen; ++i) { snprintf(this->sign + i*2, sizeof(this->sign), "%02x", sign[i]); }

            ret = this->m_strexpan(this, "{\"id\":\"{count}\",\"params\":{\"productKey\":\"{ProductKey}\",\"deviceName\":\"{DeviceName}\",\"clientId\":\"{clientId}\",\"timestamp\":\"{LoginMilliSecond}\",\"signMethod\":\"{SignMethod}\",\"sign\":\"{sign}\",\"cleanSession\":\"true\"}}", buf);
            if(ret < 0){ break; }

            ret = thisnorth->m_pub(thisnorth, &mid, topic, strlen(buf), buf, 2, false);
            if(ret < 0){ break; }

            this->state = ALIGATEWAYSUBDEVOPCUA_STATE_LOGINED;
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case ALIGATEWAYSUBDEVOPCUA_STATE_LOGINED:
            opcuald("logined, do STH???");
            //TODO do sth subscribe?
            this->fsmtv.tv_sec = 10; this->fsmtv.tv_usec = 0;
            break;

        case ALIGATEWAYSUBDEVOPCUA_STATE_RUNING:
            //opcuald("OPCUA runing, check and iterate your tasksls here");
            if(this->m_tasksrun){ this->m_tasksrun(this); }
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 10;
            break;

        default:
            opcualw("Unkown ALIGATEWAYSUBDEVOPCUA_STATE:%d", this->state);
            break;
    }

    event_add(&this->fsmev, &this->fsmtv); //redo
}

static void *aligatewaysubdevopcua_new(void)
{
    aligatewaysubdevopcua_t *this = malloc(sizeof(aligatewaysubdevopcua_t));
    if(this == NULL){ l_e("malloc() '%s'", strerror(errno)); return NULL; }
    memset(this, 0, sizeof(aligatewaysubdevopcua_t));
    return this;
}

static int aligatewaysubdevopcua_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    aligatewaysubdevopcua_t *this = vobj;

    //1. read the config from cs
    getsetstr(cs, "name", this->name);
    getsetstr(cs, "ProductKey", this->productkey);
    this->devicename = this->name;
    getsetstr(cs, "DeviceSecret", this->devicesecret);
    getsetstr(cs, "SignMethod", this->signmethod);

    char buf[256] = {0};

    snprintf(buf, sizeof(buf), "%s&%s", this->productkey, this->devicename);
    this->clientid = strdup(buf); //free(this->clientid) in aligatewaysubdevopcua_free()
    if(!this->clientid){ l_e("strdup() failed"); return -1; }
    l_d("clientid: '%s'", this->clientid);

    getsetstr(cs, "addr", this->addr);
    getsetint(cs, "port", this->port);
    //getsetstr(cs, "usr", this->usr);
    //getsetstr(cs, "pwd", this->pwd);

    //2. set all the methods to default
    opcua_set_default(this);

    //3. set some methods to yours rewrited
    this->m_getvar = alidirectgatewaysubdevopcua_getvar;
    //if(getfrom(cs, usealink){
        this->m_rcvhandle = aligatewaysubdevopcua_alink_rcvhandle;
    //}
    //else{ this->m_rcvhandle = aligatewaysubdevopcua_your_rcvhandle; }

    //this->m_onpub = your_rewrited_on_publisned_function;
    this->m_show = aligatewaysubdevopcua_show;

    //4. print all the params. or you can rewrite it aboved
    if(!this->m_show){ opcualw("this->m_show() is NULL"); }
    else { this->m_show(this); }

    //5. topics list to subscribe
    this->cssubtopicsls = config_setting_lookup(cs, "subtopics");
    if(this->cssubtopicsls == NULL) { opcuale("config_setting_lookup(subtopics) failed"); return -1; }

    //6. tasks list to run after subdev-login
    this->cstasksls = config_setting_lookup(cs, "tasksls");
    if(this->cstasksls == NULL) { opcuale("config_setting_lookup(tasksls) failed"); return -1; }
    if(opcua_tasksinit(this) < 0){ return -1; }

    //7. FSM for this(opcuaobject)
    this->fsmtv.tv_sec = 1; this->fsmtv.tv_usec = 0;
    event_assign(&this->fsmev, base, -1, EV_TIMEOUT, aligatewaysubdevopcua_fsm_cb, this);
    event_add(&this->fsmev, &this->fsmtv);

    return 0;
}

static int aligatewaysubdevopcua_free(void *vobj)
{
    aligatewaysubdevopcua_t *this = vobj;
    //if(this->anythingelse){ free(this->anythingelse); }
    if(this->clientid){ free((void *)this->clientid); }
    if(this){ free(this); }
    return 0;
}

static void *aligatewaysubdevmodbus_new(void)
{
}

static int aligatewaysubdevmodbus_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    return 0;
}

static int aligatewaysubdevmodbus_free(void *vobj)
{
    return 0;
}

class_t AliSubClazz[SUBOBJTYPE_SIZE] = {
    [SUBOBJTYPE_OPCUA] = {
        aligatewaysubdevopcua_new, aligatewaysubdevopcua_init, aligatewaysubdevopcua_free },
    [SUBOBJTYPE_MODBUS] = {
        aligatewaysubdevmodbus_new, aligatewaysubdevmodbus_init, aligatewaysubdevmodbus_free }
};


/*******************************************************************************
*                                   Gateway                                   *
*******************************************************************************/
static void aligatewaydevmqtt_your_rcvhandle(void *vthis, char *topic, char *payload)
{
    aligatewaydevmqtt_t *this = vthis;
    //TODO parse the payload of yours
    mqttld("'%s' '%s'", topic, payload);
}

static void aligatewaydevmqtt_alink_rcvhandle(void *vthis, char *topic, char *payload)
{
    aligatewaydevmqtt_t *this = vthis;
    //TODO parse the payload of alink
    mqttld("'%s' '%s'", topic, payload);
    if(strstr(topic, "topo/get_reply")){
        for (int i = 0; i < this->southlstot; ++i) {
            aligatewaysubdevopcua_t *subdev = (aligatewaysubdevopcua_t *)this->southarray[i];
            mqttld("name:%s", subdev->name);
            subdev->state = ALIGATEWAYSUBDEVOPCUA_STATE_LOGOUTED;
        }
    }
}

static void aligatewaydevmqtt_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    aligatewaydevmqtt_t *this = user_data;

    switch (this->state) {
        case ALIGATEWAYDEVMQTT_STATE_INIT:
        case ALIGATEWAYDEVMQTT_STATE_CONNECTING:
            rc = mosquitto_connect_async(this->mosq, this->addr, this->port, this->keepalive);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_connect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_connect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            mqttld("start to loop");
            mosquitto_loop_start(this->mosq);
            this->state = ALIGATEWAYDEVMQTT_STATE_RECONNECTING;
            this->fsmtv.tv_sec = 5; this->fsmtv.tv_usec = 0;
            break;

        case ALIGATEWAYDEVMQTT_STATE_DISCONNECTED:
        case ALIGATEWAYDEVMQTT_STATE_RECONNECTING:
            rc = mosquitto_reconnect_async(this->mosq);
            if (MOSQ_ERR_SUCCESS == rc) { mqttln("mosquitto_reconnect_async(%s,%d) rc%d", this->addr, this->port, rc); }
            else {
                mqttln("mosquitto_reconnect_async(%s,%d) failed:rc%d.'%s', reconnecting",
                        this->addr, this->port, rc, mosquitto_strerror(rc));
            }
            this->fsmtv.tv_sec = 10; this->fsmtv.tv_usec = 0;
            break;

        case ALIGATEWAYDEVMQTT_STATE_CONNECTED:
            if(!this->m_suball){ mqttle("this->m_suball() is NULL"); break; }
            this->m_suball(this);
            this->state = ALIGATEWAYDEVMQTT_STATE_TOPOGET;
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 1;
            break;

        case ALIGATEWAYDEVMQTT_STATE_TOPOGET:{
            //订阅/添加子设备topo关系
            int ret = -1;
            int mid = 0;
            char topogettopic[256] = {0};
            //响应Topic：/sys/{productKey}/{deviceName}/thing/topo/get_reply
            ret = this->m_strexpan(this, "/sys/{ProductKey}/{DeviceName}/thing/topo/get_reply", topogettopic);
            if(ret < 0){ break; }
            ret = this->m_sub(this, &mid, topogettopic, 2);
            if(ret < 0){ break; }

            //请求Topic：/sys/{productKey}/{deviceName}/thing/topo/get
            ret = this->m_strexpan(this, "/sys/{ProductKey}/{DeviceName}/thing/topo/get", topogettopic);
            if(ret < 0){ break; }

            char buf[10240] = {0};
            ret = this->m_strexpan(this, "{\"id\":\"{count}\",\"version\":\"1.0\",\"sys\":{\"ack\":1},\"params\":{},\"method\":\"thing.topo.get\"}", buf);
            if(ret < 0){ break; }
            ret = this->m_pub(this, &mid, topogettopic, strlen(buf), buf, 2, false);
            if(ret < 0){ break; }

            this->state = ALIGATEWAYDEVMQTT_STATE_RUNING;
            break;}

        case ALIGATEWAYDEVMQTT_STATE_RUNING:{
            //mqttld("MQTT runing, check and iterate your tasksls here");
            if(this->m_tasksrun){ this->m_tasksrun(this); }
            this->fsmtv.tv_sec = 0; this->fsmtv.tv_usec = 10;
            break;}

        default:
            mqttlw("Unkown ALIGATEWAYDEVMQTT_STATE:%d", this->state);
            break;
    }

    event_add(&this->fsmev, &this->fsmtv); //redo
}

void *aligatewaydevmqtt_new(void)
{
    aligatewaydevmqtt_t *this = malloc(sizeof(aligatewaydevmqtt_t));
    if(this == NULL){ l_e("malloc() '%s'", strerror(errno)); return NULL; }
    memset(this, 0, sizeof(aligatewaydevmqtt_t));
    return this;
}

int aligatewaydevmqtt_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    aligatewaydevmqtt_t *this = vobj;

    //1. set all the methods to default
    mqtt_set_default(this);

    //2. set some methods to yours rewrited
    this->m_getvar = alidirectgatewaydevmqtt_getvar;
    //if(getfrom(cs, usealink){
        this->m_rcvhandle = aligatewaydevmqtt_alink_rcvhandle;
    //}
    //else{ this->m_rcvhandle = aligatewaydevmqtt_your_rcvhandle; }

    //this->m_onpub = your_rewrited_on_publisned_function;
    //this->m_show = your_rewrited_show_function;

    //3. read the config from cs
    getsetstr(cs, "name", this->name);
    getsetstr(cs, "RegionId", this->regionid);
    getsetstr(cs, "DeviceId", this->deviceid);
    getsetstr(cs, "ProductKey", this->productkey);
    this->devicename = this->name;
    getsetstr(cs, "DeviceSecret", this->devicesecret);
    getsetint(cs, "Mode", this->mode);
    getsetstr(cs, "SignMethod", this->signmethod);

    char buf[256] = {0};

    int ret = this->m_strexpan(this, "{ProductKey}.iot-as-mqtt.{RegionId}.aliyuncs.com", buf);
    if(ret < 0){ return -1; }
    this->addr = strdup(buf); //free(this->addr) in aligatewaydevmqtt_free()
    if(!this->addr){ l_e("strdup() failed"); return -1; }
    l_d("addr: '%s'", this->addr);

    getsetint(cs, "port", this->port);
    getsetbool(cs, "clean_session", this->clean_session);

    ret = this->m_strexpan(this, "{DeviceId}|securemode={Mode},signmethod={SignMethod}|", buf);
    if(ret < 0){ return -1; }
    this->clientid = strdup(buf); //free(this->clientid) in aligatewaydevmqtt_free()
    if(!this->clientid){ l_e("strdup() failed"); return -1; }
    l_d("clientid: '%s'", this->clientid);

    ret = this->m_strexpan(this, "{DeviceName}&{ProductKey}", buf);
    if(ret < 0){ return -1; }
    this->usr = strdup(buf); //free(this->usr) in aligatewaydevmqtt_free()
    if(!this->usr){ l_e("strdup() failed"); return -1; }
    l_d("usr: '%s'", this->usr);

    //password
    this->loginmillisecond = this->m_getmilliseconds();
    ret = this->m_strexpan(this, "clientId{DeviceId}deviceName{DeviceName}productKey{ProductKey}", buf);
    if(ret < 0){ return -1; }
    l_d("originpassword:'%s'", buf);

    uint8_t sign[64] = {0};
    uint32_t signlen = 0;
    if(!strcmp(this->signmethod, "hmacmd5")){ /* TODO */ }
    else{ //hmacsha1
        if(!HMAC(EVP_sha1(), this->devicesecret, strlen(this->devicesecret), (unsigned char*)buf, strlen(buf), sign, &signlen)){
            return -1;
        }
    }
    for(int i = 0; i < signlen; ++i) { snprintf(buf + i*2, sizeof(buf), "%02x", sign[i]); }

    this->pwd = strdup(buf); //free(this->pwd) in aligatewaydevmqtt_free()
    if(!this->pwd){ l_e("strdup() failed"); return -1; }
    l_d("pwd: '%s'", this->pwd);

    getsetint(cs, "keepalive", this->keepalive);
    getsetint(cs, "qos", this->qos);

    //4. print all the params. or you can rewrite it aboved
    if(!this->m_show){ mqttlw("this->m_show() is NULL"); }
    else { this->m_show(this); }

    //5. init the mqtt
    mqtt_init(this);

    //6. topics list to subscribe
    this->cssubtopicsls = config_setting_lookup(cs, "subtopics");
    if(this->cssubtopicsls == NULL) { mqttle("config_setting_lookup(subtopics) failed"); return -1; }

    //7. tasks list to run after mqtt-connected
    this->cstasksls = config_setting_lookup(cs, "tasksls");
    if(this->cstasksls == NULL) { mqttle("config_setting_lookup(tasksls) failed"); return -1; }
    if(mqtt_tasksinit(this) < 0){ return -1; }

    //8. FSM for this(mqttobject)
    this->fsmtv.tv_sec = 1; this->fsmtv.tv_usec = 0;
    event_assign(&this->fsmev, base, -1, EV_TIMEOUT, aligatewaydevmqtt_fsm_cb, this);
    event_add(&this->fsmev, &this->fsmtv);


    //9. Gateway Subdevice
    config_setting_t *cssouthls = config_setting_lookup(cs, "southls");
    if(cssouthls == NULL) { mqttle("config_setting_lookup() failed"); return -1; }

    this->southlstot = config_setting_length(cssouthls);
    mqttld("southlstot = %d", this->southlstot);
    if(this->southlstot != 0){
        this->southarray = malloc(sizeof(void *)); //free(this->southarray) in aligatewaydevmqtt_free()
        if(this->southarray == NULL){ mqttle("malloc() %s", strerror(errno)); return -1; }
        memset(this->southarray, 0, sizeof(void *));
    }

    for (int i = 0; i < this->southlstot; ++i) {
        config_setting_t *cssouth = config_setting_get_elem(cssouthls, i);
        if(cssouth == NULL) { mqttle("config_setting_get_elem() failed"); return -1; }

        const char *objtypestr =NULL;
        getsetstr(cssouth, "objtype", objtypestr);
        int32_t objtype = subobjtype_getindex(objtypestr);
        if(invalidsubobjtype(objtype)){ mqttle("invalidsubobjtype(%d)", objtype); return -1; }

        //maybe i should define a subobj_t extanded from obj_t just only with an void *vnorthptr more
        obj_t *objptr = AliSubClazz[objtype].objnew(); //free(objptr) in aligatewaydevmqtt_free()
        if(objptr == NULL){ return -1; }
        objptr->objtypestr = objtypestr;
        objptr->objtype = objtype;
        objptr->vnorthptr = this;
        if(AliSubClazz[objtype].objinit(base, objptr, cssouth) < 0){ return -1; }
        this->southarray[i] = objptr;
    }
    return 0;
}

int aligatewaydevmqtt_free(void *vobj)
{
    aligatewaydevmqtt_t *this = vobj;
    mosquitto_loop_stop(this->mosq, false);
    mosquitto_destroy(this->mosq);
    //if(this->anythingelse){ free(this->anythingelse); }
    for (int i = 0; i < this->southlstot; ++i) {
        if(this->southarray[i]){ free(this->southarray[i]); }
    }
    if(this->southarray){ free(this->southarray); }
    if(this->pwd){ free((void *)this->pwd); }
    if(this->usr){ free((void *)this->usr); }
    if(this->clientid){ free((void *)this->clientid); }
    if(this->addr){ free((void *)this->addr); }
    if(this){ free(this); }
    return 0;
}
