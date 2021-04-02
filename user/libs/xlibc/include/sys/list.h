#ifndef _SYS_LIST_H
#define _SYS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* 
 * 链表数据结构，在看了Linux的链表结构之后，觉得他那个比较通用，
 * 而且也比较清晰，所以我打算移植一个过来。本文件里面的都是内联函数，
 * 使用的时候会编译在调用的地方。并且还有很多宏定义。
 */

/*
 * 链表结构体
 */
typedef struct list {
   struct list *prev;
   struct list *next;
} list_t;

/* 为链表结构赋值 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

/* 创建并赋值 */
#define LIST_HEAD(name) \
        struct list name = LIST_HEAD_INIT(name)

/* 让链表内容指针指向自己本身 */
static inline void list_init(struct list *list)
{
   list->next = list;
   list->prev = list;  
}

/* 把一个新的节点new插入到prev后，next前 */
static inline void __list_add(struct list *new, 
                              struct list *prev, 
                              struct list *next)
{
   //new和next绑定关系
   next->prev = new; 
   new->next = next; 
   //new和next绑定关系
   new->prev = prev; 
   prev->next = new; 
}

/*
 * list_add - 添加一个新的节点到链表头
 * @new： 要新添加的节点
 * @head：要添加到哪个链表头
 * 
 * 把一个节点添加到链表头后面，相当于添加到整个链表的最前面。
 */
static inline void list_add(struct list *new, struct list *head)
{
   // :) 插入到链表头和链表头的下一个节点之间
   __list_add(new, head, head->next);
}

/*
 * list_add_before - 把节点添加到一个节点前面
 * @new： 要新添加的节点
 * @head：比较的节点
 * 
 * 把一个新节点添加到一个节点前面。旧节点的前驱需要指向新节点，
 * 旧节点的前驱指向新节点，新节点的前驱指向旧节点的前驱，后驱指向旧节点。
 * 
 */
static inline void list_add_before(struct list *new, struct list *node)
{
   node->prev->next = new;

   new->prev = node->prev;
   new->next = node;

   node->prev = new;
}

/*
 * list_add_after - 把节点添加到一个节点后面
 * @new： 要新添加的节点
 * @head：比较的节点
 * 
 * 把一个新节点添加到一个节点后面。旧节点的后驱需要指向新节点，
 * 旧节点的后驱指向新节点，新节点的前驱指向旧节点，后驱指向旧节点的后驱。
 * 
 */
static inline void list_add_after(struct list *new, struct list *node)
{
   node->next->prev = new;

   new->prev = node;
   new->next = node->next;

   node->next = new;
}

/*
 * list_add_tail - 添加一个新的节点到链表尾
 * @new： 要新添加的节点
 * @head：要添加到哪个链表头
 * 
 * 把一个节点添加到链表头前面，相当于添加到整个链表的最后面。
 */
static inline void list_add_tail(struct list *new, struct list *head)
{
   // :) 插入到链表头前一个和链表头之间
   __list_add(new, head->prev, head);
}

/* 把一个节点从链表中删除 */
static inline void __list_del(struct list *prev, struct list *next)
{
   // ^_^ 把前一个和下一个进行关联，那中间一个就被删去了
   next->prev = prev;
   prev->next = next;
}

/* 把一个节点从链表中删除 */
static inline void __list_del_node(struct list *node)
{
   // 传入节点的前一个和后一个，执行后只是脱离链表，而自身还保留了节点信息
   __list_del(node->prev, node->next);
}

/*
 * list_del - 把节点从链表中删除
 * @node：要删除的节点
 * 
 * 把一个已经存在于链表中的节点删除
 */
static inline void list_del(struct list *node)
{
   __list_del_node(node);
   // @.@ 把前驱指针和后驱指针都指向空，完全脱离
   node->prev = NULL;
   node->next = NULL;
}

/*
 * list_del_init - 把节点从链表中删除
 * @node：要删除的节点
 * 
 * 把一个已经存在于链表中的节点删除
 */
