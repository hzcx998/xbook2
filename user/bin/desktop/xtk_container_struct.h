#ifndef _LIB_XTK_CONTAINER_STRUCT_H
#define _LIB_XTK_CONTAINER_STRUCT_H

#include <sys/list.h>

typedef enum {
    XTK_CONTAINER_SINGAL = 0,
    XTK_CONTAINER_MULTI,
} xtk_container_type_t;

/* 容器记录的是精灵的信息 */
typedef struct {
    list_t children_list;       // 子容器链表
    xtk_container_type_t type;
    void *spirit;               // 容器对应的精灵
} xtk_container_t;

#endif /* _LIB_XTK_CONTAINER_STRUCT_H */