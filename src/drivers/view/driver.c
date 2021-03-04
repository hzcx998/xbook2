#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <xbook/initcall.h>
#include <xbook/safety.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include <drivers/view/core.h>
#include <drivers/view/view.h>
#include <drivers/view/render.h>
#include <drivers/view/bitmap.h>
#include <drivers/view/msg.h>
#include <drivers/view/env.h>
#include <drivers/view/mouse.h>
#include <drivers/view/screen.h>


#define DRV_NAME "view"
#define DRV_VERSION "0.1"

#define DEV_NAME "view"

typedef struct _device_extension {
    view_t *view;
    unsigned long flags;
} device_extension_t;

static int view_open_count;

static iostatus_t view_open(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    int flags = ioreq->parame.open.flags;
    if (!view_open_count) {
        if (view_core_init() < 0) {
            errprint("view driver: init view driver core failed!\n", device->name.text);
            return IO_FAILED;
        }
    }
    view_open_count++;
    // 0~12:宽度，13~26：高度，27~31：type
    if (extension->view == NULL) {
        extension->view = view_create(0, 0, (flags >> 12) & 0x1fff, flags & 0x1fff, (flags >> 26) & 0x1f);
        if (extension->view == NULL) {
            status = IO_FAILED;
            goto end_open;
        }
        view_env_send_to_monitor(extension->view, VIEW_MSG_CREATE);
    }

end_open:
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_close(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    if (extension->view) {
        int z = extension->view->z;
        view_hide(extension->view);
        if (view_destroy(extension->view) < 0) {
            view_set_z(extension->view, z);
            status = IO_FAILED;
            errprint("view driver: view close %s failed!\n", device->name.text);
            goto end_close;
        }
        view_env_send_to_monitor(extension->view, VIEW_MSG_CLOSE);
        view_env_reset_hover_and_activity();
        extension->view = NULL;
    }
    extension->flags = 0;
    --view_open_count;
    if (view_open_count <= 0) {
        if (view_core_exit() < 0) {
            errprint("init view driver failed!\n");
            status = IO_FAILED;
        }
        view_open_count = 0;
    }
end_close:
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    /* 从消息池读取消息 */
    view_t *view = extension->view;
    if (view && ioreq->parame.read.length > 0) {
        if (view_get_msg(view, ioreq->user_buffer, 
            (extension->flags & DEV_NOWAIT) > 0 ? VIEW_MSG_NOWAIT : 0) < 0)
            status = IO_FAILED;
    } else {
        status = IO_FAILED;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_fastread(device_object_t *device, size_t size, void *buf)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    view_t *view = extension->view;
    if (view && size > 0) {
        if (view_get_msg(view, buf, 
            (extension->flags & DEV_NOWAIT) > 0 ? VIEW_MSG_NOWAIT : 0) < 0)
            status = IO_FAILED;
    } else {
        status = IO_FAILED;
    }
    return status;
}

static int __view_write_msg(view_msg_t *msg, int flags)
{
    if (msg->target == VIEW_TARGET_NONE) {
        errprint("view target %d none!\n", msg->target);
        return -1;        
    }
    view_t *view = view_find_by_id(msg->target);
    if (!view) {
        errprint("view target %d not found!\n", msg->target);
        return -1;
    }
    /* 转发-发送消息 */
    return view_put_msg(view, msg, flags);
}

static iostatus_t view_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    /* 往消息池放入消息 */
    view_t *view = extension->view;
    if (view && ioreq->parame.write.length > 0) {
        if (__view_write_msg(ioreq->user_buffer, 
            (extension->flags & DEV_NOWAIT) > 0 ? VIEW_MSG_NOWAIT : 0) < 0)
            status = IO_FAILED;
    } else {
        status = IO_FAILED;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_fastwrite(device_object_t *device, size_t size, void *buf)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    view_t *view = extension->view;
    if (view && size > 0) {
        if (__view_write_msg(buf, 
            (extension->flags & DEV_NOWAIT) > 0 ? VIEW_MSG_NOWAIT : 0) < 0)
            status = IO_FAILED;
    } else {
        status = IO_FAILED;
    }
    return status;
}

static iostatus_t view_mmap(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    view_t *view = extension->view;
    if (view) {
        ioreq->io_status.infomation = 0;
        /* 检测参数大小 */
        if (ioreq->parame.mmap.length <= view_get_vram_size(view)) {
            void *addr = view_get_vram_start(view);
            if (!addr) {
                status = IO_FAILED;
            } else {
                unsigned long paddr = addr_vir2phy((unsigned long) addr);
                ioreq->io_status.infomation = (unsigned long) paddr;     /* 返回物理地址 */        
            }
        }
    } else {
        status = IO_FAILED;
    }
    
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t __view_ioctl(device_extension_t *extension, int cmd, void *arg)
{
    iostatus_t status = IO_SUCCESS;
    view_t *view = extension->view;
    switch (cmd) {    
    case VIEWIO_SHOW:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (!view_show(view)) {
                view_env_send_to_monitor(view, VIEW_MSG_SHOW);
                view_env_try_activate(view);
            }
        }
        break;
    case VIEWIO_HIDE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (!view_hide(view)) {
                view_env_send_to_monitor(view, VIEW_MSG_HIDE);
                view_env_reset_hover_and_activity();
            }
        }
        break;
    case VIEWIO_SETPOS:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned long pos = * (unsigned long *) arg;
            // 高16位为x，低16位为y
            view_set_xy(view, (pos >> 16) & 0xffff, pos & 0xffff);
        }
        break;
    case VIEWIO_GETPOS:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned long pos = (view->x << 16) | view->y;
            * (unsigned long *) arg = pos;
        }
        break;
    case VIEWIO_SETFLGS:
        if (mem_copy_from_user(&extension->flags, (void *)arg, sizeof(unsigned long)) < 0)
            status = IO_FAILED;
        break;
    case VIEWIO_GETFLGS:
        if (mem_copy_to_user((void *)arg, &extension->flags, sizeof(unsigned long)) < 0)
            status = IO_FAILED;
        break;
    case VIEWIO_WRBMP:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            uview_io_t *vio = (uview_io_t *) arg;
            view_render_bitblt(view, vio->x, vio->y, (view_bitmap_t *)&vio->bmp, vio->bx, vio->by, vio->bw, vio->bh);
            if (vio->refresh)
                view_refresh(view, vio->x, vio->y, vio->x + vio->bw, vio->y + vio->bh);
        }
        break;
    case VIEWIO_SETTYPE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (view_set_type(view, *(int *)arg) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_GETTYPE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            *(int *)arg = view_get_type(view);
        }
        break;
    case VIEWIO_REFRESH:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_region_t *vreg = (view_region_t *) arg;
            view_refresh(view, vreg->left, vreg->top, vreg->right, vreg->bottom);
        }
        break;
    case VIEWIO_ADDATTR:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (view_add_attr(view, *(int *)arg) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_DELATTR:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (view_del_attr(view, *(int *)arg) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_RESIZE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned int vsize = *(unsigned int *)arg;
            if (view_env_try_resize_ex(view, (vsize >> 16) & 0xffff, vsize & 0xffff) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_GETSCREENSZ:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            *(int *)arg = view_env_get_screensize();
        }
        break;
    case VIEWIO_GETLASTPOS:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            *(unsigned int *)arg = view_env_get_lastpos();
        }
        break;
    case VIEWIO_GETMOUSEPOS:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            *(unsigned int *)arg = view_env_get_mousepos();
        }
        break;
    case VIEWIO_SETSIZEMIN:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned int size_min = *(unsigned int *)arg;
            view_set_size_min(view, (size_min >> 16) & 0xffff, size_min & 0xffff);
        }
        break;
    case VIEWIO_SETDRAGREGION:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_region_t *vreg = (view_region_t *) arg;
            view_set_drag_region(view, vreg);
        }
        break;
    case VIEWIO_SETMOUSESTATE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_mouse_state_t state = *(view_mouse_state_t *) arg;
            view_mouse_set_state(state);
        }
        break;
    case VIEWIO_SETMOUSESTATEINFO:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_mouse_state_info_t *sinfo = (view_mouse_state_info_t *) arg;
            if (view_mouse_set_state_info_ex(sinfo) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_GETVID:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            *(unsigned int *)arg = view->id; 
        }
        break;
    case VIEWIO_ADDTIMER:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned long interval = *(unsigned long *) arg;
            int timer_id = view_env_add_timer(view, interval);
            if (timer_id < 0)
                status = IO_FAILED;
            // 回写timer id
            *(unsigned long *) arg = timer_id;
        }
        break;
    case VIEWIO_DELTIMER:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            unsigned long timer_id = *(unsigned long *) arg;
            if (view_env_del_timer(view, timer_id) < 0)
                status = IO_FAILED;
        }
        break;
    case VIEWIO_RESTARTTIMER:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            viewio_timer_t *vio_timer = (viewio_timer_t *) arg;
            if (view_env_restart_timer(view, vio_timer->timer_id, vio_timer->interval) < 0)
                status = IO_FAILED;
        }
        break;   
    case VIEWIO_SETMONITOR:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            if (view_env_set_monitor(view, *(int *)arg) < 0)
                status = IO_FAILED;
        }
        break;
    default:
        status = IO_FAILED;
        break;
    }
    return status;
}

