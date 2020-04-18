#ifndef _XBOOK_DRIVER_H
#define _XBOOK_DRIVER_H

#include "list.h"
#include <arch/atomic.h>
#include "string.h"
#include "spinlock.h"

#define DRIVER_NAME_LEN 32

#ifndef DEVICE_NAME_LEN
#define DEVICE_NAME_LEN 32
#endif

/* 驱动状态：0~15位是内核使用，16~23位自定义 */
#define IO_SUCCESS             (0 << 0)    /* 成功 */
#define IO_FAILED              (1 << 0)    /* 失败 */        
#define IO_PENDING             (1 << 1)    /* 未决 */

/* 系统可以打开的设备数量，可以根据设备数调节 */
#define DEVICE_HANDLE_NR        32

#define IS_BAD_DEVICE_HANDLE(handle) \
        ((handle) < 0 || (handle) >= DEVICE_HANDLE_NR)

#define GET_DEVICE_BY_HANDLE(handle) \
        device_handle_table[handle]

/* 设备控制码：
0~15位：命令（0-0x7FFF系统保留，0x8000-0xffff用户自定义）
16~31位：设备类型
 */
#ifndef DEVCTL_CODE
#define DEVCTL_CODE(type, cmd) \
        ((unsigned int) ((((type) & 0xffff) << 16) | ((cmd) & 0xffff)))
#endif

/* io请求函数表 */
enum _io_request_function {
    IOREQ_OPEN,                     /* 设备打开派遣索引 */
    IOREQ_CLOSE,                    /* 设备关闭派遣索引 */        
    IOREQ_READ,                     /* 设备读取派遣索引 */
    IOREQ_WRITE,                    /* 设备写入派遣索引 */
    IOREQ_DEVCTL,                   /* 设备控制派遣索引 */
    MAX_IOREQ_FUNCTION_NR
};

typedef enum _device_type {
    DEVICE_TYPE_BEEP,                    /* 蜂鸣器设备 */
    DEVICE_TYPE_DISK,                    /* 磁盘设备 */
    DEVICE_TYPE_KEYBOARD,                /* 键盘设备 */
    DEVICE_TYPE_MOUSE,                   /* 鼠标设备 */
    DEVICE_TYPE_NULL,                    /* 空设备 */
    DEVICE_TYPE_PORT,                   /* 端口设备 */
    DEVICE_TYPE_SERIAL_PORT,            /* 串口设备 */
    DEVICE_TYPE_PARALLEL_PORT,           /* 并口设备 */
    DEVICE_TYPE_PHYSIC_NETCARD,          /* 物理网卡设备 */
    DEVICE_TYPE_PRINTER,                 /* 打印机设备 */
    DEVICE_TYPE_SCANNER,                 /* 扫描仪设备 */
    DEVICE_TYPE_SCREEN,                  /* 屏幕设备 */
    DEVICE_TYPE_SOUND,                   /* 声音设备 */
    DEVICE_TYPE_STREAM,                  /* 流设备 */
    DEVICE_TYPE_UNKNOWN,                 /* 未知设备 */
    DEVICE_TYPE_VIDEO,                   /* 视频设备 */
    DEVICE_TYPE_VIRTUAL_DISK,            /* 虚拟磁盘设备 */
    DEVICE_TYPE_WAVE_IN,                 /* 声音输入设备 */
    DEVICE_TYPE_WAVE_OUT,                /* 声音输出设备 */
    DEVICE_TYPE_8042_PORT,               /* 8042端口设备 */
    DEVICE_TYPE_NETWORK,                 /* 网络设备 */
    DEVICE_TYPE_BUS_EXTERNDER,           /* BUS总线扩展设备 */
    DEVICE_TYPE_ACPI,                    /* ACPI设备 */
    MAX_DEVICE_TYPE_NR
} _device_type_t;

#define DEVCTL_CODE_TEST DEVCTL_CODE(DEVICE_TYPE_SERIAL_PORT, 1)

/* 驱动状态时32位无符号整数 */
typedef unsigned int iostatus_t;

/* 设备句柄 */
typedef int handle_t;

/* 提前声明 */
struct _driver_object;
struct _device_object;


/* io请求标志 */
enum _io_request_flags {
    IOREQ_OPEN_OPERATION        = (1 << 0), 
    IOREQ_CLOSE_OPERATION       = (1 << 1),
    IOREQ_READ_OPERATION        = (1 << 2),
    IOREQ_WRITE_OPERATION       = (1 << 3),
    IOREQ_DEVCTL_OPERATION      = (1 << 4),
    IOREQ_BUFFERED_IO           = (1 << 5),
    IOREQ_COMPLETION            = (1 << 31),    /* 完成请求 */
};

