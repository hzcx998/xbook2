#include <xbook/string.h>
#include <xbook/bitmap.h>
#include <xbook/memops.h>

/*
 * bitmap_init - 初始化位图
 * btmp: 要初始化的位图结构的地址
 * 
 * 初始化一个位图结构，其实就是把位图数据指针指向的地址清0
 */
void bitmap_init(bitmap_t *btmp) 
{
   memset(btmp->bits, 0, btmp->byte_length);   
}

/*
 * bitmap_scan_test - 检测位图某位是否为1
 * @btmp: 要检测的位图
 * @idx: 要检测哪一位
 * 
 * 可以通过检测寻找哪些是已经使用的，哪些还没有使用
 */
bool bitmap_scan_test(bitmap_t *btmp, unsigned long  idx) 
{
   unsigned long byte_idx = idx / 8;   
   unsigned long bit_odd  = idx % 8;  
   return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}

/*
 * bitmap_scan - 扫描n个位
 * @btmp: 要检测的位图
 * @cnt: 要扫描多少位
 * 
 * 可以通过检测寻找哪些是已经使用的，哪些还没有使用
 */
long bitmap_scan(bitmap_t *btmp, unsigned long cnt) {
   unsigned long idx_byte = 0;

   while (( 0xff == btmp->bits[idx_byte]) && (idx_byte < btmp->byte_length)) {
      idx_byte++;
   }

   
   if (idx_byte == btmp->byte_length) {  
      return -1;
   }

   long idx_bit = 0;

   while ((unsigned char)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) { 
	 idx_bit++;
   }
	 
   long idx_start = idx_byte * 8 + idx_bit;  
   if (cnt == 1) {
      return idx_start;
   }

   unsigned long bit_left = (btmp->byte_length * 8 - idx_start);
   unsigned long next_bit = idx_start + 1;
   unsigned long count = 1;

   idx_start = -1;
   while (bit_left-- > 0) {
      if (!(bitmap_scan_test(btmp, next_bit))) {
	 count++;
      } else {
	 count = 0;
      }
      if (count == cnt) {
	 idx_start = next_bit - cnt + 1;
	 break;
      }
      next_bit++;          
   }
   return idx_start;
}

/*
 * bitmap_set - 把某一个位设置成value值
 * @btmp: 要检测的位图
 * @idx: 哪个地址
 * @value: 要设置的值（0或1）
 * 
 * 可以通过设置位图某位的值来表达某一个事物是否使用
 */
void bitmap_set(bitmap_t *btmp, unsigned long idx, char value)
{
   unsigned long byte_idx = idx / 8;
   unsigned long bit_odd  = idx % 8;

   if (value) {
      btmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);
   } else {
      btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
   }
}

/*
 * bitmap_change - 把一位取反
 * @btmp: 要检测的位图
 * @idx: 哪个地址
 */
long bitmap_change(bitmap_t *btmp, unsigned long idx)
{
   unsigned long byte_idx = idx / 8;
   unsigned long bit_odd  = idx % 8;
   //进行异或
   btmp->bits[byte_idx] ^= (BITMAP_MASK << bit_odd);
   //返回该位
   return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}

/*
 * bitmap_test_and_change - 测试并改变该位
 * @btmp: 要检测的位图
 * @idx: 哪个地址
 * 
 * 返回之前的状态
 */
long bitmap_test_and_change(bitmap_t *btmp, unsigned long idx)
{
   unsigned long byte_idx = idx / 8;
   unsigned long bit_odd  = idx % 8;

   //获取该位
   long ret = btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd);

   //进行异或取反
   btmp->bits[byte_idx] ^= (BITMAP_MASK << bit_odd);
   //返回之前的状态
   return ret;
}
