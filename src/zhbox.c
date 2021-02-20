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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "l.h"
#include "lib.h"
#include "zhbox.h"

zhbox_t zhbox;

int zhbox_init(struct event_base *base, const char *configfile, const char *includedir)
{
    struct stat sb;
    if (stat(includedir, &sb) != 0 || !(S_ISDIR(sb.st_mode))) { mkdir(includedir, 0755); }

    config_init(&zhbox.cfg);
    config_set_include_dir(&zhbox.cfg, includedir);

    if(!config_read_file(&zhbox.cfg, configfile)) {
        l_e("%s:%d - %s", config_error_file(&zhbox.cfg), config_error_line(&zhbox.cfg), config_error_text(&zhbox.cfg));
        config_destroy(&zhbox.cfg);
        return -1;
    }

    getcfgint(&zhbox.cfg, "loglevel", zhbox.loglevel);
    setlogmask(LOG_UPTO(zhbox.loglevel));

    //North
    config_setting_t *csnorthls = config_lookup(&zhbox.cfg, "northls");
    if(csnorthls == NULL) { l_e("config_lookup() failed"); return -1; }

    zhbox.northlstot = config_setting_length(csnorthls);
    l_d("zhbox.northlstot = %d", zhbox.northlstot);
    if(zhbox.northlstot != 0){
        zhbox.northarray = (obj_t **)malloc(sizeof(void *));
        if(zhbox.northarray == NULL){ l_e("malloc() %s", strerror(errno)); return -1; }
        memset(zhbox.northarray, 0, sizeof(void *));
    }

    for (int i = 0; i < zhbox.northlstot; ++i) {
        config_setting_t *csnorth = config_setting_get_elem(csnorthls, i);
        if(csnorth == NULL) { l_e("config_setting_get_elem() failed"); return -1; }

        const char *objtypestr =NULL;
        getsetstr(csnorth, "objtype", objtypestr);
        int32_t objtype = objtype_getindex(objtypestr);
        if(invalidobjtype(objtype)){ l_e("invalidobjtype(%d)", objtype); return -1; }

        obj_t *objptr = Clazz[objtype].objnew();
        if(objptr == NULL){ return -1; }
        objptr->objtypestr = objtypestr;
        if(Clazz[objtype].objinit(base, objptr, csnorth) < 0){ return -1; }
        zhbox.northarray[i] = objptr;
    }

    return 0;
}

int zhbox_destory(void)
{
    for (int i = 0; i < zhbox.northlstot; ++i) {
        obj_t *this = zhbox.northarray[i];
        int32_t objtype = objtype_getindex(this->objtypestr);
        if(invalidobjtype(objtype)){ l_e("invalidobjtype(%d)", objtype); }
        Clazz[objtype].objfree(zhbox.northarray[i]);
    }
    if(zhbox.northarray){ free(zhbox.northarray); zhbox.northarray = NULL; }
    config_destroy(&zhbox.cfg);
    return 0;
}
