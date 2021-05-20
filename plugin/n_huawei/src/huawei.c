/* Filename: main.c
 * Description: <TODO>
 * Last modified: humble 2021-01-28 09:22:42
 */
#include <stdio.h>
#include <stdlib.h>
#include "huawei.h"

int huaweidirect_new(void)
{
    printf("huaweidirect_new\n");
    return 0;
}
int huaweidirect_init(struct event_base *base, void *vobj, config_setting_t *cs)
{
    printf("huaweidirect_init\n");
    return 0;
}
int huaweidirect_free(void *vobj)
{
    printf("huaweidirect_free\n");
    return 0;
}