static iostatus_t view_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    void *arg = (void *) ioreq->parame.devctl.arg;
    ioreq->io_status.status = __view_ioctl(extension, ioreq->parame.devctl.code, arg);
    io_complete_request(ioreq);
    return ioreq->io_status.status;
}

static iostatus_t view_fastio(device_object_t *device, int cmd, void *arg)
{
    device_extension_t *extension = device->device_extension;
    return __view_ioctl(extension, cmd, arg);
}


static iostatus_t view_driver_enter(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    device_object_t *devobj;
    device_extension_t *extension;
    view_open_count = 0;
    
    // 初始化屏幕
    if (view_screen_init() < 0) {
        status = IO_FAILED;
        return status;
    }

    /* 创建视图设备 */
    int i;
    char devname[DEVICE_NAME_LEN] = {0, };
    for (i = 0; i < VIEW_MAX_NR; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME, i);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIEW, &devobj);
        if (status != IO_SUCCESS) {
            errprint("view driver: create device failed!\n");
            return status;
        }
        /* neighter io mode */
        devobj->flags = 0;    
        extension = (device_extension_t *)devobj->device_extension;
        extension->view = NULL;
        extension->flags = 0;
    }
    return status;
}

static iostatus_t view_driver_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

static iostatus_t view_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = view_driver_enter;
    driver->driver_exit = view_driver_exit;

    driver->dispatch_function[IOREQ_OPEN] = view_open;
    driver->dispatch_function[IOREQ_CLOSE] = view_close;
    driver->dispatch_function[IOREQ_READ] = view_read;
    driver->dispatch_function[IOREQ_WRITE] = view_write;
    driver->dispatch_function[IOREQ_DEVCTL] = view_devctl;
    driver->dispatch_function[IOREQ_FASTIO] = (void *) view_fastio;
    driver->dispatch_function[IOREQ_FASTREAD] = (void *) view_fastread;
    driver->dispatch_function[IOREQ_FASTWRITE] = (void *) view_fastwrite;
    driver->dispatch_function[IOREQ_MMAP] = view_mmap;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);

    return status;
}

static __init void view_driver_entry(void)
{
    if (driver_object_create(view_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

filter_initcall(view_driver_entry);