#ifndef __GUISRV_ENVIRONMENT_BOX_H__
#define __GUISRV_ENVIRONMENT_BOX_H__

typedef struct _env_box
{
    int left;       /* 做边 */
    int top;        /* 顶部 */
    int right;      /* 右边 */
    int bottom;     /* 下边 */
} env_box_t;

#define ENV_BOX_INIT(box, l, t, r, b) \
        do { \
            (box).left = l; \
            (box).top = t; \
            (box).right = r; \
            (box).bottom = b; \
        } while (0)

#define ENV_IN_BOX(box, x, y) \
        (x >= (box).left && y >= (box).top && x <= (box).right && y <= (box).bottom)

#endif  /* __GUISRV_ENVIRONMENT_BOX_H__ */