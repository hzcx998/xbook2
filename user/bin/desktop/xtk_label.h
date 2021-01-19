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
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_LABEL);
    xtk_spirit_auto_size(spirit);
    return spirit;
}

static inline int xtk_label_length(xtk_spirit_t *label)
{
    if (!label)
        return -1;
    return strlen(label->text);
}

static inline int xtk_label_set_text(xtk_spirit_t *label, char *text)
{
    if (!label)
        return -1;
    return xtk_spirit_set_text(label, text);
}

static inline char *xtk_label_get_text(xtk_spirit_t *label)
{
    if (!label)
        return NULL;
    return label->text;
}

#endif /* _LIB_XTK_LABEL_H */