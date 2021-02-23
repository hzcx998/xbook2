#ifndef _XBOOK_FSAL_FD_H
#define _XBOOK_FSAL_FD_H

#include "fsal.h"
#include <xbook/task.h>

#define FSAL_FILE_FD_IS_BAD   (1 << 31)    /* alloced */
#define FILE_FD_NORMAL  0X01    /* is normal file */
#define FILE_FD_DEVICE  0X02    /* is a device */
#define FILE_FD_SOCKET  0X04    /* is a socket */
#define FILE_FD_FIFO    0X08    /* is a fifo */
#define FILE_FD_PIPE0   0X10    /* is a pipe0: read */
#define FILE_FD_PIPE1   0X20    /* is a pipe1: write */

#define FILE_FD_TYPE_MASK   0XFF

#define FILE_FD_CLOEXEC   0X1000    /* 执行时关闭fd的标志 */

int fs_fd_init(task_t *task);
int fs_fd_exit(task_t *task);
int local_fd_install(int resid, unsigned int flags);
int local_fd_install_based(int resid, unsigned int flags, int basefd);
int local_fd_install_to(int resid, int newfd, unsigned int flags);
int local_fd_uninstall(int local_fd);
file_fd_t *fd_local_to_file(int local_fd);
int handle_to_local_fd(int handle, unsigned int flags);
int fs_fd_copy(task_t *src, task_t *dest);
int fs_fd_reinit(task_t *cur);
int fs_fd_copy_only(task_t *src, task_t *dest);

#endif  /* _XBOOK_FSAL_FD_H */