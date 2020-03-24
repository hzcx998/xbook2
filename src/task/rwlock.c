#include <xbook/rwlock.h>


void rwlock_init(rwlock_t *lock, enum rwlock_arg arg)
{
    lock->count = 0;    /* no reader */
    
    mutexlock_init(&lock->count_mutex);
    mutexlock_init(&lock->rw_mutex);
    mutexlock_init(&lock->write_mutex);

    switch (arg)
    {
    case RWLOCK_RD_FIRST:
        lock->read_lock = __rwlock_read_lock_rd_first;
        lock->read_unlock = __rwlock_read_unlock_rd_first;
        lock->write_lock = __rwlock_write_lock_rd_first;
        lock->write_unlock = __rwlock_write_unlock_rd_first;
        break;
    case RWLOCK_WR_FIRST:
        lock->read_lock = __rwlock_read_lock_wr_first;
        lock->read_unlock = __rwlock_read_unlock_wr_first;
        lock->write_lock = __rwlock_write_lock_wr_first;
        lock->write_unlock = __rwlock_write_unlock_wr_first;
        break;
    case RWLOCK_RW_FAIR:
        lock->read_lock = __rwlock_read_lock_rw_fair;
        lock->read_unlock = __rwlock_read_unlock_rw_fair;
        lock->write_lock = __rwlock_write_lock_rw_fair;
        lock->write_unlock = __rwlock_write_unlock_rw_fair;
        break;
    default: /* 参数出错，置空 */
        lock->write_lock = lock->read_lock = NULL;
        lock->write_unlock = lock->read_unlock = NULL;
        break;
    }
} 
