#ifndef _LIB_VIEW_H
#define _LIB_VIEW_H

#include <stdint.h>
#include "libview_bitmap.h"
#include "libview_msg.h"
#include "libview_shape.h"

/* 视图类型 */
enum view_type {
    VIEW_TYPE_FIXED      = 0,
    VIEW_TYPE_WINDOW,
    VIEW_TYPE_FLOAT,
};

#endif  /* _LIB_VIEW_H */