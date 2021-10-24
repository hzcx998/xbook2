
#ifndef __SOUND_AC97_CODEC_H
#define __SOUND_AC97_CODEC_H
#include <stdio.h>
#include<stddef.h>
#include <xbook/debug.h>
#include <string.h>
#include <xbook/bitops.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/io.h>
#include <xbook/hardirq.h>
#include <sys/ioctl.h>

#define MAX_AC97_WAV_BUF_SIZE 0xfffe*32*2
#define AC97_VENDOR_ID 0x8086
#define AC97_AUDIO_DEVICE_ID 0x2415  /*音频*/

/*配置空间的BARs */
#define AC97_COMMAD  0X04 
#define AC97_NAMBAR  0x10  /* Native Audio Mixer Base Address Register */
#define AC97_NABMBAR 0x14  /* Native Audio Bus Mastering Base Address Register */

/* Bus mastering IO port offsets */
#define AC97_PO_BDBAR 0x10  /* PCM out buffer descriptor BAR */ 
#define AC97_PO_CIV   0x14  /* PCM out current index value */
#define AC97_PO_LVI   0x15  /* PCM out last valid index */
#define AC97_PO_SR    0x16  /* PCM out status register */
#define AC97_PO_PICB  0x18  /* PCM out position in current buffer register */
#define AC97_PO_CR    0x1B  /* PCM out control register */

#define AC97_PI_BDBAR 0X00  /*PCM In Buffer Descriptor list Base Address Register*/
#define AC97_PI_BDBAR 0X20  /*Mic. In Buffer Descriptor list Base Address Register*/

#define AC97_OFSET_CIV   0x04
#define AC97_OFSET_LVI   0x05
#define AC97_OFSET_SR    0x06
#define AC97_OFSET_PICB  0x08
#define AC97_OFSET_CR    0x0B

#define AC97_GLB_CTRL 0X2C  /*Global Control*/
#define AC97_GLO_STAT 0X30  /*Global Status*/

/* Bus mastering misc */
/* BDL */
#define AC97_BDL_LEN              32                    /* Buffer descriptor list length */
#define AC97_BDL_BUFFER_LEN       0x1000               /* Length of buffer in BDL */
#define AC97_CL_GET_LENGTH(cl)    ((cl) & 0xFFFF)       /* Decode length from cl */
#define AC97_CL_SET_LENGTH(cl, v) ((cl) = (v) & 0xFFFF) /* Encode length to cl */
#define AC97_CL_BUP               ((uint32_t)1 << 30)             /* Buffer underrun policy in cl */
#define AC97_CL_IOC               ((uint32_t)1 << 31)             /* Interrupt on completion flag in cl */

/* PCM out control register flags */
#define AC97_X_CR_RPBM  (1 << 0)  /* Run/pause bus master */
#define AC97_X_CR_RR    (1 << 1)  /* Reset registers */
#define AC97_X_CR_LVBIE (1 << 2)  /* Last valid buffer interrupt enable */
#define AC97_X_CR_FEIE  (1 << 3)  /* FIFO error interrupt enable */
#define AC97_X_CR_IOCE  (1 << 4)  /* Interrupt on completion enable */

/* Status register flags */
#define AC97_X_SR_DCH   (1 << 0)  /* DMA controller halted */
#define AC97_X_SR_CELV  (1 << 1)  /* Current equals last valid */
#define AC97_X_SR_LVBCI (1 << 2)  /* Last valid buffer completion interrupt */
#define AC97_X_SR_BCIS  (1 << 3)  /* Buffer completion interrupt status */
#define AC97_X_SR_FIFOE (1 << 4)  /* FIFO error */


/* Mixer IO port offsets */
#define AC97_RESET          0x00
#define AC97_MASTER_VOLUME  0x02
#define AC97_HEADPHONE_VOLUME 0x04
#define AC97_MONO_VOLUME    0x06
#define AC97_PCM_OUT_VOLUME 0x18
#define AC97_PC_BEEP_VOLUME 0x0a
#define AC97_EXT_AUDIO      0x28
#define AC97_EXT_AUDIO_STC  0x2A
#define AC97_PCM_FRONT_DAC_RATE 0X2C
#define AC97_PCM_LR_ADC_RATE 0X32

