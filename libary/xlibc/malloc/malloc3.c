#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

/**原理: 
        1.通过sbrk(int size) 使break指针前移来增加内存,brk()设置break指针来释放内存
        2.把分配的内存抽象为一个block
        3.block之间通过链表来管理
        4.分配时,sbrk(0) 获取block起始地址
*/
 
/** 
    需求
    1.内存以8字节对齐
    2.以经分配的内存块用链表来管理
    4.相邻的空闲块进行合并
 
 
    1.malloc 过程 
        1.对齐size
        2.从空闲的链表中找到一个符合块(firt-fit算法)
        3.如果没有找到则使用sbrk重新分配一块内存
     
*/
 
#define  BLOCK_HEAD_SIZE   24 //结构体除data外所占用的内存大小
/*把block结构体定义为*/
struct s_block{
    int size;               //block 大小
    struct s_block *prev;   //前驱
    struct s_block *next;   //后继
    int free;               //是否空闲
    void *ptr;              //data喻指针,用来判断地址是否合法的
    int   padding;          //填充直接,保证结构体是以8字节对齐的
    char  data[1];          //数据区域
};
typedef struct s_block  *t_block;
 
 
/*所有的block以链表的方式组织*/
static t_block first_block=NULL;
 
 
/*检测内存地址是否有效*/

t_block get_block(void *p){  
    char *tmp = p;
    return (t_block)(tmp-BLOCK_HEAD_SIZE);   //指向block块头部
}
bool valid_addr(void *p){
    //地址大于第一个block的首地址,小于break的地址 break = sbrk(0)
    if((t_block)p <= first_block || p >= sbrk(0)){
        return false;
    }
    t_block tmp = get_block(p);
    if(tmp->ptr == p)
        return true;
    else
        return false;
}
 
/*遍历查找一个合适的block*/
t_block find_block(t_block *last,int size){
    t_block b = first_block;
    while(b && !(b->free && b->size >=size)){
        *last = b;
        b = b->next;
    }
    return b;
}
 
/*开辟的内存以8字节对齐(即结构体开始地址是8的整数倍)*/
int align(int size){
    if(size % 8 ==0) return size;
    return ((size >> 3) + 1) <<3;
}

unsigned int calc_align(unsigned int n,unsigned align)  
{      
    return ((n + align - 1) & (~(align - 1)));  
}  



/*开辟一个新的block*/
t_block extend_heap(t_block last,int size){
    t_block b;
    int s = calc_align(size, BLOCK_HEAD_SIZE); //size对齐8字节
    b = sbrk(0);         //指向当前break处
    if(sbrk(BLOCK_HEAD_SIZE+s)== (void*)-1){
        return NULL;
    }
    b->size = s;
 
    b->next = NULL;    //后驱指向NULL
    b->prev = last;    //前驱指向为最后一个节点
    b->ptr  = b->data; //保存当前块的data指针,用free时验证地址的有效性
    if(last)
        last->next = b;  //设置last的后驱
    b->free = 0;
    return b;
}
 
/*分裂block,分裂是为了提高内存利用率,由于是采用了first-fit方法,所以可能小size也占用了一块大的block*/
t_block split_block(t_block b,int size){
     t_block new;
     new = (t_block)(b->data + size);  //新的block首地址为b->data偏移size个字节
     new->free = 1;
     new->size = b->size - size -BLOCK_HEAD_SIZE;  //新block的大小为block->size-需要的size- block头部
     new->next = b->next;  //相当于插入了一个新的block
     b->size   = size;     
     b->next   = new;
     return b;
}
 
 
#define MIN_BLOCK_SIZE  (BLOCK_HEAD_SIZE +8)   //BLOCK 块最小字节
/*malloc的实现*/
void *malloc(size_t size){
     t_block last,b;
     int s = calc_align(size, BLOCK_HEAD_SIZE);  //对齐8字节
     if(first_block){
        /*查找合适的block*/
        last = first_block ;
        b    = find_block(&last,s);
        if(b){  //如果查找到看看能不能进行分裂以降低内存碎片  block最小为 BLOCK_HEAD_SIZE +8;
            if( (b->size -s) >= MIN_BLOCK_SIZE ){
                split_block(b,s);  //分裂BLOCK_SIZE
            }
            b->free =0;
        }else{  //如果没有找到则，从堆中新开辟一个block
            b = extend_heap(last,s);
            if(!b) return NULL;
        }
     }else{
        b = extend_heap(NULL,s);
        if(!b)
            return NULL;
        first_block = b;
     }
     return b->data;
}

void *memalign (size_t boundary, size_t size)
{
    return malloc(size);
}

/*分配连续n个size大小的联系内存空间,并对每个字节设置为0*/
void *calloc(int n,size_t size){
    t_block new;
    new = malloc(n*size);
    if(new){
        int *p = (int*)new->data;
        int i =0;
        int nbyte= new->size >> 2;
        for(i=0;i<nbyte;i++)  //为已分配的每个字节置0
            p[i] = 0;
    }
    return new->data;
}
 
/*相邻两个空闲block，合并为一个新的block,提高内存的利用率*/
t_block block_merge(t_block b){
    //如果next存在,并且空闲则合并
    if(b->next && b->next->free){
        b->size += BLOCK_HEAD_SIZE + b->next->size;
        b->next = b->next->next;
        if(b->next)
            b->next->prev = b;
        b->free =1;
    }
    return b;
}
 
/*free 函数实现*/
void free(void *p){
    t_block b;
    if(valid_addr(p)){  //验证地址是否合法
        b = get_block(p);
        b->free =1;
        if(b->prev && b->prev->free){  //如果前驱是空闲block
            b = block_merge(b->prev);
        }
        if(b->next)   //如果后续节点是空闲的block
            block_merge(b->next);
        else{//b是最后一个节点
            if(!b->prev)
                first_block = NULL;
            brk(b);   //设置break指针
        }
    }
}
 
/*realloc的实现*/
void copy_block(t_block src,t_block dest){
     int *p_src,*p_dest;
     p_src  = src->ptr; 
     p_dest = dest->ptr;
     if(dest->size < src->size) 
        return ;
     int  groups = src->size / sizeof(int);
     int  i=0;
     for(i=0;i<groups;i++){
            p_dest[i] = p_src[i];
     }
}
 
void *realloc(void *p,size_t size){
    t_block b,new;
    void *newp;
    if (!p && !size)
        return NULL;
    if(!p)
        return malloc(size);
    if (!size) {
        free(p);
        return NULL;
    }

    if(valid_addr(p)){
        int s = calc_align(size, BLOCK_HEAD_SIZE);
        b = get_block(p);
        if(b->size >=s){
            if((b->size -s)>=MIN_BLOCK_SIZE){
                split_block(b,s);  //分裂block
            } else {
                b->size = s;
            }
            return p;
        }else{
            //如果当前块与next块合并后能够满足size,则直接合并就好了
            if(b->next && b->next->free && (b->size+ BLOCK_HEAD_SIZE + b->next->size)>=s){
                b=block_merge(b);
                //如果合并后的剩余的空间能够再切分为一个block，则进行拆分
                if(b->size-s > MIN_BLOCK_SIZE)
                    b = split_block(b,s);
                
                return p;
            }else{ 
                newp = malloc(s);
                if(!newp)
                    return NULL;
                new = get_block(newp);
                copy_block(b,new);
                free(p);
                return newp;
            }   
        }
    }
    return NULL;
}