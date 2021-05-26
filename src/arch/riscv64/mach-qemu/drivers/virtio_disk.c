// This driver is from xv6-riscv
//
// driver for qemu's virtio disk device.
// uses qemu's mmio interface to virtio.
// qemu presents a "legacy" virtio interface.
//
// qemu ... -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
//

#ifdef QEMU

#include <xbook/debug.h>
#include <arch/memory.h>
#include <string.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <xbook/hardirq.h>
#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <drivers/virtio.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>

#define DRV_NAME "virtio-disk"
#define DRV_VERSION "0.1"

#define DEV_NAME "sda"
#define DRV_PREFIX  "[virtio disk] "

/* assume 128MB */
#define VIRTIO_DISK_SECTORS     (262144)

#define VIRTIO_DISK_PAGES       2

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

// the address of virtio mmio register r.
#define R(ext, r) ((volatile uint32_t *)(((ext)->memio_addr) + (r)))

// #define DEBUG_DRV

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    unsigned long sectors;          /* 磁盘扇区数 */
    unsigned long rwoffset;         // 读写偏移位置
    uint8_t *memio_addr;            /* 磁盘内存映射地址 */

    // the virtio driver and device mostly communicate through a set of
    // structures in RAM. pages[] allocates that memory. 
    char *pages;    // two page size buf
    
    // pages[] is divided into three regions (descriptors, avail, and
    // used), as explained in Section 2.6 of the virtio specification
    // for the legacy interface.
    // https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
    
    // the first region of pages[] is a set (not a ring) of DMA
    // descriptors, with which the driver tells the device where to read
    // and write individual disk operations. there are NUM descriptors.
    // most commands consist of a "chain" (a linked list) of a couple of
    // these descriptors.
    // points into pages[].
    struct virtq_desc *desc;

    // next is a ring in which the driver writes descriptor numbers
    // that the driver would like the device to process.  it only
    // includes the head descriptor of each chain. the ring has
    // NUM elements.
    // points into pages[].
    struct virtq_avail *avail;

    // finally a ring in which the device writes descriptor numbers that
    // the device has finished processing (just the head of each chain).
    // there are NUM used ring entries.
    // points into pages[].
    struct virtq_used *used;

    // our own book-keeping.
    char free[NUM];  // is a descriptor free?
    uint16_t used_idx; // we've looked this far in used[2..NUM].

    // track info about in-flight operations,
    // for use when completion interrupt arrives.
    // indexed by first descriptor index of chain.
    struct {
        //struct buf *b;
        int disk;    // does disk "own" buf?
        task_t *task;   // which task request opteration? 
        char status;
    } info[NUM];

    // disk command headers.
    // one-for-one with descriptors, for convenience.
    struct virtio_blk_req ops[NUM];
    
    spinlock_t vdisk_lock;
  
} device_extension_t;


// find a free descriptor, mark it non-free, return its index.
static int
alloc_desc(device_extension_t *extension)
{
  for(int i = 0; i < NUM; i++){
    if(extension->free[i]){
      extension->free[i] = 0;
      return i;
    }
  }
  return -1;
}

// mark a descriptor as free.
static void
free_desc(device_extension_t *extension, int i)
{
  if(i >= NUM)
    panic("free_desc 1");
  if(extension->free[i])
    panic("free_desc 2");
  extension->desc[i].addr = 0;
  extension->desc[i].len = 0;
  extension->desc[i].flags = 0;
  extension->desc[i].next = 0;
  extension->free[i] = 1;
  // wakeup(&extension->free[0]);
}

// free a chain of descriptors.
static void
free_chain(device_extension_t *extension, int i)
{
  while(1){
    int flag = extension->desc[i].flags;
    int nxt = extension->desc[i].next;
    free_desc(extension, i);
    if(flag & VRING_DESC_F_NEXT)
      i = nxt;
    else
      break;
  }
}

// allocate three descriptors (they need not be contiguous).
// disk transfers always use three descriptors.
static int
alloc3_desc(device_extension_t *extension, int *idx)
{
  for(int i = 0; i < 3; i++){
    idx[i] = alloc_desc(extension);
    if(idx[i] < 0){
      for(int j = 0; j < i; j++)
        free_desc(extension, idx[j]);
      return -1;
    }
  }
  return 0;
}