enum _device_object_flags {
    DO_BUFFERED_IO              = (1 << 0),
    DO_DIRECT_IO                = (1 << 1),
};

typedef struct _drver_extension 
{
    unsigned long unused;
} drver_extension_t;

typedef struct _io_parame {
    union 
    {
        struct {
            unsigned int flags;
            char *devname;
        } open;
        struct {
            unsigned long length;
            unsigned long offset;
        } read;
        struct {
            unsigned long length;
            unsigned long offset;
        } write;
        struct {
            unsigned int code;
            unsigned long arg;
        } devctl;
    };
} io_parame_t;

typedef struct _io_status_block {
    iostatus_t status;                  /* 状态 */
    unsigned long infomation;           /* io结果信息 */
} io_status_block_t;

/* 输入输出请求 */
typedef struct _io_request 
{
    list_t list;                        /* 队列链表 */
    unsigned int flags;                 /* 标志 */
    struct _mdl *mdl_address;           /* 内存描述列表地址 */
    void *system_buffer;                /* 系统缓冲区 */
    void *user_buffer;                  /* 用户缓冲区 */
    struct _device_object *devobj;      /* 设备对象 */
    io_parame_t parame;                 /* 参数 */
    io_status_block_t io_status;        /* 状态块 */
    
} io_request_t;

typedef struct _device_queue {
    list_t list_head;   /* 队列列表 */
    spinlock_t lock;    /* 维护队列的锁 */
    bool busy;          /* 队列是否繁忙 */
} device_queue_t;

/* 设备对象 */
typedef struct _device_object
{
    list_t list;                        /* 设备在驱动中的链表 */
    _device_type_t type;                /* 设备类型 */
    struct _driver_object *driver;      /* 设备所在的驱动 */
    void *device_extension;             /* 设备扩展，自定义 */
    unsigned int flags;                 /* 设备标志 */
    atomic_t reference;                 /* 引用计数，管理设备打开情况 */
    io_request_t *cur_ioreq;            /* 当前正在处理的io请求 */
    string_t name;                      /* 名字 */
    spinlock_t device_lock;             /* 设备锁，维护设备操作 */
    unsigned long reserved;             /* 预留 */
    device_queue_t device_queue;        /* 设备队列 */
} device_object_t;

/* 派遣函数定义 */ 
typedef iostatus_t (*driver_dispatch_t)(device_object_t *device, io_request_t *ioreq);

/* 驱动标准函数定义 */
typedef iostatus_t (*driver_func_t)(struct _driver_object *driver);

/* 驱动对象 */
typedef struct _driver_object
{
    unsigned int flags;                 /* 驱动标志 */
    list_t list;                        /* 驱动程序构成一个链表 */
    list_t device_list;                 /* 驱动下的设备构成的链表 */
    struct drver_extension *drver_extension; /* 驱动扩展 */
    string_t name;                      /* 名字 */
    /* 驱动控制函数 */
    driver_func_t driver_enter;
    driver_func_t driver_exit;
    
    /* 驱动派遣函数 */  
    driver_dispatch_t dispatch_function[MAX_IOREQ_FUNCTION_NR];
    spinlock_t device_lock;             /* 设备锁 */
} driver_object_t;


void init_driver_arch();

iostatus_t io_create_device(
    driver_object_t *driver,
    unsigned long device_extension_size,
    char *device_name,
    _device_type_t type,
    device_object_t **device
);

void io_delete_device(
    device_object_t *device
);

io_request_t *io_build_sync_request(
    unsigned long function,
    device_object_t *devobj,
    void *buffer,
    unsigned long length,
    unsigned long offset,
    io_status_block_t *io_status_block
);

int driver_object_delete(driver_object_t *driver);

device_object_t *io_search_device_by_name(char *name);

io_request_t *io_request_alloc();

iostatus_t io_call_dirver(device_object_t *device, io_request_t *ioreq);

void io_complete_request(io_request_t *ioreq);

handle_t device_open(char *devname, unsigned int flags);
int device_close(handle_t handle);
ssize_t device_read(handle_t handle, void *buffer, size_t length, off_t offset);
ssize_t device_write(handle_t handle, void *buffer, size_t length, off_t offset);
ssize_t device_devctl(handle_t handle, unsigned int code, unsigned long arg);

void dump_device_object(device_object_t *device);

int io_uninstall_driver(char *drvname);

#endif   /* _XBOOK_DRIVER_H */
