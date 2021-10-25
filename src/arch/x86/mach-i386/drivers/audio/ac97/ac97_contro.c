#include"ac97_contro.h"

#define DRV_NAME "virt-ac97"
#define DRV_VERSION "0.1"

#define DEV_NAME "ac97"


/* snd values */
#define AC97_SND_NAME "Intel AC'97"
#define AC97_PLAYBACK_SPEED 48000


typedef struct {
	uint32_t pointer;  /* Pointer to buffer */
	uint16_t len;       /* Control values and buffer length */
    uint16_t con;
}  ac97_bdl_entry2_t;

struct ac97_buffer {
	uint8_t* buffer;
	uint64_t dma;
	unsigned long time_stamp;
	uint16_t length;
	uint16_t next_to_watch;
};



typedef struct _ac97_extension {
    device_object_t *device_object;   
    pci_device_t* pci_device;
    snd_ac97_t * snd_ac97;
    uint16_t flags;
}ac97_extension_t;


#define TEST 150//声音测试


u32_t get_current_index(ac97_device_t *ac97)
{
	return in8(ac97->nabmbar + AC97_PO_CIV);
}

int  set_last_valid_index(ac97_device_t *ac97, u32_t index)
{
	if(index <0 || index > 31)
		return -1;
	out8(ac97->nabmbar + AC97_PO_LVI, index);
	return 0;
}


//根据civ 设置lvi  加31个
int set_new_index(ac97_device_t *ac97)
{
	int32_t index;
	index = get_current_index(ac97);
	index--;
	index &= 0x1F;
	keprint(" [new index = %d] ",index);
	set_last_valid_index(ac97, index);
	
	return 0;	
}

//如果civ和lvi相等，则更新
int update_lvi(ac97_device_t *ac97)
{
	int16_t index;
	int8_t civ,lvi;
	index =in16(ac97->nabmbar + AC97_PO_CIV);
	civ = index & 0xFF;
	lvi = (index >>8) & 0xFF;
	keprint(" [civ = %d, lvi = %d] ",civ,lvi);
	if(lvi == civ)
		return set_new_index(ac97);	
	return 0;
}


void bdl_bar_show(ac97_device_t *ext)
{
    ac97_bdl_entry_t *p= ext->bdl;
    for(int i=0;i<AC97_BDL_LEN;i++,p++)
    {
        keprint("bdl %d point %0x  %0x\n",i,*(uint32_t *)(p->pointer),p->cl);
    }
}

void swap(uint32_t *add)
{
    uint16_t *p=add;
    uint16_t low=*add&0xffff;
    uint16_t high=*add>>16;
    keprint("%0x %0x\n",low,high);//1000 8000 
    *p=low;
    *(p++)=high;
}


int irq_handl(irqno_t irq, void *data);

