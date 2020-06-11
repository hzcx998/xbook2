#include <environment/interface.h>
#include <guisrv.h>
#include <stdio.h>

#define DEBUG_LOCAL 1

env_display_t env_display_table[ENV_DISPLAY_NR];

/**
 * 添加一个显示信息，成功返回0，失败返回-1
 * 
 */
int env_display_add(env_display_t *disp)
{
    int i;
    for (i = 0; i < ENV_DISPLAY_NR; i++) {
        if (!env_display_table[i].dispid)
            break;
    }
    if (i >= ENV_DISPLAY_NR) {
        return -1;
    }
#if DEBUG_LOCAL == 1
    printf("[%s] %s: display id=%d, msg id=%d\n", SRV_NAME, __func__, disp->dispid, disp->msgid);
#endif
    env_display_table[i] = *disp;
    return 0;
}

/**
 * 删除一个显示信息，成功返回0，失败返回-1
 */
int env_display_del(unsigned int dispid)
{
    int i;
    for (i = 0; i < ENV_DISPLAY_NR; i++) {
        if (env_display_table[i].dispid == dispid) {
#if DEBUG_LOCAL == 1
            printf("[%s] %s: display id=%d, msg id=%d\n", SRV_NAME, __func__,
            env_display_table[i].dispid, env_display_table[i].msgid);
#endif
            env_display_table[i].dispid = 0;
            env_display_table[i].msgid = -1;
            return 0;
        } 
    }
    return -1;
}

/**
 * 查找一个显示
 */
env_display_t *env_display_find(unsigned int dispid)
{
    int i;
    for (i = 0; i < ENV_DISPLAY_NR; i++)
        if (env_display_table[i].dispid == dispid)
            return &env_display_table[i];
    return NULL;
}

int init_env_display()
{
    int i;
    for (i = 0; i < ENV_DISPLAY_NR; i++) {
        env_display_table[i].msgid = -1;
        env_display_table[i].dispid = 0;
    }
    return 0;
}

