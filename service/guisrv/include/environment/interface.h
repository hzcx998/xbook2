#ifndef __GUISRV_ENVIRONMENT_INTERFACE_H__
#define __GUISRV_ENVIRONMENT_INTERFACE_H__

#include <sys/list.h>

/* 支持的显示的最大数 */
#define ENV_DISPLAY_NR      64

/* 接口环境 */
typedef struct _env_display {
    int dispid;              /* 显示id */
    int msgid;                  /* 消息队列id */
} env_display_t;

int init_env_display();
int env_display_add(env_display_t *disp);
int env_display_del(unsigned int dispid);
env_display_t *env_display_find(unsigned int dispid);

#endif  /* __GUISRV_ENVIRONMENT_INTERFACE_H__ */