/**
 * 1. 申请pci结构(pci_device_t)并初始化厂商号和设备号
 * 2. 启动总线控制
 * 3. 申请设备io空间，在pci_device中登记io空间基地址
 * 4. 申请中断，并在pci_device中登记中断号
**/
static int ac97_control_get_pci_info(ac97_extension_t* ext)
{
    unsigned long mmio_start = 0;
    int mmio_len;
    /* get pci device */
    pci_device_t* pci_device = pci_get_device(AC97_VENDOR_ID, AC97_AUDIO_DEVICE_ID);
    if(pci_device == NULL) {
        keprint(PRINT_DEBUG "AC97_control init failed: ac97_control_get_pci_info.\n");
        return -1;
    }
    else
    {
          ext->pci_device = pci_device;
          ext->snd_ac97=(snd_ac97_t *)mem_alloc(sizeof(snd_ac97_t));
          ext->snd_ac97->private_data=(ac97_device_t *)mem_alloc(sizeof(ac97_device_t));
          snd_ac97_t *snd_ac97= ext->snd_ac97;
          ac97_device_t *ac97_device=(ac97_device_t *)ext->snd_ac97->private_data;
          //待free
          snd_ac97->flags=1;//是pcm out 待写define
          snd_ac97->scaps=1;//是audio
        ac97_device->nambar=ext->pci_device->bar[0].base_addr;
        ac97_device->nabmbar=ext->pci_device->bar[1].base_addr;  
        keprint(" nambar:%0x ,nabmbar:%0x\n",ac97_device->nambar,ac97_device->nabmbar);
        /* enable bus mastering */
        pci_enable_bus_mastering(pci_device);
        /* get mem_addr */
        mmio_start = pci_device_get_mem_addr(pci_device);
        mmio_len = pci_device_get_mem_len(pci_device);
        ac97_device->hw_addr = (uint8_t*)memio_remap(mmio_start, mmio_len);   
        /* get io address */
        ac97_device->io_addr = pci_device_get_io_addr(pci_device);
        // keprint(PRINT_DEBUG "io_base = %d\n", ext->io_base);
        if(ac97_device->io_addr == 0) {
           keprint(PRINT_DEBUG "AC97_control init failed: INVALID pci device io address.\n");
           return -1;
        }
         /* get irq */
         ac97_device->irq = pci_device_get_irq_line(pci_device);
        if(ac97_device->irq == 0xff) {
        keprint(PRINT_DEBUG "AC97_control init failed: INVALID irq.\n");
        return -1;
        }
        iostatus_t err;
        if((0 != irq_register(ac97_device->irq, irq_handl, IRQF_SHARED, "IRQ-ac97", DEV_NAME, (void *) ext))) {
            keprint("irq register error \n");
            return -1;
        }
    }
    return 0;
}


 int irq_handl(irqno_t irq, void *data)
  {
      keprint("rnter irq handl \n");
    ac97_extension_t* ext = (ac97_extension_t*)data;
    ac97_device_t *ac97_device=(ac97_device_t *)ext->snd_ac97->private_data;
    uint16_t sr=snd_ac97_read(ext->snd_ac97,ac97_device->nabmbar+AC97_PO_SR);
	//uint16_t sr = in16(ext->nabmbar + AC97_PO_SR);
	//产生中断先访问master寄存器上的 pcm输出状态寄存器
	if (!sr)
    {
        keprint("no irq\n");
        return IRQ_NEXTONE;//表示没有任何中断事件发生，直接返回
    } 
    
	if (sr & AC97_X_SR_BCIS) {//缓冲区完成中断
        //check_ac97_global_status_info(ext);
        keprint("ac97 irq is buffer complet %d\n",ac97_device->lvi);
       
		//size_t f = (ext->lvi + 2) % AC97_BDL_LEN;
		//for (size_t i = 0; i < AC97_BDL_BUFFER_LEN * sizeof(*ext->bufs[0]); i += 0x1000) {
			//snd_request_buf(&_snd, 0x1000, (uint8_t *)ext->bufs[f] + i);
			//switch_task(1);

            uint16_t current_buffer = in8(ac97_device->nabmbar + AC97_PO_CIV);
            keprint("cur index=%d\n",current_buffer);
		   // ext->lvi = ((current_buffer+2) & (AC97_BDL_LEN-1));

   
		//ext->lvi = (ext->lvi + 1) % (AC97_BDL_LEN);
		//out8(ext->nabmbar + AC97_PO_LVI, ext->lvi);//访问bus master寄存器上的pcm out 最后一个有效寄存器，并写入
	} else if (sr & AC97_X_SR_LVBCI) {//最后一个有效缓冲区处理完成中断
		keprint("ac97 irq is lvbci\n");
	} else if (sr & AC97_X_SR_FIFOE) {//表示发生了fifo欠载或者超载 pci in 寄存器为超载，pciout 为欠载
		keprint( "ac97 irq is fifoe\n");
	} else if(sr & AC97_X_CR_RR){//表示所有的buff记载完成
        keprint(" all buffer is complet %d\n",ac97_device->lvi);
    } else {
        keprint("handle enter other\n");
		return IRQ_NEXTONE;
	}
    snd_ac97_write(ext->snd_ac97,ac97_device->nabmbar+ AC97_PO_SR, sr & 0x1c);
    //out16(ext->nabmbar + AC97_PO_SR, sr & 0x1c);//访问pcm out的状态寄存器 sr&0001 1100
	return IRQ_HANDLED;
}

