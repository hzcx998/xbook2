#ifndef _FILESRV_CORE_FSTYPE_H
#define _FILESRV_CORE_FSTYPE_H

#include <fsal/fsal.h>

int fstype_register(fsal_t *fsal);
int fstype_unregister(fsal_t *fsal);
fsal_t *fstype_find(char *name);
int fstype_init();

#endif  /* _FILESRV_CORE_FSTYPE_H */