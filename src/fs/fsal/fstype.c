#include <xbook/list.h>
#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <string.h>

LIST_HEAD(fstype_list_head);

int fstype_register(fsal_t *fsal)
{
    list_add_tail(&fsal->list, &fstype_list_head);
    return 0;
}

int fstype_unregister(fsal_t *fsal)
{
    list_del(&fsal->list);
    return 0;
}

fsal_t *fstype_find(char *name)
{
    char **subtable;
    fsal_t *fsal;
    list_for_each_owner (fsal, &fstype_list_head, list) {
        if (fsal->subtable == NULL) {
            if (!strcmp(fsal->name, name)) {
                return fsal;
            }
        } else {    /* 比较子表 */
            subtable = fsal->subtable;
            int i = 0;
            while (subtable[i] != NULL) {
                if (!strcmp(subtable[i], name)) {
                    return fsal;
                }
                i++;
            }
        }
    }
    return NULL;
}

int fstype_init()
{
    INIT_LIST_HEAD(&fstype_list_head);
    /* 注册文件系统: FATFS */
    fstype_register(&fatfs_fsal);
    return 0;
}