delay(uint8_t time)
{
    for(int i=0;i<1000;i++)
    {
        for(int t=0;t<20000;t++)
            for(int k=0;k<time;k++);
    }
}

void bus_write( snd_ac97_t *ac97, unsigned short reg, unsigned short val)
{
     out16(reg,val);
}
unsigned short bus_read( snd_ac97_t *ac97, unsigned short reg)
{
     return in16(reg);
}

void bus_reset (snd_ac97_t *ac97)
{
    ac97_device_t *ac97_device=( ac97_device_t *) ac97->private_data;
    if(1)//为正确的硬件
    {
         out32(ac97_device->nabmbar + AC97_GLB_CTRL,in32(ac97_device->nabmbar + AC97_GLB_CTRL) | 0x02);
    }
}

void snd_ac97_bus_init(snd_ac97_t * snd_ac97)
{   
    snd_ac97_bus_t *snd_ac97_bus =(snd_ac97_bus_t *)mem_alloc(sizeof(snd_ac97_bus_t));
    snd_ac97_bus_ops_t * snd_ac97_bus_ops=(snd_ac97_bus_ops_t *)mem_alloc(sizeof(snd_ac97_bus_ops_t));
    //待free
    snd_ac97->bus=snd_ac97_bus;
    snd_ac97->bus->ops=snd_ac97_bus_ops;
    snd_ac97_bus_ops->write=bus_write;
    snd_ac97_bus_ops->read=bus_read;
    snd_ac97_bus_ops->reset=bus_reset;
}
int ac97_device_init(snd_ac97_t *snd_ac97);

void snd_ac97_init(ac97_extension_t * ext)
{

    snd_ac97_t *snd_ac97= ext->snd_ac97;
    snd_ac97_bus_init(snd_ac97);

    ac97_device_init(snd_ac97);
}


    /**
    1.激活io空间和master总线
    2.重置
    3.调整音量
    4.调整采样率
    5。分配bdl空间及缓冲区
    **/
