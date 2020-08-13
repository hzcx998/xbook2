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
