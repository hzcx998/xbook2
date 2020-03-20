
#include <xbook/debug.h>


#define ARDS_ADDR 0x80001000 //ARDS结构从哪儿开始储存

#define MAX_ARDS_NR 12 //最大有12个ards结构

/*
ards结构体
*/
struct ards_struct {
	unsigned int baseLow;  //基址低32位
	unsigned int base_high;
	unsigned int lengthLow;  //长度低32位
	unsigned int length_high;			
	unsigned int type;  //该结构的类型(1可以被系统使用)
};

unsigned int get_memory_size_from_hardware()
{
	unsigned int totalSize = 0;

	unsigned int ardsNum =  *((unsigned int *)ARDS_ADDR);	//ards 结构数
	
	if (ardsNum > MAX_ARDS_NR) {
		ardsNum = MAX_ARDS_NR;
	}
	struct ards_struct *ards = (struct ards_struct *) (ARDS_ADDR+4);	//ards 地址
	printk("\n");
	int i;
	for(i = 0; i < ardsNum; i++){
		//寻找可用最大内存
		if(ards->type == 1){
			//冒泡排序获得最大内存
			if(ards->baseLow+ards->lengthLow > totalSize){
				totalSize = ards->baseLow+ards->lengthLow;
			}
		}
		printk("base %8x length %8x type:%d\n",ards->baseLow, ards->lengthLow, ards->type);
		ards++;
	}
    printk("memory total:%x byte %d MB\n", totalSize, totalSize / (1024*1024));
	return totalSize;
}