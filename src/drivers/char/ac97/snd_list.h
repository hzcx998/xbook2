#ifndef __SND_LIST_H
#define __SND_LIST_H
#include <stddef.h>
#define snd_list_for_each(pos, head) \
      for (pos = (head)->next; pos != (head); pos = pos->next)

typedef struct snd_list {
   struct snd_list *prev;
   struct snd_list *next;
   void * snd_device;
} snd_list_t;

 void __snd_list_add(struct snd_list *new_, 
                              struct snd_list *prev, 
                              struct snd_list *next);


void snd_list_del(struct snd_list *node);

int snd_list_find(struct snd_list *snd_list, struct snd_list *head);

void snd_list_add_tail(struct snd_list *new_, struct snd_list *head);

#endif