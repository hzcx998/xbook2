#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <assert.h>
#include <xbook/byteorder.h>
#include <xbook/spinlock.h>
#include <math.h>
#include <xbook/waitqueue.h>
#include <xbook/kmalloc.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <xbook/pci.h>
#include <arch/atomic.h>
#include <arch/cpu.h>
#include <sys/ioctl.h>
#include <stddef.h>

#include <net/e1000_hw.h>
#include <net/e1000_osdep.h>
#include <net/e1000.h>
#include <net/kcompat.h>

#define DRV_VERSION "v0.1"
#define DRV_NAME "net-e1000" DRV_VERSION

#define DEV_NAME "e1000"

#define DEBUG_LOCAL 0

/*以太网的情况*/
#define ETH_ALEN 6 /*以太网地址，即MAC地址，6字节*/
#define ETH_ZLEN 60 /*不含CRC校验的数据最小长度*/
#define ETH_DATA_LEN 1500 /*帧内数据的最大长度*/
#define ETH_FRAME_LEN 1514 /*不含CRC校验和的最大以太网数据长度*/

/*  设备状态 */
struct net_device_status {
    /* 错误记录 */
    unsigned long tx_errors;         /* 传输错误记录 */
    unsigned long tx_aborted_errors;  /* 传输故障记录 */
    unsigned long tx_carrier_errors;  /* 传输携带错误记录 */
    unsigned long tx_window_errors;   /* 传输窗口错误记录 */
    unsigned long tx_fifo_errors;     /* 传输fifo错误记录 */
    unsigned long tx_dropped;        /* 传输丢弃记录 */

    unsigned long rx_errors;         /* 接收错误 */    
    unsigned long rx_length_errors;   /* 接收长度错误 */
    unsigned long rx_missed_errors;   /* 接收丢失错误 */
    unsigned long rx_fifo_errors;     /* 接收Fifo错误 */
    unsigned long rx_crc_errors;      /* 接收CRC错误 */
    unsigned long rx_frame_errors;    /* 接收帧错误 */
    unsigned long rx_dropped;        /* 接收丢弃记录 */
    /* 正确记录 */
    unsigned long tx_packets;        /* 传输包数量 */
    unsigned long tx_bytes;          /* 传输字节数量 */
    
    unsigned long collisions;       /* 碰撞次数 */
};

typedef struct _device_extension {
    device_object_t *device_object;   //设备对象

    uint32_t io_addr;   //IO基地址
    int drv_flags;   //驱动标志
    uint32_t irq;
    uint8_t mac_addr[6];
    flags_t flags;
    pci_device_t *pci_device;

    uint32_t dev_features;   //设备结构特征

    uint8_t *rx_buffer;   //接受缓冲区
    uint8_t *rx_ring;   //接受环
    uint8_t current_rx;
    flags_t rx_flags;
    dma_addr_t rx_ring_dma;   //dma物理地址
};