#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/layer.h>
#include <dwin/workstation.h>
#include <dwin/hal.h>

dwin_layer_t *dwin_layer_create(uint32_t width, uint32_t height)
{
    dwin_layer_t *layer = dwin_malloc(sizeof(dwin_layer_t));
    if (layer == NULL)
    {
        return NULL;
    }
    layer->buffer = dwin_malloc(DWIN_LAYER_BUF_SZ(width, height));
    if (layer->buffer == NULL)
    {
        dwin_free(layer);
        return NULL;
    }

    layer->x = -1;
    layer->y = -1;
    layer->z = -1;

    layer->width = width;
    layer->height = height;
    layer->workstation = NULL;
    layer->priority = DWIN_LAYER_PRIO_DESKTOP;
    layer->flags = 0;

    layer->id = dwin_layer_alloc_id();
    if (layer->id == -1)
    {
        dwin_free(layer->buffer);
        dwin_free(layer);
        return NULL;
    }

    list_init(&layer->list);
    list_init(&layer->global_list);

    return layer;
}

/**
 * NOTICE: layer should not on the workstatin
 */
int dwin_layer_destroy(dwin_layer_t *layer)
{
    if (layer == NULL)
    {
        return -1;
    }
    
    list_del_init(&layer->list);
    list_del_init(&layer->global_list);

    dwin_layer_free_id(layer->id);
    layer->id = -1;
    
    dwin_free(layer->buffer);
    layer->buffer = NULL;

    dwin_free(layer);
    layer = NULL;
    return 0;
}

/**
 * delete a layer on the workstation 
 */
int dwin_layer_delete(dwin_layer_t *layer)
{
    if (layer == NULL)
    {
        return -1;
    }

    if (layer->workstation == NULL)
    {
        return -1;
    }
    
    /* NOTE: hide layer first */
    dwin_layer_zorder(layer, -1);

    if (dwin_workstation_del_layer(layer->workstation, layer) < 0)
    {
        return -1;
    }

    return dwin_layer_destroy(layer);
}

void dwin_layer_test(void)
{
    dwin_layer_t *ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    dwin_assert(dwin_layer_delete(ly) == 0);

    dwin_layer_t *ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_workstation_get_by_depth(0), ly2);
    dwin_assert(dwin_layer_delete(ly2) == 0);

    ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    dwin_assert(dwin_layer_delete(ly) == 0);

    ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_workstation_get_by_depth(0), ly2);
    dwin_assert(dwin_layer_delete(ly2) == 0);

    ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    
    memset(ly->buffer, 0xff, ly->width * ly->height * DWIN_LAYER_BPP);
    dwin_layer_zorder(ly, 0);

    ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly2);
    
    memset(ly2->buffer, 0x5a, ly2->width * ly2->height * DWIN_LAYER_BPP);
    dwin_layer_zorder(ly2, 0);
    dwin_layer_move(ly2, 100, 50);

    dwin_layer_resize(ly2, 200, 200, 50, 100);
    memset(ly2->buffer, 0x5a, ly2->width * ly2->height * DWIN_LAYER_BPP);

    dwin_layer_flush(ly2, 0, 0, ly2->width, ly2->height);

#if 0
    dwin_assert(dwin_layer_delete(ly) == 0);
    dwin_assert(dwin_layer_delete(ly2) == 0);
#endif

    dwin_layer_t *ly3 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly3 != NULL);
    dwin_layer_change_priority(ly3, DWIN_LAYER_PRIO_WINDOW);
    dwin_workstation_add_layer(dwin_current_workstation, ly3);    
    memset(ly3->buffer, 0x20, ly3->width * ly3->height * DWIN_LAYER_BPP);

    dwin_layer_zorder(ly3, 0);
    dwin_layer_move(ly3, 100, 200);

    dwin_layer_t *ly4 = dwin_layer_create(dwin_current_workstation->width / 4, dwin_current_workstation->height / 4);
    dwin_assert(ly4 != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly4);    

    memset(ly4->buffer, 0xA0, ly4->width * ly4->height * DWIN_LAYER_BPP);

    dwin_layer_zorder(ly4, 1);

    dwin_layer_change_priority(ly4, DWIN_LAYER_PRIO_WINDOW);
    dwin_layer_move(ly4, 100, 200);

    ly4 = dwin_layer_create(dwin_current_workstation->width / 4, dwin_current_workstation->height / 4);
    dwin_assert(ly4 != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly4);    

    memset(ly4->buffer, 0xff, ly4->width * ly4->height * DWIN_LAYER_BPP);

    dwin_layer_zorder(ly4, 10);

    dwin_layer_move(ly4, 100, 200);
    dbgprint("test done\n");
}

