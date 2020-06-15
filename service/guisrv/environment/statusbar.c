#include <environment/statusbar.h>
#include <environment/desktop.h>
#include <drivers/screen.h>
#include <window/draw.h>
#include <guisrv.h>
#include <stdio.h>
#include <sys/time.h>

static gui_statusbar_item_t statusbar_item_table[GUI_STATUSBAR_ITEM_NR];

/* 默认的图标数据 */
static GUI_COLOR statusbar_item_icon_data[12 * 12] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

/* 默认的图标数据 */
static GUI_COLOR statusbar_item_icon_data2[12 * 12] = {
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
    0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00, 
};

gui_statusbar_manager_t statusbar_manager;

gui_statusbar_item_t *gui_statusbar_item_alloc()
{
    gui_statusbar_item_t *item;
    int i;
    for (i = 0; i < GUI_STATUSBAR_ITEM_NR; i++) {
        item = &statusbar_item_table[i];
        if (!item->flags) {
            item->flags = GUI_STATUSBAR_USED;
            item->button = NULL;
            init_list(&item->list);
            break;
        }
    }
    if (i >= GUI_STATUSBAR_ITEM_NR) {
        return NULL;
    }
    return item;
}

int gui_statusbar_item_free(gui_statusbar_item_t *_item)
{
    gui_statusbar_item_t *item;
    int i;
    for (i = 0; i < GUI_STATUSBAR_ITEM_NR; i++) {
        item = &statusbar_item_table[i];
        if (item->flags && item == _item) {
            item->flags = 0;
            return 0;
        }
    }
    return -1;
}

int statusbar_btn_down_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    return 0;
}

int statusbar_btn_up_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    return 0;
}

/**
 * 显示桌面处理
 */
int statusbar_show_desktop_handler(gui_button_t *button, int btn, int local_mx, int local_my)
{
    gui_statusbar_item_t *sbitem = (gui_statusbar_item_t *) button->data;
    
    if (!sbitem->count) {
        sbitem->count++;
        gui_window_hide_all();
    } else {
        sbitem->count = 0;
        gui_window_show_all();
    }
    return GUI_WIDGET_EVENT_HANDLED;
}

static void __manager_read()
{
    statusbar_time_read();

}

static int __destroy(gui_statusbar_item_t *sbitem)
{
    if (!sbitem)
        return -1;
    
    gui_button_destroy(sbitem->button);
    return 0;
}

static int __add(gui_statusbar_item_t *sbitem)
{
    if (!sbitem)
        return -1;

    if (sbitem->flags & GUI_STATUSBAR_MENU) {
        if (list_find(&sbitem->list, &statusbar_manager.menu_list_head))
            return -1;
        list_add_tail(&sbitem->list, &statusbar_manager.menu_list_head);
    } else if (sbitem->flags & GUI_STATUSBAR_ICON) {
        if (list_find(&sbitem->list, &statusbar_manager.icon_list_head))
            return -1;
        list_add_tail(&sbitem->list, &statusbar_manager.icon_list_head);
    }
    return 0;
}

static int __del(gui_statusbar_item_t *sbitem)
{
    if (!sbitem)
        return -1;

    if (sbitem->flags & GUI_STATUSBAR_MENU) {
        if (!list_find(&sbitem->list, &statusbar_manager.menu_list_head))
            return -1;
        list_del(&sbitem->list);
    } else if (sbitem->flags & GUI_STATUSBAR_ICON) {
        if (!list_find(&sbitem->list, &statusbar_manager.icon_list_head))
            return -1;
        list_del(&sbitem->list);
    }
    return 0;
}

static int __set_text(gui_statusbar_item_t *sbitem, char *text)
{
    if (sbitem->button->label.content.type == GUI_LABEL_TEXT) {
        sbitem->button->set_text(sbitem->button, text);
        return 0;
    }
    return -1;
}

/**
 * 当有元素的增加或者删除的时候，才会更新
 */
void gui_statusbar_update()
{
    /* 先刷新背景，再显示 */
    gui_window_draw_rect_fill(statusbar_manager.window, 0, 0, 
        statusbar_manager.window->width, statusbar_manager.window->height, statusbar_manager.back_color);
    gui_window_update(statusbar_manager.window, 0, 0, 
        statusbar_manager.window->width, statusbar_manager.window->height);

    int x = 4, y = 0;    
    /* 先更新菜单部分 */
    gui_statusbar_item_t *sbitem;
    list_for_each_owner (sbitem, &statusbar_manager.menu_list_head, list) {
        /* 设置控件位置 */
        sbitem->button->set_location(sbitem->button, x, y);
        sbitem->button->show(sbitem->button);
        x += sbitem->button->label.widget.width;

        /* 显示到菜单的最右端就不显示 */
        if (x >= statusbar_manager.window->width / 2)
            break;
    }
    /* 再更新图标部分 */
    x = statusbar_manager.window->width - 4;
    list_for_each_owner (sbitem, &statusbar_manager.icon_list_head, list) {
        x -= sbitem->button->label.widget.width;
        
        /* 显示到菜单的最右端就不显示 */
        if (x <= statusbar_manager.window->width / 2)
            break;

        /* 设置控件位置 */
        sbitem->button->set_location(sbitem->button, x, y);
        sbitem->button->show(sbitem->button);
        
    }
}