static void virtio_disk_rw(device_extension_t *extension, 
        unsigned long lba, void *data, int write)
{
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"virtio_disk_rw: lba=%lx data=%p, rw=%d [START]", lba, data, write);
    #endif
    uint64_t sector = lba;

    unsigned long iflags;
    spin_lock_irqsave(&extension->vdisk_lock, iflags);

    // the spec's Section 5.2 says that legacy block operations use
    // three descriptors: one for type/reserved/sector, one for the
    // data, one for a 1-byte status result.

    // allocate the three descriptors.
    int idx[3];
    while(1){
        if(alloc3_desc(extension, idx) == 0) {
            break;
        }
        /* wait for desc avaliable */
        // sleep(&extension->free[0], &extension->vdisk_lock);

        // unlock & sleep & lock
        spin_unlock_irqrestore(&extension->vdisk_lock, iflags);
        task_yield();
        spin_lock_irqsave(&extension->vdisk_lock, iflags);
    }

    // format the three descriptors.
    // qemu's virtio-blk.c reads them.

    struct virtio_blk_req *buf0 = &extension->ops[idx[0]];

    if(write)
        buf0->type = VIRTIO_BLK_T_OUT; // write the disk
    else
        buf0->type = VIRTIO_BLK_T_IN; // read the disk
    buf0->reserved = 0;
    buf0->sector = sector;

    extension->desc[idx[0]].addr = (uint64_t) buf0;
    extension->desc[idx[0]].len = sizeof(struct virtio_blk_req);
    extension->desc[idx[0]].flags = VRING_DESC_F_NEXT;
    extension->desc[idx[0]].next = idx[1];

    extension->desc[idx[1]].addr = (uint64_t) data;
    extension->desc[idx[1]].len = SECTOR_SIZE;
    if(write)
        extension->desc[idx[1]].flags = 0; // device reads data
    else
        extension->desc[idx[1]].flags = VRING_DESC_F_WRITE; // device writes data
    extension->desc[idx[1]].flags |= VRING_DESC_F_NEXT;
    extension->desc[idx[1]].next = idx[2];

    extension->info[idx[0]].status = 0xff; // device writes 0 on success
    extension->desc[idx[2]].addr = (uint64_t) &extension->info[idx[0]].status;
    extension->desc[idx[2]].len = 1;
    extension->desc[idx[2]].flags = VRING_DESC_F_WRITE; // device writes the status
    extension->desc[idx[2]].next = 0;

    // set 'own' to disk
    // record struct buf for virtio_disk_intr().
    extension->info[idx[0]].disk = 1;
    extension->info[idx[0]].task = task_current;    

    // tell the device the first index in our chain of descriptors.
    extension->avail->ring[extension->avail->idx % NUM] = idx[0];

    mb();

    // tell the device another avail ring entry is available.
    extension->avail->idx += 1; // not % NUM ...

    mb();

    *R(extension, VIRTIO_MMIO_QUEUE_NOTIFY) = 0; // value is queue number
    // dbgprintln(DRV_PREFIX"virtio_disk_rw: notify ok, wait for intr");

    // Wait for virtio_disk_intr() to say request has finished.
    while(extension->info[idx[0]].disk == 1) {
        // sleep(b, &extension->vdisk_lock);
        spin_unlock_irqrestore(&extension->vdisk_lock, iflags);
        task_yield();
        spin_lock_irqsave(&extension->vdisk_lock, iflags);

        // unlock & sleep & lock

    }
    extension->info[idx[0]].task = NULL;
    free_chain(extension, idx[0]);

    spin_unlock_irqrestore(&extension->vdisk_lock, iflags);
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"virtio_disk_rw: lba=%lx data=%p, rw=%d [END]]", lba, data, write);
    #endif
}


static iostatus_t virtio_disk_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off;    
    if (ioreq->parame.read.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.read.offset;
    }
    unsigned long length = ioreq->parame.read.length;
    size_t sectors = DIV_ROUND_UP(length, PAGE_SIZE);
    char *data = (char *)ioreq->system_buffer;
    /* 判断越界 */
    if (off + sectors  >= extension->sectors) {
		status = IO_FAILED;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "virtio_disk_read: read disk offset=%d counts=%d failed!\n",
            off, sectors);
