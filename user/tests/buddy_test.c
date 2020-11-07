#include"test.h"

#ifndef _LIST_H
#define _LIST_H
 
//定义核心链表结构
struct list_head
{
    struct list_head *next, *prev;
};
 
//链表初始化
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}
 
//插入结点
static inline void __list_add(struct list_head *new_list,
                            struct list_head *prev, struct list_head *next)
{
    next->prev = new_list;
    new_list->next = next;
    new_list->prev = prev;
    prev->next = new_list;
}
 
//在链表头部插入
static inline void list_add(struct list_head *new_list, struct list_head *head)
{
    __list_add(new_list, head, head->next);
}
 
//删除任意结点
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}
 
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}
 
//后序（指针向后走）遍历链表
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)
 
//前序（指针向前走）遍历链表
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)
 
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
 
//这个宏中某些可能只有GNU支持,我实验的环境是windows下qt5.2,很幸运,也支持
#define list_entry(ptr, type, member) ({			\
    const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
    (type *)( (char *)__mptr - offsetof(type,member) );})
 
#endif

#ifndef MEM_MANAGE_H
#define MEM_MANAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define MEM_NUM 11  //不同种类内存块总数目
#define BLOCK_BASE_SIZE 4096  //以4KB为单位分配
 
#define TRUE  1
#define FALSE 0
 
#define random(x) (rand()%x + 1)  //随机数范围（1到x），用于属性随机数产生
#define cloth2cover(num) ((num & (num-1)) == 0)? TRUE:FALSE  //判断是否是2^n
 
typedef unsigned int u32;
 
//内存链表数据结构
struct free_area_head
{
    u32 size;  //链表内存大小
    u32 num;  //相应内存块数目
    struct list_head list;
};
 
//内存块数据结构
struct free_chunk
{
    u32 size;   //单位是KB
    u32 addr;  //内存地址
    int dir;    //标识这块内存块是否被占用，1被占用，0空闲
    struct list_head list;
};
 
//函数声明
int mem_init();
int mem_alloc(int size);
inline int mem_avail(int size);
int mem_free(int size);
 
#endif // MEM_MANAGE_H


 
struct free_area_head mem_arr[MEM_NUM];  //指向不同种类内存块的指针数组
char *pmem = NULL;
 
/**
 * @brief 内存初始化函数，包括申请内存，分割内存，挂入管理链表等操作
 * @return 成功返回0，失败返回-1
 */
