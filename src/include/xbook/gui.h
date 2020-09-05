#ifndef _XBOOK_GUI_H
#define	_XBOOK_GUI_H

#include <xbook/task.h>

void init_gui();

int sys_g_init(void);
int sys_g_quit(void);
int gui_user_init(task_t *task);
int gui_user_exit(task_t *task);

#endif /* _XBOOK_GUI_H */