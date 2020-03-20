#ifndef _XBOOK_BITOPS_H
#define _XBOOK_BITOPS_H

#include <arch/interrupt.h>

/* 求位数值 */
#define low8(a) (unsigned char)((a) & 0xff) 
#define high8(a) (unsigned char)(((a) >> 8) & 0xff) 

#define low16(a) (unsigned short)((a) & 0xffff) 
#define high16(a) (unsigned short)(((a) >> 16) & 0xffff) 

#define low32(a) (unsigned int)((a) & 0xffffffff) 
#define high32(a) (unsigned int)(((a) >> 32) & 0xffffffff) 

/* 合并操作 */
#define merge64(a, b) (unsigned long)((((a) & 0xffffffff) << 32) | ((b) & 0xffffffff)) 
#define merge32(a, b) (unsigned int)((((a) & 0xffff) << 16) | ((b) & 0xffff)) 
#define merge16(a, b) (unsigned short)((((a) & 0xff) << 8) | ((b) & 0xff)) 
#define merge8(a, b) (unsigned char)((((a) & 0xf) << 4) | ((b) & 0xf)) 

/**
 * set_bit - 设置位为1
 * @nr: 要设置的位置
 * @addr: 要设置位的地址
 * 将addr的第nr(nr为0-31)位置值置为1， 
 * nr大于31时，把高27的值做为当前地址的偏移，低5位的值为要置为1的位数 
*/
static inline unsigned long set_bit(int nr, unsigned long *addr)  
{  
   unsigned long mask, retval;  

   addr += nr >> 5;                 //nr大于31时，把高27的值做为当前地址的偏移，  
   mask = 1 << (nr & 0x1f);         //获取31范围内的值，并把1向左偏移该位数  
   disable_intr();              //关所有中断  
   retval = (mask & *addr) != 0;    //位置置1  
   *addr |= mask;  
   enable_intr();               //开所有中断  
   
   return retval;                   //返回置数值  
} 

/**
 * clear_bit - 把位置0
 * @nr: 要设置的位置
 * @addr: 要设置位的地址
 * 
 * 将addr的第nr(nr为0-31)位置值置为0;  
 * nr大于31时，把高27的值做为当前地址的偏移，低5位的值为要置为0的位数
 */
static inline unsigned long clear_bit(int nr, unsigned long *addr)  
{  
   unsigned long mask, retval;  

   addr += nr >> 5;  
   mask = 1 << (nr & 0x1f);  
   disable_intr();  
   retval = (mask & *addr) != 0;  
   *addr &= ~mask;
   enable_intr();  
   return retval;  
}  
  
/**
 * test_bit - 测试位的值
 * @nr: 要测试的位置
 * @addr: 要设置位的地址
 * 
 * 判断addr的第nr(nr为0-31)位置的值是否为1;  
 * nr大于31时，把高27的值做为当前地址的偏移，低5位的值为要判断的位数；  
 */
static inline unsigned long test_bit(int nr, unsigned long *addr)  
{  
   unsigned long mask;  

   addr += nr >> 5;
   mask = 1 << (nr & 0x1f);  
   return ((mask & *addr) != 0);  
}

/**
 * test_and_set_bit - 测试并置1
 * @nr: 要测试的位置
 * @addr: 要设置位的地址
 * 
 * 先测试，获取原来的值，再设置为1
 */
static inline unsigned long test_and_set_bit(int nr, unsigned long *addr)  
{
   unsigned long old;  
   /* 先测试获得之前的值 */
   old = test_bit(nr, addr);
   /* 再值1 */
   set_bit(nr, addr);
   /* 返回之前的值 */
   return old;  
}

#endif   /* _XBOOK_BITOPS_H */