int mem_init(void)
{
    int i, j, total_size = 0;
    u32 trunk_size;
    struct free_chunk *tmp_chunk = NULL;
 
    for(i=0; i<MEM_NUM; i++)
    {
        memset(&mem_arr[i], 0, sizeof(struct free_area_head));
        //第0个链表代表4KB的内存块,第1个代表8KB,第2个代表16KB...以此类推
        mem_arr[i].size = (1<<i) * BLOCK_BASE_SIZE;  //单位是字节
        INIT_LIST_HEAD(&mem_arr[i].list);  //初始化每个链表头
    }
 
    //分配4KB的内存块11个，8KB的10个，16KB的9个....4MB的1个
    for(i=0; i<MEM_NUM; i++)
    {
        total_size += (i+1)*BLOCK_BASE_SIZE * (1<<(10-i));
    }
 
    //把这块内存分配出来
    pmem = (char *)malloc(total_size);
    if(pmem == NULL)
    {
        printf("err malloc mem\n");
        return -1;
    }
    else
    {
        printf("malloc mem success\n");
        printf("alloc mem init addr is %u\n\n", pmem);
    }
 
 
    /*
        这段代码可能不太好理解,需要把具体数带进去验算一下
        目的:把上面分配的一大段连续内存依次拆分为:
        4KB/8KB/16KB...4096KB          第1个序列
        4KB/8KB/16KB...2048KB          第2个序列
        4KB/8KB/16KB...1024KB          第3个序列
        4KB/8KB/16KB...512KB           第4个序列
        4KB/8KB/16KB...256KB           第5个序列
        4KB/8KB/16KB...128KB           第6个序列
        4KB/8KB/16KB...64KB            第7个序列
        4KB/8KB/16KB/32KB              第8个序列
        4KB/8KB/16KB                   第9个序列
        4KB/8KB                        第10个序列
        4KB                            第11个序列
        上面的内存段都是连续的，这样做的目的是让所有相同大小的内存块都不相邻
        两个嵌套之后,上面内存卡就依次联入相应的链表中了
    */
 
    for(i=MEM_NUM; i>0; i--)
    {
        //第一个连续内存  4KB/8KB/16KB...2048KB/4096KB
        for(j=0; j<i; j++)
        {
            //先把4KB挂在4KB的链表上
            trunk_size = BLOCK_BASE_SIZE*(1<<j);
            tmp_chunk = (struct free_chunk *)malloc(sizeof(struct free_chunk));
            tmp_chunk->size = trunk_size / 1024;  //以KB为单位
            tmp_chunk->addr = pmem;  //记录下这个地址
            tmp_chunk->dir = 0;  //初始化内存卡未被占用
            list_add(&tmp_chunk->list, &mem_arr[j].list); //插入链表
            mem_arr[j].num++;  //相应链表内存块数目加1
            pmem += trunk_size; //指针相应往后移动4KB/8KB/16KB...2048KB/4096KB
        }
    }
 
    //我们把各个链表相关内容打印出来看一下
    struct list_head *pos;
    struct free_chunk *tmp;
 
    //首先是每个链表的内存块大小和内存块数目
    for(i=0; i<MEM_NUM; i++)
    {
        printf("the %d list mem num is %d:", i, mem_arr[i].num);
        list_for_each(pos, &mem_arr[i].list)
        {
            tmp = list_entry(pos, struct free_chunk, list);
            printf("%d ", tmp->size);
        }
        printf("\n");
    }
 
    //怎么验证内存地址的正确性呢？
    printf("\nthe 4KB list chunk addr is:\n");
    //由于list_add是头部插入,所以这里按照从尾部到头部的顺序打印
    list_for_each_prev(pos, &mem_arr[0].list)
    {
        tmp = list_entry(pos, struct free_chunk, list);
        printf("%u ", tmp->addr);
    }
    printf("\n\n");
 
    /*
        malloc mem success
        alloc mem init addr is 19136544
        the 0 list mem num is 11:4 4 4 4 4 4 4 4 4 4 4
        the 1 list mem num is 10:8 8 8 8 8 8 8 8 8 8
        the 2 list mem num is 9:16 16 16 16 16 16 16 16 16
        the 3 list mem num is 8:32 32 32 32 32 32 32 32
        the 4 list mem num is 7:64 64 64 64 64 64 64
        the 5 list mem num is 6:128 128 128 128 128 128
        the 6 list mem num is 5:256 256 256 256 256
        the 7 list mem num is 4:512 512 512 512
        the 8 list mem num is 3:1024 1024 1024
        the 9 list mem num is 2:2048 2048
        the 10 list mem num is 1:4096
        the 4KB list chunk addr is:
        19136544 27521056 31711264 33804320 34848800 35368992
        35627040 35754016 35815456 35844128 35856416
        这些地址每次运行程序都不一样,根据实际情况来看
        得到上面打印结果,首先我们看到11个链表的数目和每个内存卡的大小是正确的
        然后如何验证地址呢？理解这个需要自己画下图,以第一个链表的前两个4KB块的
        首地址为例,这两个首地址应该隔着这么大一块地址空间：
        4+8+16+32+64+128+256+512+1024+2048+4096 = ?KB
        那么 27521056 - 19136544 = 8384512(byte) = 8188KB
        你可以验算一下，这两个值是相等的,同理，你可以验证第2个和第3个的差值
        看是否和理论上的值一样
    */
    return 0;
}
 
 
/**
 * @brief 得到内存数组索引，实际上就是求size是2的几次幂
 */
