#ifndef __GUISRV_ENVIRONMENT_MOUSE_H__
#define __GUISRV_ENVIRONMENT_MOUSE_H__


typedef struct _env_mouse {
    int x, y;
    void (*left_btn_down)(void);
    void (*left_btn_up)(void);
    void (*right_btn_down)(void);
    void (*right_btn_up)(void);
    void (*middle_btn_down)(void);
    void (*middle_btn_up)(void);
    
} env_mouse_t;

extern env_mouse_t env_mouse;
void env_mouse_move();

int init_env_mouse();

#endif  /* __GUISRV_ENVIRONMENT_MOUSE_H__ */
