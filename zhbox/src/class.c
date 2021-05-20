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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glob.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "class.h"
#include "l.h"


cfuhash_table_t *htclasses = NULL;
static int32_t dlhandlestot = 0;
static void **dlhandles = NULL;

static int class_destroy(void)
{
    cfuhash_destroy(htclasses);
    return 0;
}
static int class_init(void *dlhandle, config_setting_t *cs)
{
    const char *tmp = NULL;

    class_t *aclass = malloc(sizeof(class_t));
    if(aclass == NULL){ l_e("malloc() %s", strerror(errno)); return -1; }
    memset(aclass, 0, sizeof(class_t));

    if(config_setting_lookup_string(cs, "new", &tmp)){ l_d("new: '%s' ok", tmp); }
    else{ l_e("No 'new' setting in configuration file."); return -1; }
    aclass->objnew = dlsym(dlhandle, tmp);
    if(!aclass->objnew){ l_e("dlsym(%s) failed:%s", tmp, dlerror()); return -1; }
    else{ l_e("dlsym(%s) ok", tmp); }

    if(config_setting_lookup_string(cs, "init", &tmp)){ l_d("init: '%s' ok", tmp); }
    else{ l_e("No 'init' setting in configuration file."); return -1; }
    aclass->objinit = dlsym(dlhandle, tmp);
    if(!aclass->objinit){ l_e("dlsym(%s) failed:%s", tmp, dlerror()); return -1; }
    else{ l_e("dlsym(%s) ok", tmp); }

    if(config_setting_lookup_string(cs, "free", &tmp)){ l_d("free: '%s' ok", tmp); }
    else{ l_e("No 'free' setting in configuration file."); return -1; }
    aclass->objfree = dlsym(dlhandle, tmp);
    if(!aclass->objfree){ l_e("dlsym(%s) failed:%s", tmp, dlerror()); return -1; }
    else{ l_e("dlsym(%s) ok", tmp); }

    if(config_setting_lookup_string(cs, "name", &tmp)){ l_d("name: '%s' ok", tmp); }
    else{ l_e("No 'name' setting in configuration file."); return -1; }

    if(cfuhash_exists_data(htclasses, tmp, strlen(tmp))){
         l_e("cfuhash_exists_data(htclasses, %s, %ld) already in use", tmp, strlen(tmp)); return -1; }

    int ret = cfuhash_put_data(htclasses, tmp, strlen(tmp), aclass, sizeof(class_t), NULL);
    if(0 == ret){ l_e("cfuhash_put_data(htclasses,%s,...) ->0 already in use", tmp); return -1; }
    else if(1 == ret){ l_d("cfuhash_put_data(htclasses,%s,...) ok", tmp); }
    else{ l_e("cfuhash_put_data(htclasses,%s,...) ->%d unknown", tmp, ret); return -1; }

    return 0;
}

static int dlhandles_destroy(void)
{
    int ret = 0;
    for (int i = 0; i < dlhandlestot; ++i) {
        if(dlhandles[i]){
            ret = dlclose(dlhandles[i]);
            if(ret){ l_e("dlclose() failed:%s", dlerror()); }
            else{ dlhandles[i] = NULL; }
        }
    }
    return 0;
}

static int dlhandle_init(int32_t index, const char *configfile)
{
    config_t cfg;
    config_init(&cfg);
    if(!config_read_file(&cfg, configfile)) {
       l_e("%s:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
       config_destroy(&cfg);
       return -1;
    }
    else { l_d("config_read_file(%s) ok", configfile); }

    const char *tmp = NULL;

    if(CONFIG_TRUE != config_lookup_string(&cfg, "libpath", &tmp)){ l_e("No 'libpath' in %s", configfile); goto cfglk_libpath_err; }
    else{ l_d("config_lookup_string 'libpath' '%s' ok", tmp); }


    dlhandles[index] = dlopen(tmp, RTLD_LAZY);
    if(!dlhandles[index]){ l_e("dlopen() failed:%s", dlerror()); goto dlopenerr; }
    else{ l_d("dlopen(%s) ok", tmp); }

    config_setting_t *csarray = config_lookup(&cfg, "clazz");
    if(!csarray){ l_e("No 'clazz' in %s", configfile); goto cfglk_clazz_err; }

    for(int i=0; i < config_setting_length(csarray); i++){
        config_setting_t *cs = config_setting_get_elem(csarray, i);
        if(!cs){ l_e("get 'clazz[%d]' in %s failed", i, configfile); goto cslk_elem_err; }
        if(class_init(dlhandles[index], cs) < 0){ goto class_initerr; }
    }

    config_destroy(&cfg);
    return 0;

class_initerr:
    class_destroy();
cslk_elem_err:
cfglk_clazz_err:
dlopenerr:
    dlhandles_destroy();
cfglk_libpath_err:
    config_destroy(&cfg);
    return -1;
}

static int errfunc_(const char *epath, int eerrno){ l_e("epath'%s' eerrno'%d' msg:%s", epath, eerrno, strerror(eerrno)); }

int classes_init(const char *plugindir)
{
    struct stat sb;
    if(stat(plugindir, &sb) != 0 || !(S_ISDIR(sb.st_mode))){ mkdir(plugindir, 0755);  }

    char path[PATH_MAX] = {0};
    int dirpathlen = snprintf(path, PATH_MAX, "%s", plugindir);
    if(dirpathlen <= 0){ l_e("Wrong plugindir'%s'", plugindir); goto patherr; }
    if(path[dirpathlen-1] != '/'){ path[dirpathlen++] = '/'; }
    int filepathlen = snprintf(path+dirpathlen, PATH_MAX-dirpathlen, "*.cfg");
    if(filepathlen <= 0){ l_e("Wrong pluginpath'%s'", path); goto patherr; }

    glob_t pglob;
    int ret = glob(path, 0,  errfunc_, &pglob);
    if(ret){ goto globerr; }
    l_d("glob('%s'...) ok", path);

    dlhandlestot = (int32_t)pglob.gl_pathc;
    dlhandles = malloc(sizeof(void *) * dlhandlestot);
    if(dlhandles == NULL){ l_e("malloc() %s", strerror(errno)); goto mallocerr; }
    memset(dlhandles, 0, sizeof(void *) * dlhandlestot);

    htclasses = cfuhash_new_with_initial_size(pglob.gl_pathc * 3);
    cfuhash_set_flag(htclasses, CFUHASH_FROZEN_UNTIL_GROWS);
    cfuhash_set_flag(htclasses, CFUHASH_FREE_DATA);

    for(int i = 0; i < pglob.gl_pathc; ++i){
        if(dlhandle_init(i, pglob.gl_pathv[i]) < 0){ goto dlhandle_init_err; }
    }

    globfree(&pglob);
    return 0;

dlhandle_init_err:
    classes_destroy();
mallocerr:
    globfree(&pglob); //TODO where to globfree?
globerr:
    if(!ret){}
    else if(ret == GLOB_NOSPACE){ l_e("glob() GLOB_NOSPACE, running out of memory"); }
    else if(ret == GLOB_ABORTED){ l_e("glob() GLOB_ABORTED, a read error"); }
    else if(ret == GLOB_NOMATCH){ l_e("glob() GLOB_NOMATCH, no found matches, please install some plugins"); }
patherr:
    return -1;
}


int classes_destroy(void)
{
    if(dlhandles){ free(dlhandles); dlhandles = NULL; }
    return 0;
}
