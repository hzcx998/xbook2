#ifndef _LIB_XTK_LABEL_H
#define _LIB_XTK_LABEL_H

#include <stddef.h>
#include <string.h>
#include "xtk_spirit.h"

static inline xtk_spirit_t *xtk_label_create(char *text)
{
    xtk_spirit_t *spirit = xtk_spirit_create(0, 0, 0, 0);
    if (!spirit)
        return NULL;
    spirit->style.background_color = UVIEW_WHITE;
    spirit->style.align = XTK_ALIGN_CENTER;
    xtk_spirit_set_text(spirit, text);
    xtk_spirit_auto_size(spirit);
    return spirit;
}

static inline int xtk_label_length(xtk_spirit_t *label)
{
    if (!label)
        return -1;
    return strlen(label->text);
}

#endif /* _LIB_XTK_LABEL_H */