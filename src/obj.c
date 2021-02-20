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
#include <time.h>
#include <ctype.h>
#include "obj.h"
#include "N_ali.h"
#include "N_general.h"

class_t Clazz[OBJTYPE_SIZE] = {
    [OBJTYPE_GENERALDIRECTDEVMQTT]  = { generaldirectdevmqtt_new, generaldirectdevmqtt_init, generaldirectdevmqtt_free },
    [OBJTYPE_GENERALGATEWAYDEVMQTT] = { generalgatewaydevmqtt_new, generalgatewaydevmqtt_init, generalgatewaydevmqtt_free },
    //[OBJTYPE_GENERALGATEWAYSUBDEV]  = { generalgatewaysubdev_new, generalgatewaysubdev_init, generalgatewaysubdev_free},
    [OBJTYPE_ALIDIRECTDEVMQTT]  = { alidirectdevmqtt_new, alidirectdevmqtt_init, alidirectdevmqtt_free },
    [OBJTYPE_ALIGATEWAYDEVMQTT] = { aligatewaydevmqtt_new, aligatewaydevmqtt_init, aligatewaydevmqtt_free }
    //[OBJTYPE_ALIGATEWAYSUBDEV]  = { aligatewaysubdev_new, aligatewaysubdev_init, aligatewaysubdev_free}
};

int32_t objtype_getindex(const char *key)
{
    if(!strcmp(key, "generaldirectdevmqtt")){ return OBJTYPE_GENERALDIRECTDEVMQTT; }
    if(!strcmp(key, "generalgatewaydevmqtt")){ return OBJTYPE_GENERALGATEWAYDEVMQTT; }
    //if(!strcmp(key, "generalgatewaysubdev")){ return OBJTYPE_GENERALGATEWAYSUBDEV; }
    if(!strcmp(key, "alidirectdevmqtt")){ return OBJTYPE_ALIDIRECTDEVMQTT; }
    if(!strcmp(key, "aligatewaydevmqtt")){ return OBJTYPE_ALIGATEWAYDEVMQTT; }
    //if(!strcmp(key, "aligatewaysubdev")){ return OBJTYPE_ALIGATEWAYSUBDEV; }
    return -1;
}

int32_t subobjtype_getindex(const char *key)
{
    if(!strcmp(key, "opcua")){ return SUBOBJTYPE_OPCUA; }
    if(!strcmp(key, "modbus")){ return SUBOBJTYPE_MODBUS; }
    return -1;
}

static int32_t obj_getvar(void *vthis, const char *key, int32_t keylen, char *dptr)
{
    struct _obj_t *this = vthis;
    OBJGETBASESTRINGVAR(this, key, keylen, dptr);
    return 0;
}

static int32_t obj_stringexpansion(void *vthis, const char *str, char *dptr)
{
    struct _obj_t *this = vthis;
    int finallen = 0;
    if((!this) || (!str) || (!dptr)){ objle("Argv is NULL"); return 0; }
    if(!this->m_getvar){ objle("this->m_getvar() is NULL"); return 0; }

    while(*str != '\0'){
        if(str[0] == '{' && isalpha(str[1])){
            str++;
            int32_t keylen = strchr(str, '}') - str;
            int32_t len = this->m_getvar(this, str, keylen, &dptr[finallen]);
            if(len < 0){ objle("sprintf() failed"); return 0; }
            finallen += len;
            str += (keylen + 1);
        }
        dptr[finallen++] = *str++;
    }
    dptr[finallen++] = '\0';
    return finallen;
}

static void obj_show(void *vthis)
{
    struct _obj_t *this = vthis;
    objld("{");
    objld("  objtype: '%s'", this->objtypestr);
    objld("  name: '%s'", this->name);
    objld("}");
    return ;
}

static int64_t obj_getmilliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t)tv.tv_sec)*1000UL + tv.tv_usec/1000;
}

void obj_set_default(void *vthis)
{
    struct _obj_t *this = vthis;
    this->m_getvar = obj_getvar;
    this->m_strexpan = obj_stringexpansion;
    this->m_show = obj_show;
    this->m_getmilliseconds = obj_getmilliseconds;
}