static inline void list_del_init(struct list *node)
{
   __list_del_node(node);
   //初始化节点，使得可以成为链表头，我猜的。:-)
   list_init(node);
}

/*
 * list_replace - 用新的节点替代旧的节点
 * @old：旧的节点
 * @new：要插入的新节点
 * 
 * 用一个节点替代已经存在于链表中的节点
 */
static inline void list_replace(struct list *old, struct list *new)
{
   /*
   @.@ 把old的前后指针都指向new，那么new就替代了old，真可恶！
   不过，旧的节点中还保留了链表的信息
   */
   new->next = old->next;
   new->next->prev = new;
   new->prev = old->prev;
   new->prev->next = new;
}

static inline void list_replace_init(struct list *old, struct list *new)
{
   /*
   先把old取代，然后把old节点初始化，使它完全脱离链表。
   */
   list_replace(old, new);
   list_init(old);
}

/*
 * list_move - 从一个链表删除，然后移动到另外一个链表头后面
 * @node：要操作的节点
 * @head：新的链表头
 */
static inline void list_move(struct list *node, struct list *head)
{
   // ^.^ 先把自己脱离关系，然后添加到新的链表
   __list_del_node(node);
   list_add(node, head);   
}

/*
 * list_move_tail - 从一个链表删除，然后移动到另外一个链表头前面
 * @node：要操作的节点
 * @head：新的链表头
 */
static inline void list_move_tail(struct list *node, struct list *head)
{
   // ^.^ 先把自己脱离关系，然后添加到新的链表
   __list_del_node(node);
   list_add_tail(node, head);   
}

/*
 * list_is_first - 检测节点是否是链表中的第一个节点
 * @node：要检测的节点
 * @head：链表头
 */
static inline int list_is_first(const struct list *node, 
                                 const struct list *head)
{
   return (node->prev == head); //节点的前一个是否为链表头
}

/*
 * list_is_last - 检测节点是否是链表中的最后一个节点
 * @node：要检测的节点
 * @head：链表头
 */
static inline int list_is_last(const struct list *node, 
                                 const struct list *head)
{
   return (node->next == head); //节点的后一个是否为链表头
}

/*
 * list_empty - 测试链表是否为空链表
 * @head：链表头
 * 
 * 把链表头传进去，通过链表头来判断
 */
static inline int list_empty(const struct list *head)
{
   return (head->next == head); //链表头的下一个是否为自己
}

/* ！！！！前方！！！！高能！！！！ */

/*
 * list_owner - 获取节点的宿主
 * @ptr： 节点的指针
 * @type： 宿主结构体的类型
 * @member: 节点在宿主结构体中的名字 
 */
#define list_owner(ptr, type, member) container_of(ptr, type, member)

/* 嘻嘻，就这样就把container_of用上了 */

/*
 * list_first_owner - 获取链表中的第一个宿主
 * @head： 链表头
 * @type： 宿主结构体的类型
 * @member: 节点在宿主结构体中的名字 
 * 
 * 注：链表不能为空
 */
#define list_first_owner(head, type, member) \
      list_owner((head)->next, type, member)


/*
 * list_last_owner - 获取链表中的最后一个宿主
 * @head:  链表头
 * @type： 宿主结构体的类型
 * @member: 节点在宿主结构体中的名字 
 * 
 * 注：链表不能为空
 */
#define list_last_owner(head, type, member) \
      list_owner((head)->prev, type, member)

/*
 * list_first_owner_or_null - 获取链表中的第一个宿主
 * @head： 链表头
 * @type： 宿主结构体的类型
 * @member: 节点在宿主结构体中的名字 
 * 
 * 注：如果链表是空就返回NULL
 */
#define list_first_owner_or_null(head, type, member) ({ \
      struct list *__head = (head); \
      struct list *__pos = (__head->next); \
      __pos != __head ? list_owner(__pos, type, member) : NULL; \
})

