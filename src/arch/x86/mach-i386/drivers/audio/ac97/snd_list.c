#include "snd_list.h"


/* 把一个新的节点new插入到prev后，next前 */
  void __snd_list_add(struct snd_list *new_, 
                              struct snd_list *prev, 
                              struct snd_list *next)
{
   //new和next绑定关系
   next->prev = new_; 
   new_->next = next; 
   //new和next绑定关系
   new_->prev = prev; 
   prev->next = new_; 
}


/* 把一个节点从链表中删除 */
  void __snd_list_del(struct snd_list *prev, struct snd_list *next)
{
   // ^_^ 把前一个和下一个进行关联，那中间一个就被删去了
   next->prev = prev;
   prev->next = next;
}

/* 把一个节点从链表中删除 */
  void __snd_list_del_node(struct snd_list *node)
{
   // 传入节点的前一个和后一个，执行后只是脱离链表，而自身还保留了节点信息
   __snd_list_del(node->prev, node->next);
}






/* 让链表内容指针指向自己本身 */
  void snd_list_init(struct snd_list *snd_list)
{
   snd_list->next = snd_list;
   snd_list->prev = snd_list;  
}

/*
 * snd_list_add_tail - 添加一个新的节点到链表尾
 * @new： 要新添加的节点
 * @head：要添加到哪个链表头
 * 
 * 把一个节点添加到链表头前面，相当于添加到整个链表的最后面。
 */
void snd_list_add_tail(struct snd_list *new_, struct snd_list *head)
{
   // :) 插入到链表头前一个和链表头之间
   __snd_list_add(new_, head->prev, head);
}


/*
 * snd_list_find - 从前往后遍历查找链表节点
 * @snd_list: 要查找的节点指针
 * @head: 链表头
 * 
 * 找到返回1，否则返回0 
 */
  int snd_list_find(struct snd_list *snd_list, struct snd_list *head)
{
   struct snd_list *node;
   snd_list_for_each(node, head)
   {
      // 找到一样的
      if (node == snd_list) {
         return 1;
      }
   }
   return 0;
}


/*
 * snd_list_del - 把节点从链表中删除
 * @node：要删除的节点
 * 
 * 把一个已经存在于链表中的节点删除
 */
  void snd_list_del(struct snd_list *node)
{
   __snd_list_del_node(node);
   // @.@ 把前驱指针和后驱指针都指向空，完全脱离
   node->prev = NULL;
   node->next = NULL;
}