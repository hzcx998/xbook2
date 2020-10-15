#ifndef _FREETERM_PTY_H /* 伪终端 */
#define _FREETERM_PTY_H

#define FT_PTM_BUFLEN   256

typedef struct {
    int initialized; 
    int fd_master;
} ft_pty_t;

extern ft_pty_t ft_pty;

int ft_pty_init(ft_pty_t *pty);
int ft_pty_launch(ft_pty_t *pty, char *pathname);
int ft_pty_exit(ft_pty_t *pty);

#endif /* _FREETERM_PTY_H */