gui_statusbar_item_t *gui_create_statusbar_item(
    gui_label_content_t *content,
    int flags,
    btn_handler_t down_handler,
    btn_handler_t up_handler
) {
    if (content->type == GUI_LABEL_UNKNOWN)
        return NULL;

    gui_statusbar_item_t *sbitem = gui_statusbar_item_alloc();
    if (sbitem == NULL)
        return NULL;

    unsigned int width;

    if (content->type == GUI_LABEL_TEXT) {
        width = content->text.text_len * current_font->width + 4; 
        if (width < GUI_STATUSBAR_ITEM_SIZE)
            width = GUI_STATUSBAR_ITEM_SIZE;
    } else if (content->type == GUI_LABEL_PIXMAP) {
        width =  GUI_STATUSBAR_ITEM_SIZE;
    }

    sbitem->button = gui_create_button(content->type, 0, 0, 
        width, GUI_STATUSBAR_ITEM_SIZE);

    if (sbitem->button == NULL) {
        gui_statusbar_item_free(sbitem);
        return NULL;
    }
    /* 设置元素信息 */
    if (content->type == GUI_LABEL_TEXT) {
        if (content->text.text == NULL) {
            sbitem->button->set_text(sbitem->button, " ");
        } else {
            sbitem->button->set_text(sbitem->button, content->text.text);
        }
    } else if (content->type == GUI_LABEL_PIXMAP) {
        if (content->pixmap.data == NULL) {
            sbitem->button->set_pixmap(sbitem->button, 12, 12,
                statusbar_item_icon_data);
        } else {
            sbitem->button->set_pixmap(sbitem->button, 
                content->pixmap.width, content->pixmap.height, content->pixmap.data);
        }
    }

    sbitem->button->set_color(sbitem->button, statusbar_manager.back_color, statusbar_manager.font_color);
    sbitem->button->set_color3(sbitem->button, statusbar_manager.back_color,
        statusbar_manager.back_color + 0x202020, statusbar_manager.back_color + 0x101010);

    sbitem->button->set_data(sbitem->button, sbitem);

    /* 设置处理函数 */
    if (down_handler == NULL )
        down_handler = statusbar_btn_down_handler;
    if (up_handler == NULL )
        up_handler = statusbar_btn_up_handler;
    
    sbitem->button->set_handler(sbitem->button, down_handler, up_handler);
    sbitem->button->set_align(sbitem->button, GUI_WIDGET_ALIGN_CENTER);
    sbitem->button->add(sbitem->button, statusbar_manager.window->layer);

    init_list(&sbitem->list);
    sbitem->flags |= flags;
    sbitem->count = 0;

    return sbitem;
}

int init_statusbar_manager()
{
    init_list(&statusbar_manager.menu_list_head);
    init_list(&statusbar_manager.icon_list_head);
    statusbar_manager.back_color = GUI_STATUSBAR_BACK_COLOR;
    statusbar_manager.active_color = GUI_STATUSBAR_ACTIVE_COLOR;
    statusbar_manager.font_color = GUI_STATUSBAR_FONT_COLOR;
    statusbar_manager.read = __manager_read;


    statusbar_manager.window = gui_create_window(
        NULL, 0, 0, drv_screen.width, GUI_STATUSBAR_HEIGHT,
        statusbar_manager.back_color, GUIW_NO_TITLE | GUIW_FIXED, NULL);
    
    if (statusbar_manager.window == NULL)
        return -1;

    gui_window_show(statusbar_manager.window);

    int i;
    for (i = 0; i < GUI_STATUSBAR_ITEM_NR; i++) {
        statusbar_item_table[i].flags = 0;
        statusbar_item_table[i].button = NULL;
        init_list(&statusbar_item_table[i].list);
        statusbar_item_table[i].count = 0;
        statusbar_item_table[i].add = __add;
        statusbar_item_table[i].del = __del;
        statusbar_item_table[i].destroy = __destroy;
        statusbar_item_table[i].set_text = __set_text;
    }

    gui_label_content_t cont;
    cont.type = GUI_LABEL_TEXT;
    cont.text.text = "menu";
    cont.text.text_len = 4;
    gui_statusbar_item_t *sbitem0 = gui_create_statusbar_item(&cont, GUI_STATUSBAR_MENU, NULL, NULL);
    if (sbitem0 == NULL)
        return -1;

    sbitem0->add(sbitem0);

    cont.type = GUI_LABEL_PIXMAP;
    cont.pixmap.data = NULL;
    gui_statusbar_item_t *sbitem1 = gui_create_statusbar_item(&cont, GUI_STATUSBAR_MENU, NULL, NULL);
    if (sbitem1 == NULL)
        return -1;
    sbitem1->add(sbitem1);

    cont.type = GUI_LABEL_PIXMAP;
    cont.pixmap.data = statusbar_item_icon_data2;
    cont.pixmap.width = 12;
    cont.pixmap.height = 12;
    gui_statusbar_item_t *sbitem3 = gui_create_statusbar_item(&cont, GUI_STATUSBAR_ICON, statusbar_show_desktop_handler, NULL);
    if (sbitem3 == NULL)
        return -1;
    sbitem3->add(sbitem3);

    /* 显示时间 */
    ktime_t ktm;
    ktime(&ktm);
    /* 生成一个时间字符串 */
    char timestr[6] = {0};
    sprintf(timestr, "%d:%d", ktm.hour, ktm.minute);
    cont.type = GUI_LABEL_TEXT;
    cont.text.text = timestr;
    cont.text.text_len = 6;
    gui_statusbar_item_t *sbitem2 = gui_create_statusbar_item(&cont, GUI_STATUSBAR_ICON, NULL, NULL);
    if (sbitem2 == NULL)
        return -1;
    sbitem2->add(sbitem2);
    statusbar_manager.time_item = sbitem2;

    gui_statusbar_update();
    return 0;
}