#endif

	} else {
		/* 进行磁盘读取 */
		int i;
        for (i = 0; i < sectors; i++) {
            virtio_disk_rw(extension, off + i, data + i * SECTOR_SIZE, 0);
        }

        ioreq->io_status.infomation = length;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "virtio_disk_read: read disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif

	}
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "virtio_disk_read: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t virtio_disk_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off;    
    if (ioreq->parame.write.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.write.offset;
    }
    unsigned long length = ioreq->parame.write.length;
    size_t sectors = DIV_ROUND_UP(length, PAGE_SIZE);
    char *data = (char *)ioreq->system_buffer;
    /* 判断越界 */
    if (off + sectors  >= extension->sectors) {
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "virtio_disk_write: write disk offset=%d counts=%d failed!\n",
            off, sectors);
#endif
		status = IO_FAILED;
	} else {

		/* 进行磁盘写入 */
		int i;
        for (i = 0; i < sectors; i++) {
            virtio_disk_rw(extension, off + i, data + i * SECTOR_SIZE, 1);
        }
        
        ioreq->io_status.infomation = length;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "virtio_disk_write: write disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif
	}
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "virtio_disk_write: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t virtio_disk_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    unsigned long off;
    iostatus_t status = IO_SUCCESS;
    switch (ioreq->parame.devctl.code)
    {    
    case DISKIO_GETSIZE:
        *((unsigned int *) arg) = extension->sectors; 
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "virtio_disk_devctl: get disk sectors=%d\n", extension->sectors);
#endif
        break;
    case DISKIO_SETOFF:
        off = *((unsigned long *) arg);
        if (off > extension->sectors - 1)
            off = extension->sectors - 1;
        extension->rwoffset = off;
        break;
    case DISKIO_GETOFF:
        *((unsigned long *) arg) = extension->rwoffset;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static int virtio_disk_intr(irqno_t irqno, void *data)
{
    device_extension_t *extension = (device_extension_t *) data; 
    
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"virtio_disk_intr: interrupt occur!");
    #endif
    
    unsigned long iflags;
    spin_lock_irqsave(&extension->vdisk_lock, iflags);

    // the device won't raise another interrupt until we tell it
    // we've seen this interrupt, which the following line does.
    // this may race with the device writing new entries to
    // the "used" ring, in which case we may process the new
    // completion entries in this interrupt, and have nothing to do
    // in the next interrupt, which is harmless.
    *R(extension, VIRTIO_MMIO_INTERRUPT_ACK) = *R(extension, VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3;

    mb();

    // the device increments extension->used->idx when it
    // adds an entry to the used ring.

    while(extension->used_idx != extension->used->idx){
        mb();
        int id = extension->used->ring[extension->used_idx % NUM].id;

        if(extension->info[id].status != 0) {
            errprintln(DRV_PREFIX "virtio_disk_intr status");
            return IRQ_NEXTONE;
        }
        /* 唤醒在缓冲区上面等待的任务，并且让该任务可以读取数据 */
        extension->info[id].disk = 0;
        #ifdef DEBUG_DRV
        dbgprintln(DRV_PREFIX"virtio_disk_intr: ID %d xmit done.", id);
        #endif
        extension->used_idx += 1;
    }

    spin_unlock_irqrestore(&extension->vdisk_lock, iflags);
    
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"virtio_disk_intr: interrupt finished!");
    #endif
    
    return IRQ_HANDLED;
}

