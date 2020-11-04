#ifndef _XBOOK_WALLTIME_H
#define _XBOOK_WALLTIME_H

#include <sys/walltime.h>

extern walltime_t walltime;
void walltime_update_second();
void walltime_printf();
void walltime_init();
void sys_get_walltime(walltime_t *wt);
long walltime_make_timestamp(walltime_t *wt);

#endif   /* _XBOOK_WALLTIME_H */
