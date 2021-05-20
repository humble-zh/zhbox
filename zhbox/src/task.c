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
#include <errno.h>
#include "l.h"
#include "lib.h"
#include "task.h"

static int tasks_run(void *vpthis, void *vobj)
{
    taskls_t *pthis = vpthis;
    if(!pthis){ l_e("Argv is NULL"); return -1; }

    task_t *atask = NULL;
    size_t atask_size = 0;

    if(pthis->ret == 0){ pthis->ret = cfulist_each_data(pthis->ls, (void **)&atask, &atask_size); }
    else{ pthis->ret = cfulist_next_data(pthis->ls, (void **)&atask, &atask_size); }
    if(pthis->ret == 0) { goto nexttask; }
    if(atask_size != sizeof(task_t)) { l_d("atask_size != sizeof(task_t)"); goto nexttask; }

    if(!atask->task_run){ l_e("atask->task_run() is NULL"); goto nexttask; }
    atask->task_run(atask, vobj);
    return 0;

nexttask:
    return -1;
}


#if 0
static int tasks_run(void *vmqttptr)
{
    l_d("undefined");
#if 0
    struct _mqtt_t *pthis = vmqttptr;
    int32_t now = time(NULL);
    config_setting_t *cstasksls = pthis->cstasksls;
    for(int i = 0; i < pthis->taskstot; ++i){
        if(now - pthis->taskslasttimels[i] < pthis->tasksintervalls[i]){ continue; }

        config_setting_t *cstask = config_setting_get_elem(pthis->cstasksls, i);
        if(!cstask){ le(pthis, "config_setting_get_elem() failed"); return -1; }

        //topic
        const char *origin = NULL;
        config_setting_lookup_string(cstask, "pubtopic", &origin);
        if(!origin){ le(pthis, "config_setting_lookup_string(pubtopic)"); continue; }

        char pubtopic[1024] = {0};
        if(!pthis->m_strexpan){ le(pthis, "pthis->m_strexpan() is NULL"); break; }
        int topiclen = pthis->m_strexpan(pthis, origin, pubtopic);
        if(topiclen <= 0){ le(pthis, "wrong pubtopic'%s'", origin); exit(1); }

        //payload
        const char *payloadfmt = NULL;
        config_setting_lookup_string(cstask, "payloadfmt", &payloadfmt);
        if(!payloadfmt) { le(pthis, "config_setting_lookup_string(payloadfmt)"); continue; }

        char payload[4096000] = {0};
        if(!pthis->m_strexpan){ le(pthis, "pthis->m_strexpan() is NULL"); break; }
        int payloadlen = pthis->m_strexpan(pthis, payloadfmt, payload);
        if(payloadlen <= 0){ le(pthis, "wrong payloadfmt'%s'", payloadfmt); exit(1); }

        if(!pthis->m_pub){ le(pthis, "pthis->m_pub() is NULL"); break; }
        pthis->m_pub(pthis, NULL, pubtopic, payloadlen, payload, 2, false);
        pthis->taskslasttimels[i] = now;
    }
#endif
    return 0;
}
#endif

void task_set_default(void *vpthis)
{
    struct _taskls_t *pthis = vpthis;
    if(!pthis){ l_e("Argv is NULL"); return ; }
    pthis->m_tasksrun = tasks_run;
}


int taskls_init(void *vpthis, config_setting_t *cstasksls, task_run_t task_run)
{
    struct _taskls_t *pthis = vpthis;
    if(!pthis){ l_e("Argv is NULL"); goto arg_err; }

    pthis->ls = cfulist_new();
    if(!pthis->ls){ l_e("cfulist_new() failed"); goto cfulist_new_ls_err; }

    for(int i = 0; i < config_setting_length(cstasksls); ++i){
        task_t *atask = malloc(sizeof(task_t));
        if(!atask){ l_e("malloc() %s", strerror(errno)); goto malloc_err; }
        memset(atask, 0, sizeof(task_t));

        config_setting_t *cstask = config_setting_get_elem(cstasksls, i);
        atask->task_run = task_run;
        cslkse(cstask, "type", atask->type);
        cslkie(cstask, "interval", atask->interval);
        cslkse(cstask, "pubtopic", atask->pubtopic);
        cslkse(cstask, "payloadprefix", atask->payloadprefix);
        cslkse(cstask, "payloadsuffix", atask->payloadsuffix);
        cslkse(cstask, "propertyfmt", atask->propertyfmt);

        atask->lsproperties = cfulist_new();
        if(!atask->lsproperties){ l_e("cfulist_new() failed"); goto cfulist_new_lsproperties_err; }

        config_setting_t *csproperites = config_setting_lookup(cstask, "properties");
        if(!csproperites) { l_e("config_setting_lookup(properties) failed"); goto cslk_properties_err; }

        for(int j = 0; j < config_setting_length(csproperites); ++j){
            const char *propertyname = config_setting_get_string_elem(csproperites, j);

            cfulist_push_data(atask->lsproperties, (void *)propertyname, strlen(propertyname)); //TODO check ret?
        }

        cfulist_push_data(pthis->ls, atask, sizeof(task_t)); //TODO check ret?
    }

    return 0;

cslk_properties_err:
cfulist_new_lsproperties_err:
cslk_err:
malloc_err:

cfulist_new_ls_err:
arg_err:
    return -1;
}

int taskls_destroy(void *vobj)
{
    //if(atask){ free(atask); atask = NULL; }
}

#if 0
int tasks_init(void *vpthis, config_setting_t *cstasksls)
{
    l_d("undefined");
#if 0
    struct _mqtt_t *pthis = vmqttptr;
    pthis->taskstot = config_setting_length(pthis->cstasksls);
    if(pthis->taskstot){
        pthis->taskslasttimels = malloc(sizeof(int32_t) * pthis->taskstot);
        if(!pthis->taskslasttimels){ le(pthis, "malloc() %s", strerror(errno)); return -1; }

        pthis->tasksintervalls = malloc(sizeof(int32_t) * pthis->taskstot);
        if(!pthis->tasksintervalls){ le(pthis, "malloc() %s", strerror(errno)); return -1; }
    }

    memset(pthis->taskslasttimels, 0, sizeof(int32_t) * pthis->taskstot);
    for(int i = 0; i < pthis->taskstot; ++i){
        config_setting_t *cstask = config_setting_get_elem(pthis->cstasksls, i);
        if(!cstask){ le(pthis, "config_setting_get_elem(%u) failed", i); return -1; }
        config_setting_lookup_int(cstask, "interval", &pthis->tasksintervalls[i]);
        ld(pthis, "taskls[%d].interval: %d", i, pthis->tasksintervalls[i]);
    }
#endif
    return 0;
}
#endif