static int virtio_disk_init(device_extension_t *extension)
{
    spinlock_init(&extension->vdisk_lock);
    
    /* 内存映射到内核虚拟地址 */
    extension->memio_addr = memio_remap(VIRTIO0, PAGE_SIZE);
    if (extension->memio_addr == NULL) {
        errprintln(DRV_PREFIX"io memory addr %p remap to kernel failed!", VIRTIO0);
        return -1;
    }
    dbgprintln(DRV_PREFIX"memory io addr %p\n", extension->memio_addr);
    /* 检测磁盘是否存在 */
    if(*R(extension, VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        *R(extension, VIRTIO_MMIO_VERSION) != 1 ||
        *R(extension, VIRTIO_MMIO_DEVICE_ID) != 2 ||
        *R(extension, VIRTIO_MMIO_VENDOR_ID) != 0x554d4551){
        errprintln(DRV_PREFIX"could not find virtio disk");
        return -1;
    }
    uint32_t status = 0;
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    *R(extension, VIRTIO_MMIO_STATUS) = status;

    status |= VIRTIO_CONFIG_S_DRIVER;
    *R(extension, VIRTIO_MMIO_STATUS) = status;

    // negotiate features
    uint64_t features = *R(extension, VIRTIO_MMIO_DEVICE_FEATURES);
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
    *R(extension, VIRTIO_MMIO_DRIVER_FEATURES) = features;

    // tell device that feature negotiation is complete.
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    *R(extension, VIRTIO_MMIO_STATUS) = status;

    // tell device we're completely ready.
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    *R(extension, VIRTIO_MMIO_STATUS) = status;

    *R(extension, VIRTIO_MMIO_GUEST_PAGE_SIZE) = PAGE_SIZE;

    // initialize queue 0.
    *R(extension, VIRTIO_MMIO_QUEUE_SEL) = 0;
    uint32_t max = *R(extension, VIRTIO_MMIO_QUEUE_NUM_MAX);
    if(max == 0) {
        errprintln(DRV_PREFIX"virtio disk has no queue 0");
        return -1;
    }
    if(max < NUM) {
        errprintln(DRV_PREFIX"virtio disk max queue too short");
        return -1;
    }
    *R(extension, VIRTIO_MMIO_QUEUE_NUM) = NUM;

    extension->pages = kern_phy_addr2vir_addr(page_alloc_normal(VIRTIO_DISK_PAGES)); 
    if (extension->pages == NULL) {
        errprintln(DRV_PREFIX"alloc page for pages buf failed!");
        return -1;
    }
    memset(extension->pages, 0, VIRTIO_DISK_PAGES * PAGE_SIZE);
    *R(extension, VIRTIO_MMIO_QUEUE_PFN) = ((uint64_t) extension->pages) >> PAGE_SHIFT;

    // desc = pages -- num * virtq_desc
    // avail = pages + 0x40 -- 2 * uint16, then num * uint16
    // used = pages + 4096 -- 2 * uint16, then num * vRingUsedElem

    extension->desc = (struct virtq_desc *) extension->pages;
    extension->avail = (struct virtq_avail *)(extension->pages + NUM*sizeof(struct virtq_desc));
    extension->used = (struct virtq_used *) (extension->pages + PAGE_SIZE);

    // all NUM descriptors start out unused.
    int i;
    for(i = 0; i < NUM; i++)
        extension->free[i] = 1;

    // Register interrupt
    if (irq_register(VIRTIO0_IRQ, virtio_disk_intr, IRQF_DISABLED,
        "virtio0", DEV_NAME, extension) < 0) 
    {
        errprintln(DRV_PREFIX"irq %d register failed!", VIRTIO0_IRQ);
        page_free((unsigned long)extension->pages);
        return -1; 
    }
    dbgprintln(DRV_PREFIX"disk init done.");
    return 0;
}

static iostatus_t virtio_disk_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_VIRTUAL_DISK, &devobj);
    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "virtio_disk_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = DO_BUFFERED_IO;
    extension = (device_extension_t *)devobj->device_extension;
    extension->device_object = devobj;
    extension->sectors = VIRTIO_DISK_SECTORS;
    extension->rwoffset = 0;

    if (virtio_disk_init(extension) < 0) {
        io_delete_device(devobj);
        status = IO_FAILED; 
    }
    // spin("test");
    return status;
}


static iostatus_t virtio_disk_exit(driver_object_t *driver)
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

static iostatus_t virtio_disk_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = virtio_disk_enter;
    driver->driver_exit = virtio_disk_exit;

    driver->dispatch_function[IOREQ_READ] = virtio_disk_read;
    driver->dispatch_function[IOREQ_WRITE] = virtio_disk_write;
    driver->dispatch_function[IOREQ_DEVCTL] = virtio_disk_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "virtio_disk_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void virtio_disk_driver_entry(void)
{
    if (driver_object_create(virtio_disk_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(virtio_disk_driver_entry);
#endif