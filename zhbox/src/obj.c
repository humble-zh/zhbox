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

static int32_t obj_getvar(void *vpthis, const char *key, int32_t keylen, char *dptr)
{
    struct _obj_t *pthis = vpthis;
    if((!pthis) || (!key) || (!dptr)){ le(pthis, "Argv is NULL"); return 0; }
    OBJGETBASESTRINGVAR(pthis, key, keylen, dptr);
    return 0;
}

static int32_t obj_stringexpansion(void *vpthis, const char *str, char *dptr)
{
    struct _obj_t *pthis = vpthis;
    int finallen = 0;
    if((!pthis) || (!str) || (!dptr)){ le(pthis, "Argv is NULL"); return 0; }
    if(!pthis->m_getvar){ le(pthis, "pthis->m_getvar() is NULL"); return 0; }

    while(*str != '\0'){
        if(str[0] == '{' && isalpha(str[1])){
            str++;
            int32_t keylen = strchr(str, '}') - str;
            int32_t len = pthis->m_getvar(pthis, str, keylen, &dptr[finallen]);
            if(len < 0){ le(pthis, "sprintf() failed"); return 0; }
            finallen += len;
            str += (keylen + 1);
        }
        dptr[finallen++] = *str++;
    }
    dptr[finallen++] = '\0';
    return finallen;
}

static void obj_show(void *vpthis)
{
    struct _obj_t *pthis = vpthis;
    if(!pthis){ le(pthis, "Argv is NULL"); return ; }
    ld(pthis, "{");
    ld(pthis, "  objtype: '%s'", pthis->objtypestr);
    ld(pthis, "  name: '%s'", pthis->name);
    ld(pthis, "}");
    return ;
}

static int64_t obj_getmilliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t)tv.tv_sec)*1000UL + tv.tv_usec/1000;
}

void obj_set_default(void *vpthis)
{
    struct _obj_t *pthis = vpthis;
    if(!pthis){ le(pthis, "Argv is NULL"); return ; }
    pthis->m_getvar = obj_getvar;
    pthis->m_strexpan = obj_stringexpansion;
    pthis->m_show = obj_show;
    pthis->m_getmilliseconds = obj_getmilliseconds;

    taskls_t *taskls = (taskls_t *)&pthis->ls;
    task_set_default(taskls);
}
