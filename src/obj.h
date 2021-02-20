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
#include <sys/types.h>
#include <libconfig.h>
#include <event.h>
#include "l.h"

//class_t
typedef void *(objnew_t)(void);
typedef int (objinit_t)(struct event_base *base, void *, config_setting_t *);
typedef int (objfree_t)(void *);

typedef struct _class_t
{
    objnew_t *objnew;
    objinit_t *objinit;
    objfree_t *objfree;
}class_t;

typedef enum _objtype_t
{
    OBJTYPE_GENERALDIRECTDEVMQTT = 0,
    OBJTYPE_GENERALGATEWAYDEVMQTT,
    OBJTYPE_GENERALGATEWAYSUBDEV,
    OBJTYPE_ALIDIRECTDEVMQTT,
    OBJTYPE_ALIGATEWAYDEVMQTT,
    OBJTYPE_ALIGATEWAYSUBDEV,
    OBJTYPE_SIZE
}objtype_t;
#define invalidobjtype(id) ((id) < 0 || (id) >= OBJTYPE_SIZE)

extern class_t Clazz[OBJTYPE_SIZE];
extern int32_t objtype_getindex(const char *key);

//subobj class
typedef enum _subobjtype_t
{
    SUBOBJTYPE_OPCUA = 0,
    SUBOBJTYPE_MODBUS,
    SUBOBJTYPE_SIZE
}subobjtype_t;
#define invalidsubobjtype(id) ((id) < 0 || (id) >= SUBOBJTYPE_SIZE)
extern int32_t subobjtype_getindex(const char *key);


//obj_t
#define OBJ_BASESTATE(PREFIX) \
    PREFIX##_STATE_INIT = 0

typedef int (*method_getvar_t)(void *vthis, const char *key, int32_t keylen, char *dptr);
typedef int (*method_stringexpansion_t)(void *vthis, const char *str, char *dptr);
typedef void (*method_show_t)(void *vthis);
typedef int64_t (*method_getmilliseconds_t)(void);

#define OBJ_BASEATTRIBUTES \
    const char *objtypestr;\
    int8_t objtype;\
    const char *name;\
    void *vnorthptr;\
    method_getvar_t          m_getvar;\
    method_stringexpansion_t m_strexpan;\
    method_show_t            m_show;\
    method_getmilliseconds_t m_getmilliseconds;

#define OBJGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    if(!strncmp("objtype", key, keylen)){ return sprintf(dptr, "%s", this->objtypestr); }\
    if(!strncmp("name", key, keylen)){ return sprintf(dptr, "%s", this->name); }\
    if(!strncmp("timestamp", key, keylen)){ return sprintf(dptr, "%d", (int32_t)time(NULL)); }\
    if(!strncmp("milliseconds", key, keylen)){ return sprintf(dptr, "%ld", this->m_getmilliseconds()); }\
} while (0)

typedef struct _obj_t
{
    OBJ_BASEATTRIBUTES
}obj_t;

#define objle(fmt, arg...) do\
{\
    l_e("'%s' " fmt, this->name, ## arg);\
} while (0)

#define objlw(fmt, arg...) do\
{\
    l_w("'%s' " fmt, this->name, ## arg);\
} while (0)

#define objln(fmt, arg...) do\
{\
    l_n("'%s' " fmt, this->name, ## arg);\
} while (0)

#define objli(fmt, arg...) do\
{\
    l_i("'%s' " fmt, this->name, ## arg);\
} while (0)

#define objld(fmt, arg...) do\
{\
    l_d("'%s' " fmt, this->name, ## arg);\
} while (0)

extern void obj_set_default(void *vthis);

#endif //__OBJ_H__
