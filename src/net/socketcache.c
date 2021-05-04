#include <xbook/socketcache.h>
#include <xbook/debug.h>
#include <xbook/memalloc.h>

static DEFINE_SPIN_LOCK_UNLOCKED(socket_cache_lock);
static LIST_HEAD(socket_cache_list_head);

socket_cache_t *socket_cache_create(int socket)
{
    socket_cache_t *socache = mem_alloc(sizeof(socket_cache_t));
    if (!socache) {
        return NULL;
    }
    unsigned long iflags;
    spin_lock_irqsave(&socket_cache_lock, iflags);
    list_add(&socache->list, &socket_cache_list_head);
    socache->socket = socket;
    atomic_set(&socache->reference, 1);
    spin_unlock_irqrestore(&socket_cache_lock, iflags);
    return socache;
}

int socket_cache_destroy(socket_cache_t *socache)
{
    if (!socache) {
        return -1;
    }
    if (atomic_get(&socache->reference) < 0) {
        warnprint("socket cache: destroy reference %d error!" endl, atomic_get(&socache->reference));
    }
    unsigned long iflags;
    spin_lock_irqsave(&socket_cache_lock, iflags);
    list_del_init(&socache->list);
    spin_unlock_irqrestore(&socket_cache_lock, iflags);
    mem_free(socache);
    return 0;
}

int socket_cache_inc(socket_cache_t *socache)
{
    if (!socache) {
        return -1;
    }
    if (atomic_get(&socache->reference) < 0) {
        warnprint("socket cache: inc reference %d error!" endl, atomic_get(&socache->reference));
    }
    atomic_inc(&socache->reference);
    return 0;    
}

int socket_cache_dec(socket_cache_t *socache)
{
    if (!socache) {
        return -1;
    }   
    if (atomic_get(&socache->reference) < 0) {
        warnprint("socket cache: dec reference %d error!" endl, atomic_get(&socache->reference));
    }
    atomic_dec(&socache->reference);
    return 0;    
}

socket_cache_t *socket_cache_find(int socket)
{
    if (socket < 0) {
        return NULL;
    }
    socket_cache_t *tmp, *found = NULL;
    unsigned long iflags;
    spin_lock_irqsave(&socket_cache_lock, iflags);
    list_for_each_owner (tmp, &socket_cache_list_head, list) {
        if (tmp->socket == socket) {
            found = tmp;
            break;
        }
    }
    spin_unlock_irqrestore(&socket_cache_lock, iflags);
    return found;    
}
