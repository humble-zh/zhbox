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
#ifndef __S_OPCUA_H__
#define __S_OPCUA_H__
#include <libconfig.h>
#include <open62541/client_config.h>
#include <event.h>
#include "obj.h"
#include "task.h"

#define OPCUA_BASESTATE(PREFIX) \
    OBJ_BASESTATE(PREFIX##OPCUA),\
    PREFIX##OPCUA##_STATE_CONNECTING,\
    PREFIX##OPCUA##_STATE_DISCONNECTED,\
    PREFIX##OPCUA##_STATE_CONNECTED,\
    PREFIX##OPCUA##_STATE_RECONNECTING,\
    PREFIX##OPCUA##_STATE_RUNING

typedef enum _opcua_state_t
{
    OPCUA_BASESTATE(),
    OPCUA_STATE_END
}opcua_state_t;


typedef void opcua_t;

#define OPCUAGETBASESTRINGVAR(this, key, keylen, dptr) do\
{\
    OBJGETBASESTRINGVAR(this, key, keylen, dptr);\
    if(!strncmp("addr", key, keylen)){ return sprintf(dptr, "%s", this->addr); }\
    if(!strncmp("port", key, keylen)){ return sprintf(dptr, "%d", this->port); }\
    if(!strncmp("usr", key, keylen)){ return sprintf(dptr, "%s", this->usr); }\
    if(!strncmp("pwd", key, keylen)){ return sprintf(dptr, "%s", this->pwd); }\
} while (0)

typedef int (*method_tasksrun_t)(void *vopcuaptr);
typedef void (*method_rcvhandle_t)(void *vopcuaptr, char *topic, char *payload);

#define OPCUA_BASEATTRIBUTES \
    OBJ_BASEATTRIBUTES\
    TASK_BASEATTRIBUTES\
    const char *addr;\
    int32_t port;\
    const char *usr;\
    const char *pwd;\
    UA_Client *client;\
    int8_t isconnected;\
    int32_t state;\
    struct timeval fsmtv;\
    struct event fsmev;\
    method_tasksrun_t  m_tasksrun;\
    method_rcvhandle_t m_rcvhandle;

struct _opcua_t
{
    OPCUA_BASEATTRIBUTES
};


#define opcuale(fmt, arg...) do\
{\
    objle("'%s:%d' " fmt, this->addr, this->port, ## arg);\
} while (0)

#define opcualw(fmt, arg...) do\
{\
    objlw("'%s:%d' " fmt, this->addr, this->port, ## arg);\
} while (0)

#define opcualn(fmt, arg...) do\
{\
    objln("'%s:%d' " fmt, this->addr, this->port, ## arg);\
} while (0)

#define opcuali(fmt, arg...) do\
{\
    objli("'%s:%d' " fmt, this->addr, this->port, ## arg);\
} while (0)

#define opcuald(fmt, arg...) do\
{\
    objld("'%s:%d' " fmt, this->addr, this->port, ## arg);\
} while (0)

extern int opcua_tasksinit(void *vopcuaptr);
extern void opcua_set_default(void *vopcuaptr);

#endif //__S_OPCUA_H__