void dwin_layer_test2(void)
{
    /* window 0 100, 200, w/2, h/2 */
    dwin_layer_t *ly3 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly3 != NULL);
    dwin_layer_change_priority(ly3, DWIN_LAYER_PRIO_WINDOW);
    dwin_workstation_add_layer(dwin_current_workstation, ly3);    

    dwin_layer_draw_rect(ly3, 0, 0, ly3->width, ly3->height, 0xff00ff00);

    dwin_layer_zorder(ly3, 1);
    dwin_layer_move(ly3, 100, 200);

    /* window 1 200, 300, w/2, h/2 */
    ly3 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly3 != NULL);
    dwin_layer_change_priority(ly3, DWIN_LAYER_PRIO_WINDOW);
    dwin_workstation_add_layer(dwin_current_workstation, ly3);   

    dwin_layer_draw_rect(ly3, 0, 0, ly3->width, ly3->height, 0xff0000ff);

    dwin_layer_zorder(ly3, 2);
    dwin_layer_move(ly3, 200, 300);

    /* panel 1 200, 300, 100, 200 */
    ly3 = dwin_layer_create(100, 200);
    dwin_assert(ly3 != NULL);
    dwin_layer_change_priority(ly3, DWIN_LAYER_PRIO_PANEL);
    dwin_workstation_add_layer(dwin_current_workstation, ly3);    

    dwin_layer_draw_rect(ly3, 0, 0, ly3->width, ly3->height, 0xffff0000);

    dwin_buffer_t buf;
    dwin_buffer_init(&buf, 32, 32, dwin_malloc(32 * 32 * DWIN_LAYER_BPP));

    int i, j;
    for (j = 0; j < 32; j++)
    {
        for (i = 0; i < 32; i++)
        {    
            dwin_buffer_putpixel(&buf, i, j, 0xff000000 | (i * j * 0x101010) );
        }
    }

    dwin_layer_bitblt(ly3, 0, 0, &buf, NULL);

    dwin_layer_zorder(ly3, 0);
    dwin_layer_move(ly3, 200, 200);
}

void dwin_layer_scroll(void)
{
    dwin_layer_t *l0 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(l0 != NULL);
    dwin_layer_change_priority(l0, DWIN_LAYER_PRIO_WINDOW);
    dwin_workstation_add_layer(dwin_current_workstation, l0);    
    dwin_layer_draw_rect(l0, 0, 0, l0->width, l0->height, 0xfff0f0f0);
    dwin_layer_move(l0, 200, 200);
    dwin_layer_zorder(l0, 0);

    int priority = DWIN_LAYER_PRIO_PANEL;
    while (1)
    {
        dwin_layer_change_priority(l0, priority);
        dwin_layer_zorder(l0, priority);

        dwin_layer_draw_rect(l0, 0, 0, l0->width, l0->height, 0xff000000 | (priority * 0X101010 ));
        dwin_layer_move(l0, 200, 200);

        priority++;
        if (priority >= DWIN_LAYER_PRIO_NR)
            priority = DWIN_LAYER_PRIO_DESKTOP;

        dbgprint("priority:%d\n", priority);
    }
}

void dwin_layer_init(void)
{
    dwin_layer_id_init();

    dwin_workstation_idle_layer_init(dwin_current_workstation);

    dwin_workstation_mouse_layer_init(dwin_current_workstation);

    // dwin_layer_test();
    dwin_layer_test2();

    // dwin_layer_scroll();
}
