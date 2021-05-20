#ifndef __HUAWEI_H__
#define __HUAWEI_H__
#include <event.h>
#include <libconfig.h>

extern int huaweidirect_new(void);
extern int huaweidirect_init(struct event_base *base, void *vobj, config_setting_t *cs);
extern int huaweidirect_free(void *vobj);

#endif /* ifndef __HUAWEI_H__ */