/*
 * list_last_owner_or_null - 获取链表中的最后一个宿主
 * @head： 链表头
 * @type： 宿主结构体的类型
 * @member: 节点在宿主结构体中的名字 
 * 
 * 注：如果链表是空就返回NULL
 */
#define list_last_owner_or_null(head, type, member) ({ \
      struct list *__head = (head); \
      struct list *__pos = (__head->prev); \
      __pos != __head ? list_owner(__pos, type, member) : NULL; \
})

/*
 * list_next_owner - 获取链表中的下一个宿主
 * @pos： 临时宿主的指针
 * @member: 节点在宿主结构体中的名字 
 */
#define list_next_owner(pos, member) \
      list_owner((pos)->member.next, typeof(*(pos)), member)

/*
 * list_prev_onwer - 获取链表中的前一个宿主
 * @pos： 临时宿主的指针
 * @member: 节点在宿主结构体中的名字 
 */
#define list_prev_onwer(pos, member) \
      list_owner((pos)->member.prev, typeof(*(pos)), member)

/* 把代码自己打一遍，好累啊！但是感觉这些东西也更加明白了 */

/* 记住啦，这是遍历链表节点，不是宿主 -->>*/

/*
 * list_for_each - 从前往后遍历每一个链表节点
 * @pos： 节点指针
 * @head: 链表头 
 */
#define list_for_each(pos, head) \
      for (pos = (head)->next; pos != (head); pos = pos->next)

/*
 * list_find - 从前往后遍历查找链表节点
 * @list: 要查找的节点指针
 * @head: 链表头
 * 
 * 找到返回1，否则返回0 
 */
static inline int list_find(struct list *list, struct list *head)
{
   struct list *node;
   list_for_each(node, head) {
      // 找到一样的
      if (node == list) {
         return 1;
      }
   }
   return 0;
}

/*
 * list_length - 获取链表长度
 * @head: 链表头
 */
static inline int list_length(struct list *head)
{
   struct list *list;
   int n = 0;
   list_for_each(list, head) {
      // 找到一样的
      if (list == head)
         break;
      n++;
   }
   return n;
}

/*
 * list_for_each_prev - 从后往前遍历每一个链表节点
 * @pos： 节点指针
 * @head: 链表头 
 */
#define list_for_each_prev(pos, head) \
      for (pos = (head)->prev; pos != (head); pos = pos->prev)

/*
 * list_for_each_safe - 从前往后遍历每一个链表节点
 * @pos: 节点指针
 * @_next: 临时节点指针（为了避免和pos->next混淆，在前面加_）
 * @head: 链表头 
 * 
 * 用next来保存下一个节点指针，如果在遍历过程中pos出的节点被删除了，
 * 还是可以继续往后面遍历其它节点。
 */
#define list_for_each_safe(pos, _next, head) \
      for (pos = (head)->next, _next = pos->next; pos != (head); \
         pos = _next, _next = pos->next)

/*
 * list_for_each_prev_safe - 从后往前遍历每一个链表节点
 * @pos: 节点指针
 * @_prev: 临时节点指针（为了避免和pos->prev混淆，在前面加_）
 * @head: 链表头 
 * 
 * 用prev来保存前一个节点指针，如果在遍历过程中pos出的节点被删除了，
 * 还是可以继续往前面遍历其它节点。
 */
#define list_for_each_prev_safe(pos, _prev, head) \
      for (pos = (head)->prev, _prev = pos->prev; pos != (head); \
         pos = _prev, _prev = pos->prev)

/*  <<-- 遍历链表节点结束了，接下来开始的是遍历宿主 -->> */

/*
 * list_for_each_owner - 从前往后遍历每一个链表节点宿主
 * @pos: 宿主类型结构体指针
 * @head: 链表头 
 * @member: 节点在宿主中的名字
 */
#define list_for_each_owner(pos, head, member)                    \
      for (pos = list_first_owner(head, typeof(*pos), member);   \
         &pos->member != (head);                               \
         pos = list_next_owner(pos, member))

