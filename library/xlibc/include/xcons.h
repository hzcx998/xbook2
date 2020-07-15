#ifndef _XLIBC_XCONSOLE_H
#define _XLIBC_XCONSOLE_H


#define XCONS_KMOD_SHIFT_L    0x01
#define XCONS_KMOD_SHIFT_R    0x02
#define XCONS_KMOD_SHIFT      (XCONS_KMOD_SHIFT_L | XCONS_KMOD_SHIFT_R)
#define XCONS_KMOD_CTRL_L     0x04
#define XCONS_KMOD_CTRL_R     0x08
#define XCONS_KMOD_CTRL       (XCONS_KMOD_CTRL_L | XCONS_KMOD_CTRL_R)
#define XCONS_KMOD_ALT_L      0x10
#define XCONS_KMOD_ALT_R      0x20
#define XCONS_KMOD_ALT        (XCONS_KMOD_ALT_L | XCONS_KMOD_ALT_R)
#define XCONS_KMOD_PAD	    0x40
#define XCONS_KMOD_NUM	    0x80
#define XCONS_KMOD_CAPS	    0x100

typedef struct {
    long type;
    int data;   /* 传递的数据 */        
    int ctrl;   /* 控制 */
} xcons_msg_t;


int xcons_connect();
int xcons_close();
int xcons_next_msg(xcons_msg_t *m);
int xcons_poll_msg(xcons_msg_t *m);
int xcons_xmit_data(void *buf, size_t buflen);
int xcons_clear();

#endif  /* _XLIBC_XCONSOLE_H */
