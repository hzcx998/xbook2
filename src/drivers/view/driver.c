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
            errprint("init view driver failed!\n");
            return IO_FAILED;
        }
    }
    view_open_count++;
    // 高16为宽度，低16位为高度
    if (extension->view == NULL) {
        extension->view = view_create(0, 0, (flags >> 16) & 0xffff, flags & 0xffff);
        if (extension->view == NULL) {
            status = IO_FAILED;
            goto end_open;
        }
        view_render_rectfill(extension->view, 0, 0, 
            extension->view->width, extension->view->height, VIEW_WHITE);
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
            errprint("view destroy failed!\n");
            goto end_close;
        }
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

static iostatus_t view_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    /* 往消息池放入消息 */
    view_t *view = extension->view;
    if (view && ioreq->parame.write.length > 0) {
        if (view_put_msg(view, ioreq->user_buffer, 
            (extension->flags & DEV_NOWAIT) > 0 ? VIEW_MSG_NOWAIT : 0) < 0)
            status = IO_FAILED;
    } else {
        status = IO_FAILED;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    iostatus_t status = IO_SUCCESS;
    view_t *view = extension->view;
    switch (ioreq->parame.devctl.code) {    
    case VIEWIO_SHOW:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_show(view);
        }
        break;
    case VIEWIO_HIDE:
        if (view == NULL) {
            status = IO_FAILED;
        } else {
            view_hide(view);
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
            view_rect_t *rect = (view_rect_t *) arg;
            if (view_try_resize(view, rect) < 0)
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
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t view_driver_enter(driver_object_t *driver)
{
    iostatus_t status;
    device_object_t *devobj;
    device_extension_t *extension;
    view_open_count = 0;
    /* 创建视图设备 */
    int i;
    char devname[DEVICE_NAME_LEN] = {0, };
    for (i = 0; i < VIEW_MAX_NR; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME, i);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIEW, &devobj);
        if (status != IO_SUCCESS) {
            view_core_exit();
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