/*
 * list_for_each_owner_reverse - 从后往前遍历每一个链表节点宿主
 * @pos: 宿主类型结构体指针
 * @head: 链表头 
 * @member: 节点在宿主中的名字
 */
#define list_for_each_owner_reverse(pos, head, member)            \
      for (pos = list_last_owner(head, typeof(*pos), member);   \
         &pos->member != (head);                              \
         pos = list_prev_onwer(pos, member))

/*
 * list_for_each_owner_safe - 从前往后遍历每一个链表节点宿主
 * @pos: 宿主类型结构体指针
 * @next: 临时指向下一个节点的指针
 * @head: 链表头 
 * @member: 节点在宿主中的名字
 * 
 * 可以保证在遍历过程中如果
 */
#define list_for_each_owner_safe(pos, next, head, member)          \
      for (pos = list_first_owner(head, typeof(*pos), member),   \
         next = list_next_owner(pos, member);                    \
         &pos->member != (head);                               \
         pos = next, next = list_next_owner(next, member))

/*
 * list_for_each_owner_reverse_safe - 从后往前遍历每一个链表节点宿主
 * @pos: 宿主类型结构体指针
 * @_prev: 临时指向前一个节点的指针
 * @head: 链表头 
 * @member: 节点在宿主中的名字
 * 
 * 可以保证在遍历过程中如果
 */
#define list_for_each_owner_reverse_safe(pos, prev, head, member)   \
      for (pos = list_last_owner(head, typeof(*pos), member),    \
         prev = list_prev_onwer(pos, member);                    \
         &pos->member != (head);                               \
         pos = prev, prev = list_prev_onwer(prev, member))

/*  <<-- 遍历链表宿主也结束了，very nice 啊！ */


/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
struct hlist_head {
	struct hlist_node * first;
};

struct hlist_node {
	struct hlist_node * next, ** pprev;
};

#define HLIST_HEAD(name) \
	struct hlist_head name = { .first = NULL }

static inline void init_hlist_head(struct hlist_head * hlist)
{
	hlist->first = NULL;
}

static inline void init_hlist_node(struct hlist_node * h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node * h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head * h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node * n)
{
	struct hlist_node * next = n->next;
	struct hlist_node ** pprev = n->pprev;

	*pprev = next;
	if(next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node * n)
{
	__hlist_del(n);
	n->next = 0;
	n->pprev = 0;
}

static inline void hlist_del_init(struct hlist_node * n)
{
	if(!hlist_unhashed(n))
	{
		__hlist_del(n);
		init_hlist_node(n);
	}
}

static inline void hlist_add_head(struct hlist_node * n, struct hlist_head * h)
{
	struct hlist_node * first = h->first;
	n->next = first;
	if(first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node * n,
					struct hlist_node * next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void hlist_add_behind(struct hlist_node * n,
				    struct hlist_node * prev)
{
	n->next = prev->next;
	prev->next = n;
	n->pprev = &prev->next;

	if(n->next)
		n->next->pprev = &n->next;
}

/* after that we'll appear to be on some hlist and hlist_del will work */
static inline void hlist_add_fake(struct hlist_node * n)
{
	n->pprev = &n->next;
}

static inline int hlist_fake(struct hlist_node * h)
{
	return h->pprev == &h->next;
}

/*
 * Check whether the node is the only node of the head without
 * accessing head:
 */
static inline int hlist_is_singular_node(struct hlist_node * n, struct hlist_head * h)
{
	return !n->next && n->pprev == &h->first;
}

/*
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move_list(struct hlist_head * old,
				   struct hlist_head * new)
{
	new->first = old->first;
	if(new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define hlist_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

#define hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
	})

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(pos, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); \
	     pos; \
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(pos, member) \
	for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member); \
	     pos; \
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(pos, member) \
	for (; pos; \
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(pos, n, head, member) \
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); \
	     pos && ({ n = pos->member.next; 1; }); \
	     pos = hlist_entry_safe(n, typeof(*pos), member))

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_LIST_H */