//------------------------------------------------------------------
/* ac97->scaps */
#define AC97_SCAP_AUDIO		(1<<0)	/* audio codec 97 */
#define AC97_SCAP_MODEM		(1<<1)	/* modem codec 97 */

typedef struct snd_ac97_buf
{
	/* data */
	uint16_t b;

}snd_ac97_buf_t;


typedef struct snd_ac97 {
	void *private_data;
	char name[256];  
	snd_ac97_buf_t ac97_buf;
	struct snd_ac97_bus *bus;

	unsigned short num;	  /*  0 primary, 1 secondary */
	unsigned short addr;	/*四个codec的地址 */
	unsigned int id;	/* id of codec */
	unsigned int scaps;	/*设备分类，定义ac97设备是audio还是modem  */
	unsigned int flags;	/*编解码器的分类，是master，vc，pc等  */
	unsigned short regs[0x80]; /* 寄存器缓存 */
}snd_ac97_t;

typedef struct snd_ac97_bus_ops {
	void (*reset) (struct snd_ac97 *ac97);
	void (*warm_reset)(struct snd_ac97 *ac97);
	void (*write) (struct snd_ac97 *ac97, unsigned short reg, unsigned short val);
	unsigned short (*read) (struct snd_ac97 *ac97, unsigned short reg);
	void (*wait) (struct snd_ac97 *ac97);
	void (*init) (struct snd_ac97 *ac97);
}snd_ac97_bus_ops_t;

typedef struct snd_ac97_bus {
	struct snd_ac97_bus_ops *ops;
	unsigned short num;	/* bus number */
	unsigned int clock;	/* AC'97 base clock  */
	spinlock_t bus_lock;	/* used mainly for slot allocation */
	unsigned short used_slots[2][4]; /* PCM slots */
	unsigned short pcms_count; /* count of PCMs */
	struct snd_ac97 *codec[4];
}snd_ac97_bus_t;

typedef struct ac97_bdl_entry{
	uint32_t pointer;  /* Pointer to buffer */
	uint32_t cl;       /* Control values and buffer length */
}ac97_bdl_entry_t;


typedef struct ac97_device
{
	uint8_t hw_addr;  //内存基地址
    uint32_t io_addr;   //IO基地址
    int drv_flags;   //驱动标志
    uint32_t irq;    //中断
    flags_t flags;
    spinlock_t stats_lock;
    atomic_t irq_sem;
    uint16_t nabmbar;               /*  mastring BAR */
	uint16_t nambar;                /*  mixing BAR */
    uint8_t lvi;                    /* 最后有效位 */
	uint8_t bits;                   /* 支持5还是6位*/
	ac97_bdl_entry_t * bdl;        
	uint16_t * bufs[AC97_BDL_LEN];  /*bdl 物理地址 */
	uint32_t bdl_p;
	uint32_t mask;
    uint16_t *wav_buf;

}ac97_device_t;

int ac97_is_audio(struct snd_ac97 * ac97);

int ac97_is_modem(struct snd_ac97 * ac97);

/**
	*@breaf   ac97访问寄存器写。
	*@param   ac97[in],ac97设备指针。
	*@param   reg[in],寄存器。
	*@param   value[in],访问值。
**/
void snd_ac97_write(struct snd_ac97 *ac97, unsigned short reg, unsigned short value);

/**
	*@breaf   ac97访问寄存器读。
	*@param   ac97[in],设备指针。
	*@param   reg[in],寄存器。
	*@return  读的内容
**/
unsigned short snd_ac97_read(struct snd_ac97 *ac97, unsigned short reg);

/**
	*@breaf   ac97通过缓存方式访问寄存器写。
	*@param   ac97[in],ac97设备指针。
	*@param   reg[in],寄存器。
	*@param   value[in],访问值。
**/
void snd_ac97_write_cache(struct snd_ac97 *ac97, unsigned short reg, unsigned short value);

int snd_ac97_update(struct snd_ac97 *ac97, unsigned short reg, unsigned short value);
#endif