int ac97_device_init(snd_ac97_t *snd_ac97)
{

    //bus master 启动，允许访问io空间
    ac97_device_t *ac97_device=(ac97_device_t *)snd_ac97->private_data;
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_COMMAD,snd_ac97_read(snd_ac97,ac97_device->nambar+AC97_COMMAD)|0x05);
    //out16(ac97_device->nambar+AC97_COMMAD,in16(ac97_device->nambar+AC97_COMMAD) |0x05);
    
    //重置，写入任意值
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_RESET,42);
    //out16 (ext->nambar+AC97_RESET, 42);
    //冷启动重置
    
    snd_ac97->bus->ops->reset(snd_ac97);
    //out32(ext->nabmbar + AC97_GLB_CTRL,in32(ext->nabmbar + AC97_GLB_CTRL) | 0x02); //还是OUT8 ?

    //延迟100
    delay(100);

	//将PCM输出,master设为满音量
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_MASTER_VOLUME,0X0000);
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_PCM_OUT_VOLUME,0X0000);
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_MONO_VOLUME,0X0000);

    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_PC_BEEP_VOLUME,0X0000);

    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_HEADPHONE_VOLUME,0X0000);
    //out16 (ext->nambar + AC97_HEADPHONE_VOLUME, 0x0000);//耳机音量

    delay(100);
    if(!(snd_ac97_read(snd_ac97,ac97_device->nambar+AC97_EXT_AUDIO)&1))
    //if(!(in16(ext->nambar+ AC97_EXT_AUDIO) & 1))
     { 
         /*采样固定为48khz */ 
         keprint("sample rate 48khz\n");
     }
     else
     {
        snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_EXT_AUDIO_STC,snd_ac97_read(snd_ac97,ac97_device->nambar+AC97_EXT_AUDIO_STC)|1);
        //out16(ext->nambar + AC97_EXT_AUDIO_STC, in16(ext->nambar + AC97_EXT_AUDIO_STC) | 1); //启用音频速率变量
        delay(10);
        snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_PCM_FRONT_DAC_RATE,44100);
        //out16(ext->nambar + AC97_PCM_FRONT_DAC_RATE, 44100); //Allg. Samplerate: 44100 Hz
        delay(10);
        snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_PCM_LR_ADC_RATE,44100);
        //out16(ext->nambar + AC97_PCM_LR_ADC_RATE, 44100); //Stereo-Samplerate: 44100 Hz
        keprint("sample rate 44.1khz\n");
     }

	//分配我们的BDL和缓冲区
	//个数*每个的指针大小
    ac97_device->bdl = (ac97_bdl_entry_t *)mem_alloc(AC97_BDL_LEN * sizeof(ac97_bdl_entry_t ));
    if(!ac97_device->bdl)
    {
        keprint("bdl mem_alloc error\n");
        return -1;
    }
    ac97_device->bdl_p=&ac97_device->bdl;
	memset(ac97_device->bdl, 0, AC97_BDL_LEN * sizeof(ac97_bdl_entry_t ));
	for (int i = 0; i < AC97_BDL_LEN; i++) {
        ac97_device->bufs[i] = (uint16_t *)mem_alloc(AC97_BDL_BUFFER_LEN * sizeof(uint16_t));
        if(!ac97_device->bufs[i])
        {
            keprint("bdl buf mem_alloc error\n");
            return -1;
        }
        ac97_device->bdl[i].pointer=ac97_device->bufs[i]; //--//    
        keprint("%d pointer add=%0x  phy add=%0x\n",i,ac97_device->bdl[i].pointer,ac97_device->bufs[i]);                        
		memset(ac97_device->bufs[i], 0, AC97_BDL_BUFFER_LEN * sizeof(uint16_t));
		AC97_CL_SET_LENGTH(ac97_device->bdl[i].cl, AC97_BDL_BUFFER_LEN);
		//ac97_device->bdl[i].len=AC97_BDL_BUFFER_LEN;
        //ac97_device->bdl[i].con=0;

		/* Set all buffers to interrupt */
        //CL的最高位置15为1,开启中断，设置14位为1代表最后一个数据
		ac97_device->bdl[i].cl |= AC97_CL_IOC;
	}       
	/* Tell the ac97 where our BDL is */
    //keprint("bdl_p:%0x\n",ext->bdl_p);
	out32(ac97_device->nabmbar + AC97_PO_BDBAR, ac97_device->bdl_p);//告诉pcm out buff 基地址，bdl的地址
	/* Set the LVI to be the last index */
	ac97_device->lvi = 0;
	out8(ac97_device->nabmbar + AC97_PO_LVI, ac97_device->lvi);//高斯pcm out 最后有效索引寄存器

  

    //全局控制寄存器
   // uint32_t go_c=in32(ext->nabmbar + 0X2C);
   // uint8_t tt=go_c & 3<<22;//pcm mode 16bit or 20bit
   // keprint("pcm mode %d\n",tt);//默认16位
   //  tt=go_c & 3<<20;//pcm pit channel mode
    //keprint("pcm out channel mode %d\n",tt);//默认双通道

	/* detect whether device supports MSB */
    snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_MASTER_VOLUME,0x2020);
    uint16_t t =snd_ac97_read(snd_ac97,ac97_device->nambar+AC97_MASTER_VOLUME)& 0x1f;
	if (t == 0x1f) {
		keprint( "This device only supports 5 bits of audio volume.\n");
		ac97_device->bits = 5;
		ac97_device->mask = 0x1f;
        snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_MASTER_VOLUME,0x0f0f);
	} else {
		ac97_device->bits = 6;
		ac97_device->mask = 0x3f;
        snd_ac97_write(snd_ac97,ac97_device->nambar+AC97_MASTER_VOLUME,0x1f1f);

	}

    /* Enable all matter of interrupts */
	//使能控制器的第4，3位也就是开启完成时中断和fifo中断,和最后一个buff中断（2）
	out8(ac97_device->nabmbar + AC97_PO_CR, AC97_X_CR_FEIE | AC97_X_CR_IOCE|AC97_X_CR_LVBIE);


	//置pcm out 控制寄存器的第一位为1，及运行声卡
	//out8(ext->nabmbar + AC97_PO_CR, in8(ext->nabmbar + AC97_PO_CR) | AC97_X_CR_RPBM);
    //out8(ext->nabmbar + AC97_PO_CR, in8(ext->nabmbar + 0x2B) | AC97_X_CR_RPBM); //test

	keprint("AC97 initialized successfully\n");

    return 0;
}

