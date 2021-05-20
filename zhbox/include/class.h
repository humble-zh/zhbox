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
#ifndef __CLASS_H__
#define __CLASS_H__
#include <cfuhash.h>
#include <libconfig.h>
#include <event.h>

//#define ZHBOX_LIB_PATH "/etc/zhbox/lib/"

typedef void *(objnew_t)(void);
typedef int (objinit_t)(struct event_base *base, void *vobj, config_setting_t *cs);
typedef int (objfree_t)(void *vobj);

typedef struct _class_t
{
    objnew_t *objnew;
    objinit_t *objinit;
    objfree_t *objfree;
}class_t;

extern cfuhash_table_t *htclasses;
extern int classes_init(const char *plugindir);
extern int classes_destroy(void);
#endif /* ifndef __CLASS_H__ */
