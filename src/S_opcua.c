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
#include "lib.h"
#include "S_opcua.h"
#include <open62541/client.h>
#include <open62541/client_config_default.h>

static int32_t opcua_getvar(void *vopcuaptr, const char *key, int32_t keylen, char *dptr)
{
    struct _opcua_t *this = vopcuaptr;
    OPCUAGETBASESTRINGVAR(this, key, keylen, dptr);
    return 0;
}

static void opcua_show(void *vopcuaptr)
{
    struct _opcua_t *this = vopcuaptr;
    opcuald("{");
    opcuald("  objtypestr: '%s'", this->objtypestr);
    opcuald("  name: '%s'", this->name);
    opcuald("  addr: '%s'", this->addr);
    opcuald("  port: %d", this->port);
    opcuald("  usr: '%s'", this->usr);
    opcuald("  pwd: '%s'", this->pwd);
    opcuald("}");
    return ;
}

static int opcua_tasksrun(void *vopcuaptr)
{
    struct _opcua_t *this = vopcuaptr;
    int32_t now = time(NULL);
    config_setting_t *cstasksls = this->cstasksls;
    for(int i = 0; i < this->taskstot; ++i){
        if(now - this->taskslasttimels[i] < this->tasksintervalls[i]){ continue; }

        config_setting_t *cstask = config_setting_get_elem(this->cstasksls, i);
        if(!cstask){ opcuale("config_setting_get_elem() failed"); return -1; }

        //topic
        const char *origin = NULL;
        config_setting_lookup_string(cstask, "pubtopic", &origin);
        if(!origin){ opcuale("config_setting_lookup_string(pubtopic)"); continue; }

        char pubtopic[1024] = {0};
        if(!this->m_strexpan){ opcuale("this->m_strexpan() is NULL"); break; }
        int topiclen = this->m_strexpan(this, origin, pubtopic);
        if(topiclen <= 0){ opcuale("wrong pubtopic'%s'", origin); exit(1); }

        //payload
        const char *payloadfmt = NULL;
        config_setting_lookup_string(cstask, "payloadfmt", &payloadfmt);
        if(!payloadfmt) { opcuale("config_setting_lookup_string(payloadfmt)"); continue; }

        char payload[4096000] = {0};
        if(!this->m_strexpan){ opcuale("this->m_strexpan() is NULL"); break; }
        int payloadlen = this->m_strexpan(this, payloadfmt, payload);
        if(payloadlen <= 0){ opcuale("wrong payloadfmt'%s'", payloadfmt); exit(1); }

        //if(!this->m_pub){ opcuale("this->m_pub() is NULL"); break; }
        //this->m_pub(this, NULL, pubtopic, payloadlen, payload, 2, false);
        this->taskslasttimels[i] = now;
    }
    return 0;
}

static void opcua_fsm_cb(evutil_socket_t fd, short event, void *user_data)
{
    int rc;
    struct _opcua_t *this = user_data;

    //TODO

    event_add(&this->fsmev, &this->fsmtv); //redo
}

int opcua_tasksinit(void *vopcuaptr)
{
    struct _opcua_t *this = vopcuaptr;
    this->taskstot = config_setting_length(this->cstasksls);
    if(this->taskstot){
        this->taskslasttimels = malloc(sizeof(int32_t) * this->taskstot);
        if(!this->taskslasttimels){ opcuale("malloc() %s", strerror(errno)); return -1; }

        this->tasksintervalls = malloc(sizeof(int32_t) * this->taskstot);
        if(!this->tasksintervalls){ opcuale("malloc() %s", strerror(errno)); return -1; }
    }

    memset(this->taskslasttimels, 0, sizeof(int32_t) * this->taskstot);
    for(int i = 0; i < this->taskstot; ++i){
        config_setting_t *cstask = config_setting_get_elem(this->cstasksls, i);
        if(!cstask){ opcuale("config_setting_get_elem(%u) failed", i); return -1; }
        config_setting_lookup_int(cstask, "interval", &this->tasksintervalls[i]);
        opcuald("taskls[%d].interval: %d", i, this->tasksintervalls[i]);
    }
    return 0;
}

void opcua_set_default(void *vopcuaptr)
{
    struct _opcua_t *this = vopcuaptr;
    obj_set_default(this);
    this->m_getvar = opcua_getvar;
    this->m_show = opcua_show;
    this->m_tasksrun = opcua_tasksrun;

    this->client = UA_Client_new();
    if (NULL == this->client) { opcuale("UA_Client_new() failed"); exit(1); }
    UA_ClientConfig_setDefault(UA_Client_getConfig(this->client));

    this->state = OPCUA_STATE_INIT;
}
