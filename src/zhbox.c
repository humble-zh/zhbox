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
#include "l.h"
#include "lib.h"
#include "zhbox.h"
#include "mqttgeneral.h"
#include "mqttali.h"

static cloud_t cloudls[CLOUDSIZE] = {
    [CLOUDGENERAL] = { mqttgeneral_new, mqttgeneral_init, mqttgeneral_free },
    [CLOUDALI]     = { mqttali_new, mqttali_init, mqttali_free }
};

zhbox_t zhbox;

int zhbox_init(struct event_base *base)
{
    config_init(&zhbox.cfg);
    if(!config_read_file(&zhbox.cfg, "./zhbox.cfg")) {
        l_e("%s:%d - %s", config_error_file(&zhbox.cfg), config_error_line(&zhbox.cfg), config_error_text(&zhbox.cfg));
        config_destroy(&zhbox.cfg);
        return -1;
    }

    getcfgbool(&zhbox.cfg, "enable", zhbox.enable);
    if(zhbox.enable != 1){ return -1; }
    getcfgint(&zhbox.cfg, "loglevel", zhbox.loglevel);
    setlogmask(LOG_UPTO(zhbox.loglevel));

    config_setting_t *csnorthls = config_lookup(&zhbox.cfg, "northls");
    if(csnorthls == NULL) { l_e("config_lookup() failed"); return -1; }

    zhbox.northlstot = config_setting_length(csnorthls);
    l_d("zhbox.northlstot = %d", zhbox.northlstot);
    if(zhbox.northlstot != 0){
        zhbox.northarray = (mqtt_t **)malloc(sizeof(void *));
        if(zhbox.northarray == NULL){ l_e("malloc() %s", strerror(errno)); return -1; }
        memset(zhbox.northarray, 0, sizeof(void *));
    }

    for (int i = 0; i < zhbox.northlstot; ++i) {
        config_setting_t *csnorth = config_setting_get_elem(csnorthls, i);
        if(csnorth == NULL) { l_e("config_setting_get_elem() failed"); return -1; }

        int32_t cloudid = 0;
        getsetint(csnorth, "cloud", cloudid);
        if(invalidcloudid(cloudid)){ l_e("invalidcloudid(%d)", cloudid); return -1; }

        zhbox.northarray[i] = cloudls[cloudid].mqttnew();
        if(zhbox.northarray[i] == NULL){ return -1; }
        zhbox.northarray[i]->cloudid = cloudid;
        if(cloudls[cloudid].mqttinit(base, zhbox.northarray[i], csnorth) < 0){ return -1; }
    }

    return 0;
}

int zhbox_destory(void)
{
    for (int i = 0; i < zhbox.northlstot; ++i) {
        cloudls[zhbox.northarray[i]->cloudid].mqttfree(zhbox.northarray[i]);
    }
    if(zhbox.northarray){ free(zhbox.northarray); zhbox.northarray = NULL; }
    config_destroy(&zhbox.cfg);
    return 0;
}
