#ifndef _LIB_XTK_BOX_H
#define _LIB_XTK_BOX_H

#include <stddef.h>
#include <stdbool.h>
#include "xtk_spirit.h"

typedef enum {
    XTK_ORIENTATION_MIXED = 0,
    XTK_ORIENTATION_HORIZONTAL,
    XTK_ORIENTATION_VERTICAL,
} xtk_orientation_t;

typedef struct {
    xtk_spirit_t spirit;
    xtk_orientation_t orientation;
    int spacing;
} xtk_box_t;

#define XTK_BOX(spirit) ((xtk_box_t *)(spirit))

xtk_spirit_t *xtk_box_create(xtk_orientation_t orientation, int spacing);



#endif /* _LIB_XTK_BOX_H */