#ifndef _XBOOK_FSAL_FSTYPE_H
#define _XBOOK_FSAL_FSTYPE_H

#include <xbook/fsal.h>

#define FSAL_FSTYPE_DEFAULT "fat32"

int fstype_register(fsal_t *fsal);
int fstype_unregister(fsal_t *fsal);
fsal_t *fstype_find(char *name);
int fstype_init();

#endif  /* _XBOOK_FSAL_FSTYPE_H */