static int get_index(int size)
{
    int i, tmp = 0;
    size /= 4;
 
    for(i=0; tmp < size; i++)
    {
        tmp = 1<<i;
        if(tmp == size)
        {
            return i;
        }
    }
 
    return -1;  //实际上经过前面判断不可能执行到这
}
 
 
/**
 * @brief 比较核心的函数，完成功能如下
 * 1.判断需要拆分的内存块是目标内存块的多少倍，从而知道应该拆分为几块
 * 2.把被拆分的大内存块从相应链表执行上断链操作
 * 3.把拆分的新内存块链入目标内存链表中并置dir位为0（未占用）
 * 4.选取一块返回，申请成功
 *
 * @param dst_index 目的链表索引值，即本想要申请的内存块所在链表索引值
 * @param src_index 源链表索引值，即需要拆分的内存块所在的链表索引值
 * @param block 需要拆分内存块
 */
static void separate_block(int dst_index, int src_index, struct free_chunk *block)
{
    int i;
    char *pmem;
    u32 block_num, dst_size;
    block_num = (1<<(src_index - dst_index));  //2^差值 倍
 
    list_del(&block->list);  //把被拆分的大内存块从相应链表执行上断链
    mem_arr[src_index].num--;  //内存块数目-1
    printf("%d list separate 1 block\n", src_index);
 
    //拆分为block_num块
    pmem = block->addr;  //记录首地址
    dst_size = mem_arr[dst_index].size;  //目的内存块大小
 
    //拆分并入链
    struct free_chunk *tmp_chunk = NULL;
 
    printf("%d list increase %d block\n", dst_index, block_num);
    for(i=0; i<block_num; i++)
    {
        tmp_chunk = (struct free_chunk *)malloc(sizeof(struct free_chunk));
        tmp_chunk->size = dst_size / 1024;  //以KB为单位
        tmp_chunk->addr = pmem;  //记录下这个地址
        tmp_chunk->dir = 0;  //初始化内存卡未被占用
        list_add(&tmp_chunk->list, &mem_arr[dst_index].list); //插入链表
        mem_arr[dst_index].num++;  //相应链表内存块数目加1
        pmem += dst_size; //指针相应往后移动dst_size字节
    }
 
    //经过上面的循环，拆分入链操作就完成了，下面只需要把拆分的内存块选一块返回即可
    struct list_head *pos;
    struct free_chunk *tmp;
 
    //肯定能找到的
    list_for_each(pos, &mem_arr[dst_index].list)
    {
        tmp = list_entry(pos, struct free_chunk, list);
        if(tmp->dir == 0)
        {
            printf("malloc success,addr = %u\n", tmp->addr);
            tmp->dir = 1; //标记内存块为占用
            return;
        }
    }
}
 
/**
 * @brief 内联函数，判断输入的size是否合法
 */
