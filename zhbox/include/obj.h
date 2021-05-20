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
#ifndef __OBJ_H__
#define __OBJ_H__
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <libconfig.h>
#include <event.h>
#include <syslog.h>
#include "task.h"


#define OBJ_BASESTATE(PREFIX) \
    PREFIX##_STATE_INIT = 0

typedef int (*method_getvar_t)(void *vpthis, const char *key, int32_t keylen, char *dptr);
typedef int (*method_stringexpansion_t)(void *vpthis, const char *str, char *dptr);
typedef void (*method_show_t)(void *vpthis);
typedef int64_t (*method_getmilliseconds_t)(void);

#define OBJ_BASEATTRIBUTES \
    const char *objtypestr;\
    int8_t objtype;\
    void *vpclass;\
    const char *name;\
    void *vnorthptr;\
    method_getvar_t          m_getvar;\
    method_stringexpansion_t m_strexpan;\
    method_show_t            m_show;\
    method_getmilliseconds_t m_getmilliseconds;\
    TASKLS_BASEATTRIBUTES

#define OBJGETBASESTRINGVAR(pthis, key, keylen, dptr) do\
{\
    if(!strncmp("objtype", key, keylen)){ return sprintf(dptr, "%s", pthis->objtypestr); }\
    if(!strncmp("name", key, keylen)){ return sprintf(dptr, "%s", pthis->name); }\
    if(!strncmp("timestamp", key, keylen)){ return sprintf(dptr, "%d", (int32_t)time(NULL)); }\
    if(!strncmp("milliseconds", key, keylen)){ return sprintf(dptr, "%" PRId64, pthis->m_getmilliseconds()); }\
} while (0)

typedef struct _obj_t
{
    OBJ_BASEATTRIBUTES
}obj_t;

#define le(pthis, fmt, arg...) do\
{\
    fprintf(stderr, "%s:%d %s " fmt "\n", __FUNCTION__, __LINE__, pthis->name, ## arg);\
    syslog(LOG_ERR, "%s:%d %s " fmt, __FUNCTION__, __LINE__, pthis->name, ## arg);\
} while (0)

#define lw(pthis, fmt, arg...) do\
{\
    printf("%s:%d %s " fmt "\n", __FUNCTION__, __LINE__, pthis->name, ## arg);\
    syslog(LOG_WARNING, "%s:%d %s " fmt, __FUNCTION__, __LINE__, pthis->name, ## arg);\
} while (0)

#define ln(pthis, fmt, arg...) do\
{\
    printf("%s:%d %s " fmt "\n", __FUNCTION__, __LINE__, pthis->name, ## arg);\
    syslog(LOG_NOTICE, "%s:%d %s " fmt, __FUNCTION__, __LINE__, pthis->name, ## arg);\
} while (0)

#define li(pthis, fmt, arg...) do\
{\
    printf("%s:%d %s " fmt "\n", __FUNCTION__, __LINE__, pthis->name, ## arg);\
    syslog(LOG_INFO, "%s:%d %s " fmt, __FUNCTION__, __LINE__, pthis->name, ## arg);\
} while (0)

#define ld(pthis, fmt, arg...) do\
{\
    printf("%s:%d %s " fmt "\n", __FUNCTION__, __LINE__, pthis->name, ## arg);\
    syslog(LOG_DEBUG, "%s:%d %s " fmt, __FUNCTION__, __LINE__, pthis->name, ## arg);\
} while (0)


extern void obj_set_default(void *vpthis);

#endif //__OBJ_H__
