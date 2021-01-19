#ifndef _LIB_XTK_CONTAINER_H
#define _LIB_XTK_CONTAINER_H

#include "xtk_container_struct.h"
#include "xtk_spirit.h"

#define XTK_CONTAINER(spirit) ((xtk_container_t *)((spirit)->container))

xtk_container_t *xtk_container_create(xtk_container_type_t type, xtk_spirit_t *spirit);
int xtk_container_destroy(xtk_container_t *container);
int xtk_container_add(xtk_container_t *container, xtk_spirit_t *spirit);
int xtk_container_remove(xtk_container_t *container, xtk_spirit_t *spirit);

#endif /* _LIB_XTK_CONTAINER_H */