inline int mem_avail(int size)
{
    if(size<4 || size>(4<<10))  //最小4KB，最大4096KB
    {
        printf("size must > 4 and <= 4096\n");
        return FALSE;
    }
    else if(!cloth2cover(size))  //必须是2的幂
    {
        printf("size must be 2^n\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
 
/**
* @brief 模拟内核申请内存过程
* @param size 申请内存大小，单位为KB
* @return 成功返回0，失败返回-1
*/
int mem_alloc(int size)
{
    int index, i;
 
    if(!mem_avail(size))
    {
        return -1;
    }
 
    /*
      下面是伙伴算法内存申请的过程，思想如下：
      1.首先根据size大小去相应的连表上去查找有没有空闲的内存块
        如果有，那么直接返回地址，并把相应内存块的的dir标志置位
      2.如果没有，那么去它上一级链表中找空闲块，比如4KB没有，那就去8KB找
        如果8KB也没有，就去16KB找...
      3.如果上一级链表中找到了空闲块，那么把这个空闲块从上一级链表中分类
        拆分为相应大小的内存卡后链入查找的链表中
      4.如果直到4MB的内存卡都没有空闲块，那么返回错误，提示内存不足
      这段代码的难点在穷尽的查找比比size大的链表操作
    */
    index = get_index(size);
 
    printf("first find %d list\n", index);
 
    struct list_head *pos;
    struct free_chunk *tmp;
 
    list_for_each(pos, &mem_arr[index].list)
    {
        tmp = list_entry(pos, struct free_chunk, list);
        if(tmp->dir == 0)  //找到了一块
        {
            printf("malloc success,addr = %u\n", tmp->addr);
            tmp->dir = 1; //标记内存块为占用
            return 0;
        }
    }
 
    //如果执行到这里，那么说明对应的内存块链表没有找到空闲内存块
    printf("the %d list has no suitable mem block\n", index);
 
    //我们从比它大的内存块中再去查找,最大就是4MB的链表了
    for(i=index+1; i<MEM_NUM; i++)
    {
        printf("we will find %d list\n", i);
 
        list_for_each(pos, &mem_arr[i].list)
        {
            tmp = list_entry(pos, struct free_chunk, list);
            if(tmp->dir == 0)  //找到了一块
            {
                printf("find a free block from %d list,addr = %u\n", i, tmp->addr);
                //把这块大的内存块拆分为小的，并进行分配处理
                separate_block(index, i, tmp);
                return 0;
            }
        }
    }
 
    //如果执行到这里，那么说明相应大小的内存块无法分配成功
    printf("can't malloc mem\n");
    return -1;
}
 
/**
 * @brief 判断两个地址是否相邻
 * @param compare_addr 比较的地址
 * @param target_addr  目标地址
 * @param size  链表上内存块的大小
 * @return 相邻：TRUE 不相邻：FALSE
 */
static int inline is_neighbor(u32 compare_addr, u32 target_addr, u32 size)
{
    //这里是无符号数，不能用绝对值
    if(compare_addr > target_addr)
    {
        if(compare_addr - target_addr == size)
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        if(target_addr - compare_addr == size)
            return TRUE;
        else
            return FALSE;
    }
}
 
/**
 * @brief 从索引值为index的链表上查找block的伙伴内存块并返回
 * @param block
 * @param index
 * @return
 */
struct free_chunk *find_buddy(struct free_chunk *block, int index)
{
    //伙伴内存块:大小相同，地址相邻，并且也没有被占用
    struct list_head *pos;
    struct free_chunk *tmp;
 
    list_for_each(pos, &mem_arr[index].list)
    {
        tmp = list_entry(pos, struct free_chunk, list);
        if(tmp->dir == 0)  //没有被占用才有比较的资格
        {
            if(is_neighbor(tmp->addr, block->addr, block->size*1024))
            {
                return tmp;
            }
        }
    }
 
    //到这里就是没找到
    return NULL;
}
 
enum BUDDY_TYPE
{
    NO_BUDDY = 0,  //没有伙伴
    LAST_LIST,      //到了最后的链表了
};
 
/**
 * @brief 递归的查找伙伴并释放内存
 * @param block 需要释放的内存块
 * @param index 内存块所在的链表索引
 */
int recursive_free(struct free_chunk *block, int index)
{
    struct free_chunk *buddy;
 
    if(index > MEM_NUM)  //递归到了4MB的链表上
    {
        printf("max index list\n");
        return LAST_LIST;
    }
 
    buddy = find_buddy(block, index);  //在本链表上为它找一个“伙伴内存块”
    if(buddy == NULL)  //这个内存块没有”伙伴“
    {
        printf("this block has no buddy\n");
        block->dir = 0;  //释放它既可
        return NO_BUDDY;
    }
    else
    {
        printf("this block find a buddy\n");
        //两个内存块从原来链表断链
        list_del(&block->list);
        list_del(&buddy->list);
        mem_arr[index].num -= 2;  //少了两块
        printf("%d list decrease 2 block\n", index);
 
        //合并为新的内存块并入链下一级链表
        int new_addr = (block->addr < buddy->addr) ? block->addr:buddy->addr;
 
        struct free_chunk *tmp_chunk = NULL;
        tmp_chunk = (struct free_chunk *)malloc(sizeof(struct free_chunk));
        tmp_chunk->size = block->size * 2;  //是原来内存块的两倍大
        tmp_chunk->addr = new_addr;  //记录下这个地址
        tmp_chunk->dir = 0;  //初始化内存块未被占用
        index++;
        list_add(&tmp_chunk->list, &mem_arr[index].list); //插入链表
        mem_arr[index].num++;  //相应链表内存块数目加1
        printf("%d list increase 1 block\n", index);
 
        //循环这个过程，在上一级链表中查找伙伴，直到找不到伙伴或者到了4MB链表
        recursive_free(tmp_chunk, index);
    }
    return 0;
}
 
/**
* @brief 模拟内核释放内存过程
* @param size 释放内存大小，单位为KB
* @return 成功返回0，失败返回-1
*/
int mem_free(int size)
{
    int index;
 
    if(!mem_avail(size))
    {
        return -1;
    }
 
    /*
      下面是伙伴算法释放内存的过程，思想如下：
      1.首先根据size大小去相应的连表上去查找第一个被占用的内存块
      2.判断这个内存块是否有伙伴（大小相同，地址相邻）
      3.如果没有，直接把dir位清0即可
      4.如果有，那么把这两个内存块分别从所在的链表上断链，然后入链到上一级链表中
      5.到上一级链表中继续 2 3 4 操作，直到某一级链表没有伙伴为止
    */
    index = get_index(size);
 
    printf("first find %d list\n", index);
 
    struct list_head *pos;
    struct free_chunk *tmp;
 
    list_for_each(pos, &mem_arr[index].list)
    {
        tmp = list_entry(pos, struct free_chunk, list);
        if(tmp->dir == 1)  //找到了第一块被占用的内存
        {
            printf("find an occupy block,addr = %u\n", tmp->addr);
            recursive_free(tmp, index);
            return 0;
        }
    }
 
    //如果执行到这里，那么说明对应的内存块链表没有占用内存块
    printf("the %d list has no occupy mem block\n", index);
    return -1;
}

int buddy_test(int argc,char *argv[])
{
    mem_init();  //初始化内存
 
    /*
        到这了为止我们的内存分配及链表的链接工作就已经做完了,下面就是在应用层模拟内核的伙伴算法
        我们怎么模拟呢,我们以4MB和2MB为实验材料,因为这两个消耗的快
        可以想象这么一种情况：
        1.第一次分配一个2MB,ok,没有问题，分配给你，然后又分配一个2MB，还是没有问题
        2.第三次分配2MB就出问题了，2MB的链表上已经没有2MB了，怎么办
        3.拆4MB的，把4MB的内存卡拆为两个2MB的，这两个地址是连续的，并把这个内存块从
          4MB的链表删除，链入2MB的链表中，我们记这两个内存块为a和b
        4.这时候如果再分配4MB的，就会提示没有可分配的内存，而2MB的又有了
        5.当我们释放a时候，没有什么发生，但是a释放后如果再释放了b，就会把a和b组成新的4MB
          内存块重新链入4MB的内存链表
    */
    char c;
    int size;

    mem_alloc(4);
    mem_free(4);
    mem_alloc(12);
    mem_free(12);
    mem_alloc(32);
    mem_free(32);
    
    
    return 0;

    while(1)
    {
        printf("\nmalloc type m, free type f:");
        scanf("%c",&c);
 
        if(c == 'm')
        {
            printf("\ninput alloc mem size,unit is KB :");
            scanf("%d",&size);
            if(!mem_avail(size))
            {
                continue;
            }
 
            mem_alloc(size);
        }
        else if(c == 'f')
        {
            printf("\nfree mem size,unit is KB :");
            scanf("%d",&size);
            if(!mem_avail(size))
            {
                continue;
            }
 
            mem_free(size);
        }
        else
        {
//            printf("\nerr input, again\n");
            continue;
        }
    }
	return 0;
}
