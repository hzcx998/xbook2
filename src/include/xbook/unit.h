#ifndef _XBOOK_UNIT_H
#define _XBOOK_UNIT_H

#include "list.h"

typedef struct unit {
    list_t list;            /* unit list */
    char *name;             /* name */
    char *version;          /* version */
    int (*login)(void);    /* login function */
    void (*logout)(void);   /* logout function */
} unit_t;

#define IMPORT_UNIT(unit)   extern unit_t unit

unit_t *unit_find(char *name);
int unit_login(unit_t *unit);
int unit_logout(char *name);

void init_unit();

#endif /* _XBOOK_UNIT_H */
