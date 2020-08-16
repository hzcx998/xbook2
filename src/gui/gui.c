#include <gui/screen.h>
#include <gui/keyboard.h>
#include <gui/mouse.h>
#include <gui/event.h>
#include <gui/font.h>
#include <gui/text.h>
#include <gui/rect.h>
#include <gui/console/console.h>
#include <gui/layer.h>

#include <xbook/debug.h>
#include <xbook/gui.h>
#include <xbook/task.h>

/* 主要是处理输入事件 */
void kgui_thread(void *arg)
{
    con_loop();
}

void init_gui()
{
    if (gui_init_screen() < 0)
        panic("init gui screen failed!\n");
    if (gui_init_keyboard() < 0)
        panic("init gui keyboard failed!\n");
    if (gui_init_mouse() < 0)
        panic("init gui keyboard failed!\n");

    if (gui_init_event() < 0)
        panic("init gui event failed!\n");

    gui_init_font();

    if (gui_screen.open() < 0)
        panic("open gui screen failed!\n");
    if (gui_keyboard.open() < 0)
        panic("open gui keyboard failed!\n");
    if (gui_mouse.open() < 0)
        panic("open gui keyboard failed!\n");

    pr_info("[gui]: init done.\n");

    if (gui_init_layer() < 0)
        panic("init gui layer failed!\n");



    if (gui_init_console() < 0)
        panic("init gui console failed!\n");

    if (kthread_start("kgui", TASK_PRIO_USER, kgui_thread, NULL) == NULL)
        panic("start kgui thread failed!\n");
}
