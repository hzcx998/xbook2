#include <xbook/syscall.h>
#include <xbook/process.h>
#include <xbook/memspace.h>
#include <xbook/resource.h>
#include <xbook/trigger.h>
#include <xbook/alarm.h>
#include <xbook/clock.h>
#include <xbook/mutexqueue.h>
#include <xbook/fs.h>
#include <xbook/driver.h>
#include <xbook/net.h>
#include <xbook/sharemem.h>
#include <xbook/sem.h>
#include <xbook/msgqueue.h>
#include <xbook/walltime.h>
#include <xbook/gui.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <gui/layer.h>
#include <gui/message.h>
#include <gui/screen.h>
#include <gui/timer.h>

syscall_t syscalls[SYSCALL_NR];

void syscall_init()
{
    syscalls[SYS_EXIT] = sys_exit;
    syscalls[SYS_FORK] = sys_fork;
    syscalls[SYS_WAITPID] = sys_waitpid;
    syscalls[SYS_GETPID] = sys_get_pid;
    syscalls[SYS_GETPPID] = sys_get_ppid;
    syscalls[SYS_TRIGGER] = sys_trigger_handler;
    syscalls[SYS_TRIGGERON] = sys_trigger_active;
    syscalls[SYS_TRIGGERACT] = sys_trigger_action;
    syscalls[SYS_TRIGRET] = sys_trigger_return;
    syscalls[SYS_SLEEP] = sys_sleep;
    syscalls[SYS_THREAD_CREATE] = sys_thread_create;
    syscalls[SYS_THREAD_EXIT] = sys_thread_exit;
    syscalls[SYS_THREAD_JOIN] = sys_thread_join;
    syscalls[SYS_THREAD_DETACH] = sys_thread_detach;
    syscalls[SYS_GETTID] = sys_get_tid;
    syscalls[SYS_THREAD_CANCEL] = sys_thread_cancel;
    syscalls[SYS_THREAD_TESTCANCEL] = sys_thread_testcancel;
    syscalls[SYS_THREAD_CANCELSTATE] = sys_thread_setcancelstate;
    syscalls[SYS_THREAD_CANCELTYPE] = sys_thread_setcanceltype;
    syscalls[SYS_SCHED_YEILD] = sys_sched_yeild;
    syscalls[SYS_MUTEX_QUEUE_CREATE] = sys_mutex_queue_free;
    syscalls[SYS_MUTEX_QUEUE_DESTROY] = sys_mutex_queue_alloc;
    syscalls[SYS_MUTEX_QUEUE_WAIT] = sys_mutex_queue_wait;
    syscalls[SYS_MUTEX_QUEUE_WAKE] = sys_mutex_queue_wake;
    syscalls[SYS_HEAP] = sys_mem_space_expend_heap;
    syscalls[SYS_MUNMAP] = sys_munmap;
    syscalls[SYS_ALARM] = sys_alarm;
    syscalls[SYS_WALLTIME] = sys_get_walltime;
    syscalls[SYS_GETTICKS] = sys_get_ticks;
    syscalls[SYS_GETTIMEOFDAY] = sys_gettimeofday;
    syscalls[SYS_CLOCK_GETTIME] = sys_clock_gettime;
    syscalls[SYS_UNID] = sys_unid;
    syscalls[SYS_TSTATE] = sys_tstate;
    syscalls[SYS_GETVER] = sys_getver;
    syscalls[SYS_MSTATE] = sys_mstate;    
    syscalls[SYS_USLEEP] = sys_usleep;    
    syscalls[SYS_OPEN] = sys_open;
    syscalls[SYS_CLOSE] = sys_close;
    syscalls[SYS_READ] = sys_read;
    syscalls[SYS_WRITE] = sys_write;
    syscalls[SYS_LSEEK] = sys_lseek;
    syscalls[SYS_ACCESS] = sys_access;
    syscalls[SYS_UNLINK] = sys_unlink;
    syscalls[SYS_FTRUNCATE] = sys_ftruncate;
    syscalls[SYS_FSYNC] = sys_fsync;
    syscalls[SYS_IOCTL] = sys_ioctl;
    syscalls[SYS_FCNTL] = sys_fcntl;
    syscalls[SYS_TELL] = sys_tell;
    syscalls[SYS_MKDIR] = sys_mkdir;
    syscalls[SYS_RMDIR] = sys_rmdir;
    syscalls[SYS_RENAME] = sys_rename;
    syscalls[SYS_CHDIR] = sys_chdir;
    syscalls[SYS_GETCWD] = sys_getcwd;
    syscalls[SYS_EXECVE] = sys_execve;
    syscalls[SYS_STAT] = sys_stat;
    syscalls[SYS_FSTAT] = sys_fstat;
    syscalls[SYS_CHMOD] = sys_chmod;
    syscalls[SYS_FCHMOD] = sys_fchmod;
    syscalls[SYS_OPENDIR] = sys_opendir;
    syscalls[SYS_CLOSEDIR] = sys_closedir;
    syscalls[SYS_READDIR] = sys_readdir;
    syscalls[SYS_REWINDDIR] = sys_rewinddir;
    syscalls[SYS_MKFS] = sys_mkfs;
    syscalls[SYS_MOUNT] = sys_mount;
    syscalls[SYS_UNMOUNT] = sys_unmount;
    syscalls[SYS_SOCKET] = sys_socket;
    syscalls[SYS_BIND] = sys_bind;
    syscalls[SYS_CONNECT] = sys_connect;
    syscalls[SYS_LISTEN] = sys_listen;
    syscalls[SYS_ACCEPT] = sys_accept;
    syscalls[SYS_SEND] = sys_send;
    syscalls[SYS_RECV] = sys_recv;
    syscalls[SYS_SENDTO] = sys_sendto;
    syscalls[SYS_RECVFROM] = sys_recvfrom;
    syscalls[SYS_SHUTDOWN] = sys_shutdown;
    syscalls[SYS_GETPEERNAME] = sys_getpeername;
    syscalls[SYS_GETSOCKNAME] = sys_getsockname;
    syscalls[SYS_GETSOCKOPT] = sys_getsockopt;
    syscalls[SYS_SETSOCKOPT] = sys_setsockopt;
    syscalls[SYS_IOCTLSOCKET] = sys_ioctlsocket;
    syscalls[SYS_SELECT] = sys_select;
    syscalls[SYS_DUP] = sys_dup;
    syscalls[SYS_DUP2] = sys_dup2;
    syscalls[SYS_PIPE] = sys_pipe;
    syscalls[SYS_SHMGET] = sys_shmem_get;
    syscalls[SYS_SHMPUT] = sys_shmem_put;
    syscalls[SYS_SHMMAP] = sys_shmem_map;
    syscalls[SYS_SHMUNMAP] = sys_shmem_unmap;
    syscalls[SYS_SEMGET] = sys_sem_get;
    syscalls[SYS_SEMPUT] = sys_sem_put;
    syscalls[SYS_SEMDOWN] = sys_sem_down;
    syscalls[SYS_SEMUP] = sys_sem_up;
    syscalls[SYS_MSGGET] = sys_msgque_get;
    syscalls[SYS_MSGPUT] = sys_msgque_put;
    syscalls[SYS_MSGSEND] = sys_msgque_send;
    syscalls[SYS_MSGRECV] = sys_msgque_recv;
    syscalls[SYS_TRIGPENDING] = sys_trigger_pending;
    syscalls[SYS_TRIGPROCMASK] = sys_trigger_proc_mask;
    syscalls[SYS_LAYERNEW] = sys_new_layer;
    syscalls[SYS_LAYERDEL] = sys_del_layer;
    syscalls[SYS_LAYERZ] = sys_layer_z;
    syscalls[SYS_LAYERMOVE] = sys_layer_move;
    syscalls[SYS_LAYERREFRESH] = sys_layer_refresh;
    syscalls[SYS_LAYERGETWINTOP] = sys_layer_get_win_top;
    syscalls[SYS_LAYERSETWINTOP] = sys_layer_set_win_top;
    syscalls[SYS_GINIT] = sys_g_init;
    syscalls[SYS_GQUIT] = sys_g_quit;
    syscalls[SYS_GGETMSG] = sys_g_get_msg;
    syscalls[SYS_GTRYGETMSG] = sys_g_try_get_msg;
    syscalls[SYS_LAYERSETFOCUS] = sys_layer_set_focus;
    syscalls[SYS_LAYERGETFOCUS] = sys_layer_get_focus;
    syscalls[SYS_LAYERSETREGION] = sys_layer_set_region;
    syscalls[SYS_GPOSTMSG] = sys_g_post_msg;
    syscalls[SYS_GSENDMSG] = sys_g_send_msg;
    syscalls[SYS_LAYERSETFLG] = sys_layer_set_flags;
    syscalls[SYS_LAYERRESIZE] = sys_layer_resize;
    syscalls[SYS_LAYERFOCUS] = sys_layer_focus;
    syscalls[SYS_LAYERFOCUSWINTOP] = sys_layer_focus_win_top;
    syscalls[SYS_GSCREENGET] = sys_screen_get;
    syscalls[SYS_GSCREENSETWINRG] = sys_screen_set_window_region;
    syscalls[SYS_LAYERGETDESKTOP] = sys_layer_get_desktop;
    syscalls[SYS_LAYERSETDESKTOP] = sys_layer_set_desktop;
    syscalls[SYS_GNEWTIMER] = sys_gui_new_timer;
    syscalls[SYS_GMODIFYTIMER] = sys_gui_modify_timer;
    syscalls[SYS_GDELTIMER] = sys_gui_del_timer;
    syscalls[SYS_LAYERSYNCBMP] = sys_layer_sync_bitmap;
    syscalls[SYS_LAYERSYNCBMPEX] = sys_layer_sync_bitmap_ex;
    syscalls[SYS_LAYERCOPYBMP] = sys_layer_copy_bitmap;
    syscalls[SYS_GGETICONPATH] = sys_gui_get_icon;
    syscalls[SYS_GSETICONPATH] = sys_gui_set_icon;
    syscalls[SYS_PROBE] = sys_probe;
}