void check_ac97_global_status_info(ac97_device_t *ext)
{
        uint32_t glo_info=in32(ext->nabmbar+AC97_GLO_STAT);
        keprint("global status :%0x\n",glo_info);

}


 int ac97_mixer_read(ac97_device_t *ext, uint32_t *val) {
	uint16_t tmp;
			tmp = in16(ext->nambar + AC97_PCM_OUT_VOLUME);//访问混音寄存器的pcm out
			if (tmp == 0x8000) {
				*val = 0;
			} else {
				/* 5 bit value */
				*val = (tmp & 0x1f) << (sizeof(*val) * 8 - 5);
				*val = ~*val;
				*val &= 0x1f << (sizeof(*val) * 8 - 5);
			}
	return 0;
}

 int ac97_mixer_write(ac97_device_t *ext, uint32_t val) {
	
			uint16_t encoded;
			if (val == 0x0) {
				encoded = 0x8000;
			} else {
				/* 0 is the highest volume */
				val = ~val;
				/* 5 bit value */
				val >>= (sizeof(val) * 8 - 5);
				encoded = (val & 0xFF) | (val << 8);
			}
			out16(ext->nambar + AC97_PCM_OUT_VOLUME, encoded);//访问混音寄存器的pcm out
	return 0;
}



iostatus_t ac97_control_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    snd_device_t * snd_dev=(snd_device_t *)device->device_extension;
    ac97_extension_t * ext=(ac97_extension_t *)snd_dev->device;
    int wavlen = ioreq->parame.write.length;
    snd_ac97_t *snd_ac97=ext->snd_ac97;
    ac97_device_t * ac97_device=(ac97_device_t *)snd_ac97->private_data;
    //keprint("wavlen==%d\n",wavlen);
    
    ac97_device->wav_buf=ioreq->user_buffer;
    if (wavlen >MAX_AC97_WAV_BUF_SIZE)//一次读若干
    {
        wavlen=MAX_AC97_WAV_BUF_SIZE;
    }
    int len=wavlen;
    uint8_t num=0;
    uint8_t remainder=0;
	for (int i = 0; i < AC97_BDL_LEN; i++)
     {
        num=wavlen/AC97_BDL_BUFFER_LEN;
        remainder=wavlen%AC97_BDL_BUFFER_LEN;
        if(num>0)
        {
            memcpy(ac97_device->bufs[i],ac97_device->wav_buf,AC97_BDL_BUFFER_LEN*2 );//2字节 
            AC97_CL_SET_LENGTH(ac97_device->bdl[i].cl, AC97_BDL_BUFFER_LEN);
            //CL的最高位置15为1,开启中断，设置14位为1代表最后一个数据
            ac97_device->bdl[i].cl |= AC97_CL_IOC;
            //ac97_device->bdl[i].len=AC97_BDL_BUFFER_LEN;		
		    /* Set all buffers to interrupt */

            //ac97_device->bdl[i].con |=1<<15;
            // swap(&ext->bdl[i].cl);
            wavlen-=AC97_BDL_BUFFER_LEN;
            ac97_device->wav_buf+=AC97_BDL_BUFFER_LEN;

        }
        else
        {
            memcpy(ac97_device->bufs[i],ac97_device->wav_buf,remainder);
            AC97_CL_SET_LENGTH(ac97_device->bdl[i].cl, remainder);
            ac97_device->bdl[i].cl |= 1<<31;
            //ac97_device->bdl[i].len=remainder;
            ac97_device->lvi = i;
            //swap(&ext->bdl[i].cl);
	        out8(ac97_device->nabmbar + AC97_PO_LVI, ac97_device->lvi);
            keprint("lvi==%d\n",ac97_device->lvi);
            uint16_t current_buffer = in8(ac97_device->nabmbar + AC97_PO_CIV);
            keprint("cur index=%d\n",current_buffer);

            bdl_bar_show(ac97_device);
           
            //out8(ext->nabmbar + 0x2B, in8(ext->nabmbar + 0x2B) | AC97_X_CR_RPBM); //test
            break;
        }
        
    } 
    //运行
    out8(ac97_device->nabmbar + AC97_PO_CR, in8(ac97_device->nabmbar + AC97_PO_CR) | AC97_X_CR_RPBM);  
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ac97_control_read(device_object_t *device, io_request_t *ioreq)
{

    iostatus_t status = IO_SUCCESS;
    ioreq->io_status.infomation = ioreq->parame.read.length;    /* 读取永远是0 */
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}



