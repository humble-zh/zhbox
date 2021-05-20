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
#ifndef __TASK_H__
#define __TASK_H__
#include <libconfig.h>
#include <sys/types.h>
#include <cfulist.h>

typedef struct _task_t task_t;
typedef int (*task_run_t)(void *vpthis, void *vobj);
struct _task_t
{
    const char *type;
    task_run_t task_run;
    int32_t lasttime;
    int32_t interval;
    const char *pubtopic;
    const char *payloadprefix;
    const char *payloadsuffix;
    const char *propertyfmt;
    cfulist_t *lsproperties;
};


typedef int (*method_tasksrun_t)(void *vmqttptr, void *vobj);


#define TASKLS_BASEATTRIBUTES \
    cfulist_t *ls;\
    int32_t ret;\
    method_tasksrun_t     m_tasksrun;
/*
    int32_t taskstot;\
    int32_t *taskslasttimels;\
    int32_t *tasksintervalls;\
*/

typedef struct _taskls_t
{
    TASKLS_BASEATTRIBUTES
}taskls_t;

extern void task_set_default(void *vpthis);
//extern int tasks_init(void *vmqttptr);
extern int taskls_init(void *vpthis, config_setting_t *cstasksls, task_run_t task_run);

#endif //__TASK_H__
