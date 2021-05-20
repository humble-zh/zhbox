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
#ifndef __LIB_H__
#define __LIB_H__
#include <libconfig.h>

#define cfglkir(cfg, key, dptr) \
    do{\
        if(config_lookup_int(cfg, key, &dptr)){ l_d("%s: %d ok", key, dptr);} \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)

#define cfglkbr(cfg, key, dptr) \
    do{\
        if(config_lookup_bool(cfg, key, &dptr)){ l_d("%s: %d ok", key, dptr);} \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)

#define cfglksr(cfg, key, dptr) \
    do{\
        if(config_lookup_string(cfg, key, &dptr)){ l_d("%s: '%s' ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)


//getsetstr(setting, key, dptr)
#define cslksr(setting, key, dptr) \
    do{\
        if(config_setting_lookup_string(setting, key, &dptr)){ l_d("%s: '%s' ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)

//getsetint(setting, key, dptr)
#define cslkir(setting, key, dptr) \
    do{\
        if(config_setting_lookup_int(setting, key, &dptr)){ l_d("%s: %d ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)

//getsetbool(setting, key, dptr)
#define cslkbr(setting, key, dptr) \
    do{\
        if(config_setting_lookup_bool(setting, key, &dptr)){ l_d("%s: %d ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);return -1; } \
    }while(0)


#define cslkse(setting, key, dptr) \
    do{\
        if(config_setting_lookup_string(setting, key, &dptr)){ l_d("%s: '%s' ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);goto cslk_err; } \
    }while(0)

#define cslkie(setting, key, dptr) \
    do{\
        if(config_setting_lookup_int(setting, key, &dptr)){ l_d("%s: %d ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);goto cslk_err; } \
    }while(0)

#define cslkbe(setting, key, dptr) \
    do{\
        if(config_setting_lookup_bool(setting, key, &dptr)){ l_d("%s: %d ok", key, dptr); } \
        else{ l_e("No '%s' setting in configuration file.", key);goto cslk_err; } \
    }while(0)
#endif //__LIB_H__
