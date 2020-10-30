#include <xbook/list.h>
#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <string.h>

/* 文件系统类型链表，管理所有的文件系统 */
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
        if (fsal->subtable == NULL) {   /* 没有子表，就比较名字 */
            if (!strcmp(fsal->name, name)) {    /* 比较文件系统是否找到 */
                return fsal;
            }
        } else {    /* 比较子表 */
            subtable = fsal->subtable;
            int i = 0;
            while (subtable[i] != NULL) {
                if (!strcmp(subtable[i], name)) {    /* 比较文件系统是否找到 */
                    return fsal;
                }
                i++;
            }
        }
    }
    return NULL;
}

int init_fstype()
{
    INIT_LIST_HEAD(&fstype_list_head);

    /* 注册文件系统: FATFS */
    fstype_register(&fatfs_fsal);

    return 0;
}
