#include <sgi/sgif.h>
#include <sgi/sgi.h>
#include <sgi/sgii.h>
#include <string.h>

int SGI_RegisterFont(
    SGI_Display *display,
    const char *name,
    const char *copyright,
    void *addr,
    unsigned int width,
    unsigned int height
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;

    SGI_FontInfo *font = SGI_Malloc(sizeof(SGI_FontInfo));
    if (font == NULL)
        return -1;
    memset(font, 0, sizeof(SGI_FontInfo));
    strcpy(font->name, name);
    strcpy(font->copyright, copyright);
    font->addr = addr;
    font->width = width;
    font->height = height;
    list_add_tail(&font->list, &display->font_list_head);
    return 0;
}

int SGI_UnregisterFont(
    SGI_Display *display,
    SGI_FontInfo *font
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;

    if (!list_find(&font->list, &display->font_list_head))
        return -1;

    list_del(&font->list);
    SGI_Free(font);

    return 0;
}

int SGI_UnregisterFontByName(
    SGI_Display *display,
    const char *name
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;

    SGI_FontInfo *font;

    list_for_each_owner (font, &display->font_list_head, list) {
        if (!strcmp(font->name, name)) {
            list_del(&font->list);
            SGI_Free(font);
            return 0;
        }
    }
    return -1;
}

SGI_FontInfo *SGI_LoadFont(
    SGI_Display *display,
    const char *name
) {
    if (!display)
        return NULL;
    if (!display->connected)
        return NULL;

    SGI_FontInfo *font;
    list_for_each_owner (font, &display->font_list_head, list) {
        if (!strcmp(font->name, name))
            return font;
    }
    return NULL;
}

int SGI_SetFont(
    SGI_Display *display,
    SGI_Window win,
    SGI_FontInfo *font
) {
    if (!display)
        return -1;
    if (!display->connected)
        return -1;
    if (SGI_BAD_WIN_HANDLE(win))
        return -1;
    if (!font)
        return -1;
    
    SGI_WindowInfo *winfo = SGI_DISPLAY_GET_WININFO(display, win);
    if (!winfo)
        return -1;

    winfo->font = font;

    return 0;
}

int SGI_InitFont(SGI_Display *display)
{
    /* 字体链表头 */
    init_list(&display->font_list_head);

    /* 注册一个默认字体 */
    if (SGI_RegisterFontStandard(display) < 0)  
        return -1;
    return 0;
}
