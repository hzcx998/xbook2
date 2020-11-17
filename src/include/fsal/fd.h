#ifndef _FSAL_FD_H
#define _FSAL_FD_H

#include "fsal.h"
#include <xbook/task.h>

#define FILE_FD_ALLOC   0X01    /* alloced */
#define FILE_FD_NORMAL  0X02    /* is normal file */
#define FILE_FD_DEVICE  0X04    /* is a device */
#define FILE_FD_SOCKET  0X08    /* is a socket */
#define FILE_FD_FIFO    0X10    /* is a fifo */
#define FILE_FD_PIPE0   0X20    /* is a pipe0: read */
#define FILE_FD_PIPE1   0X40    /* is a pipe1: write */

int fs_fd_init(task_t *task);
int fs_fd_exit(task_t *task);
int local_fd_install(int resid, unsigned int flags);
int local_fd_uninstall(int local_fd);
int local_fd_install_to(int resid, int newfd, unsigned int flags);
file_fd_t *fd_local_to_file(int local_fd);
int handle_to_local_fd(int handle, unsigned int flags);
int fs_fd_copy(task_t *src, task_t *dest);
int fs_fd_reinit(task_t *cur);

#endif  /* _FSAL_FD_H */