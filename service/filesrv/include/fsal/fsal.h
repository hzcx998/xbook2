#ifndef __FILESRV_FSAL_H__
#define __FILESRV_FSAL_H__

/* File system abstraction layer (FSAL) 文件系统抽象层 */

typedef struct {
    void *extention;
    int (*mount)(void *, int );
} fsal_t;

int init_fsal();

#endif  /* __FILESRV_FSAL_H__ */