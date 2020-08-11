#ifndef _XBOOK_DRIVER_H
#define _XBOOK_DRIVER_H

#include <list.h>
#include "string.h"
#include "spinlock.h"
#include "mutexlock.h"
#include "waitqueue.h"
#include <arch/atomic.h>
#include <sys/res.h>
#include <sys/input.h>

#define DRIVER_NAME_LEN 32

#ifndef DEVICE_NAME_LEN
#define DEVICE_NAME_LEN 32
#endif

/* 驱动状态：0~15位是内核使用，16~23位自定义 */
#define IO_SUCCESS             0            /* 成功 */
#define IO_FAILED              (1 << 0)    /* 失败 */        
#define IO_PENDING             (1 << 1)    /* 未决 */
#define IO_NOWAIT              (1 << 2)    /* 不需要等待 */
#define IO_KERNEL              (1 << 3)    /* 内核IO */

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
    IOREQ_MMAP,                     /* 设备内存映射派遣索引 */
    MAX_IOREQ_FUNCTION_NR
};

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
    IOREQ_MMAP_OPERATION        = (1 << 5),
    IOREQ_BUFFERED_IO           = (1 << 6),
    IOREQ_COMPLETION            = (1 << 31),    /* 完成请求 */
};

enum _device_object_flags {
    DO_BUFFERED_IO              = (1 << 0),     /* 缓冲区IO */
    DO_DIRECT_IO                = (1 << 1),     /* 直接内存IO */
    DO_DISPENSE                 = (1 << 2),     /* 分发位 */
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
        struct {
            int flags;
            size_t length;
        } mmap;
        
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

#define DEVICE_QUEUE_ENTRY_NR 12

/* 设备队列管理 */
typedef struct _device_queue {
    list_t list_head;   /* 队列列表 */
    spinlock_t lock;    /* 维护队列的锁 */
    wait_queue_t wait_queue;    /* 等待队列 */
    char entry_count;           /* 队列项数[0-DEVICE_QUEUE_ENTRY_NR] */
} device_queue_t;

/* 设备队列项 */
typedef struct _device_queue_entry {
    list_t list;                        /* 链表 */
    unsigned char *buf;                 /* 缓冲区 */
    int length;                         /* 数据长度 */
} device_queue_entry_t;

/* 设备可分发设备（网卡）可以被DEVICE_QUEUE_NR个进程同时打开使用 */
#define DEVICE_QUEUE_NR 12

/* 设备对象 */
typedef struct _device_object
{
    list_t list;                        /* 设备在驱动中的链表 */
    device_type_t type;                /* 设备类型 */
    struct _driver_object *driver;      /* 设备所在的驱动 */
    void *device_extension;             /* 设备扩展，自定义 */
    unsigned int flags;                 /* 设备标志 */
    atomic_t reference;                 /* 引用计数，管理设备打开情况 */
    io_request_t *cur_ioreq;            /* 当前正在处理的io请求 */
    string_t name;                      /* 名字 */
    struct {
        spinlock_t spinlock;            /* 设备自旋锁 */
        mutexlock_t mutexlock;          /* 设备互斥锁 */
    } lock;
    unsigned long reserved;             /* 预留 */
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
    device_type_t type,
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

void io_device_queue_init(device_queue_t *queue);

iostatus_t io_device_queue_append(
    device_queue_t *queue, 
    unsigned char *buf,
    int len
);

int io_device_queue_pickup(
    device_queue_t *queue,
    unsigned char *buf,
    int buflen,
    int flags
);

handle_t device_open(char *devname, unsigned int flags);
int device_close(handle_t handle);
ssize_t device_read(handle_t handle, void *buffer, size_t length, off_t offset);
ssize_t device_write(handle_t handle, void *buffer, size_t length, off_t offset);
ssize_t device_devctl(handle_t handle, unsigned int code, unsigned long arg);
int device_grow(handle_t handle);
void *device_mmap(handle_t handle, size_t length, int flags);

void dump_device_object(device_object_t *device);

int io_uninstall_driver(char *drvname);

device_object_t *io_iterative_search_device_by_type(device_object_t *devptr, device_type_t type);
int sys_devscan(devent_t *de, device_type_t type, devent_t *out);

/* 事件缓冲区大小，事件个数 */
#define EVBUF_SIZE        64

/* 输入事件缓冲区 */
typedef struct _input_even_buf {
    input_event_t evbuf[EVBUF_SIZE];       /* 事件输入缓冲区 */
    int head, tail;                        /* 输入输出时的指针 */
    spinlock_t lock;                    /* 自旋锁来保护写入和读取操作 */
} input_even_buf_t;

int input_even_init(input_even_buf_t *evbuf);
int input_even_put(input_even_buf_t *evbuf, input_event_t *even);
int input_even_get(input_even_buf_t *evbuf, input_event_t *even);

#endif   /* _XBOOK_DRIVER_H */
