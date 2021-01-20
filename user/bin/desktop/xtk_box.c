#include "xtk_box.h"
#include "xtk_container.h"
#include <stdlib.h>

xtk_spirit_t *xtk_box_create(xtk_orientation_t orientation, int spacing)
{
    xtk_box_t *box = malloc(sizeof(xtk_box_t));
    if (!box)
        return NULL;
    xtk_spirit_t *spirit = &box->spirit;
    box->orientation = orientation;
    box->spacing = spacing;
    
    xtk_spirit_init(spirit, 0, 0, 0, 0);
    xtk_spirit_set_type(spirit, XTK_SPIRIT_TYPE_BOX);
    spirit->style.align = XTK_ALIGN_CENTER;
    spirit->container = xtk_container_create(XTK_CONTAINER_MULTI, spirit);
    if (!spirit->container) {
        free(box);
        return NULL;
    }
    return spirit;
}