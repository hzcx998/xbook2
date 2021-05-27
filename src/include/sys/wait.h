#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

/* 等待标志 */
#define WNOHANG     0X01    /* 若由pid指定的子进程未发生状态改变(没有结束)，则waitpid()不阻塞，立即返回0 */
#define WUNTRACED   2   /* unsupported! :Report status of stopped children.  */


#  if !defined (WSTOPSIG)
#    define WSTOPSIG(s)       ((s) >> 8)
#  endif /* !WSTOPSIG */

#  if !defined (WTERMSIG)
#    define WTERMSIG(s)	      ((s) & 0177)
#  endif /* !WTERMSIG */

#  if !defined (WEXITSTATUS)
#    define WEXITSTATUS(s)    (((s) & 0xff00) >> 8)
#  endif /* !WEXITSTATUS */

#  if !defined (WIFSTOPPED)
#    define WIFSTOPPED(s)     (((s) & 0177) == 0177)
#  endif /* !WIFSTOPPED */

#  if !defined (WIFEXITED)
#    define WIFEXITED(s)      (((s) & 0377) == 0)
#  endif /* !WIFEXITED */

#  if !defined (WIFSIGNALED)
#    define WIFSIGNALED(s)    (!WIFSTOPPED(s) && !WIFEXITED(s))
#  endif /* !WIFSIGNALED */

#  if !defined (WIFCORED)
#    if defined (WCOREDUMP)
#      define WIFCORED(s)	(WCOREDUMP(s))
#    else
#      define WIFCORED(s)       ((s) & 0200)
#    endif
#  endif /* !WIFCORED */

#endif   /* _SYS_WAIT_H */