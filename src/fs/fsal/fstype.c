#include <xbook/list.h>
#include <xbook/fsal.h>
#include <xbook/fatfs.h>
#include <string.h>

LIST_HEAD(fstype_list_head);
DEFINE_SPIN_LOCK(fstype_lock);

int fstype_register(fsal_t *fsal)
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fstype_lock, irq_flags);
    list_add_tail(&fsal->list, &fstype_list_head);
    spin_unlock_irqrestore(&fstype_lock, irq_flags);
    return 0;
}

int fstype_unregister(fsal_t *fsal)
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fstype_lock, irq_flags);
    list_del(&fsal->list);
    spin_unlock_irqrestore(&fstype_lock, irq_flags);
    return 0;
}

fsal_t *fstype_find(char *name)
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fstype_lock, irq_flags);
    char **subtable;
    fsal_t *fsal;
    list_for_each_owner (fsal, &fstype_list_head, list) {
        if (fsal->subtable == NULL) {
            if (!strcmp(fsal->name, name)) {
                spin_unlock_irqrestore(&fstype_lock, irq_flags);
                return fsal;
            }
        } else {    /* 比较子表 */
            subtable = fsal->subtable;
            int i = 0;
            while (subtable[i] != NULL) {
                if (!strcmp(subtable[i], name)) {
                    spin_unlock_irqrestore(&fstype_lock, irq_flags);
                    return fsal;
                }
                i++;
            }
        }
    }
    spin_unlock_irqrestore(&fstype_lock, irq_flags);
    return NULL;
}

int fstype_init()
{
    INIT_LIST_HEAD(&fstype_list_head);
    /* 注册文件系统: FATFS */
    fstype_register(&fatfs_fsal);
    return 0;
}