static iostatus_t ac97_control_enter(driver_object_t *driver)
{
    iostatus_t status;
    ac97_extension_t* devext;
    device_object_t *devobj;
    snd_device_t * snd_device;
    snd_device=(snd_device_t *)mem_alloc(sizeof(snd_device_t));
    /* 初始化一些其它内容 */
    status = io_create_device(driver, 0, DEV_NAME, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);
    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "ac97_control_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    devobj->device_extension=snd_device ;
    snd_device->device=devext;
   
    devext->device_object = devobj;
    devext->flags = 0;
    /*初始化接收队列，用内核队列结构保存，等待被读取*/
    //io_device_queue_init(&devext->rx_queue);
    /* 申请并初始化pci_device_t，io_addr、中断号*/
    if(ac97_control_get_pci_info(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        keprint(PRINT_DEBUG "AC97 get pci info err\n");
        return status;
    }
    keprint("\nxxxxxxxx\n");
    snd_ac97_init(devext);
    keprint("\nxxxxxxxx\n");
    return status;
}


int ac97_control_reset(driver_object_t* ext)
{
    
}

static iostatus_t ac97_control_exit(driver_object_t *driver)
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

//open 时候将ac97注册到声卡
static iostatus_t ac97_control_open(driver_object_t *driver)
{
    snd_device_t * snd_device = (snd_device_t * )driver->drver_extension;
    snd_register(snd_device);
    return IO_SUCCESS;
}

//关闭时候将ac97从声卡中注销
static iostatus_t ac97_control_close(driver_object_t *driver)
{
    snd_device_t * snd_device = (snd_device_t * )driver->drver_extension;
    snd_unregister(snd_device);
    return IO_SUCCESS;
}
iostatus_t ac97_control_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = ac97_control_enter;
    driver->driver_exit = ac97_control_exit;
    //driver->dispatch_function[IOREQ_OPEN] = ac97_control_open;
    //driver->dispatch_function[IOREQ_CLOSE] = ac97_control_close;
    driver->dispatch_function[IOREQ_WRITE] = ac97_control_write;
    driver->dispatch_function[IOREQ_READ] = ac97_control_read;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "ac97_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void ac97_control_driver_entry(void)
{
    if (driver_object_create(ac97_control_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

//driver_initcall(ac97_control_driver_entry);

