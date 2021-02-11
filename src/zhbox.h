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
#ifndef __ZHBOX_H__
#define __ZHBOX_H__
#include <sys/types.h>
#include <libconfig.h>
#include <event.h>
#include "mqtt.h"

typedef enum _cloudid_t{
    CLOUDGENERAL = 0,
    CLOUDALI,
    CLOUDSIZE
}cloudid_t;
#define invalidcloudid(id) ((id) < 0 || (id) >= CLOUDSIZE)

typedef mqtt_t *(mqttnew_t)(void);
typedef int (mqttinit_t)(struct event_base *base, mqtt_t *, config_setting_t *);
typedef int (mqttfree_t)(mqtt_t *);

typedef struct _cloud_t{
    mqttnew_t *mqttnew;
    mqttinit_t *mqttinit;
    mqttfree_t *mqttfree;
}cloud_t;

typedef struct _zhbox_t{
    config_t cfg;
    int32_t enable;
    int32_t loglevel;
    int8_t northlstot;
    mqtt_t **northarray;
}zhbox_t;

extern int zhbox_init(struct event_base *base, const char *configfile);
extern int zhbox_destory(void);
#endif //__ZHBOX_H__
