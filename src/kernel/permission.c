#include <xbook/account.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>

permission_database_t *permission_db;

static void permission_data_init(permission_data_t *permdata, uint32_t attr, char *str)
{
    memset(permdata->str, 0, PERMISION_DATA_LEN);
    if (str) {
        strcpy(permdata->str, str);
        permdata->str[strlen(str)] = '\0';
    }
    permdata->attr = attr;
}

void permission_database_dump()
{
    pr_dbg("Permission database length:%d\n", permission_db->length);
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        permission_data_t *data = &permission_db->datasets[i];
        if (data->attr) {
            pr_dbg("%d: attr:%x str:%s\n", i, data->attr, data->str);
        }
    }
}

int permission_database_init()
{
    permission_db = mem_alloc(sizeof(permission_database_t));
    if (!permission_db) {
        panic("permission database alloc failed!\n");
    }
    spinlock_init(&permission_db->lock);
    permission_db->length = 0;
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        permission_data_init(&permission_db->datasets[i], 0, NULL);
    }
    permission_database_load();
    permission_database_dump();
    return 0;
}

int permission_database_freesolt()
{
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (!permission_db->datasets[i].attr)
            break;
    }
    if (i >= PERMISION_DATABASE_LEN)
        return -1;
    return i;
}

int permission_database_insert(uint32_t attr, char *str)
{
    unsigned long flags;
    spin_lock_irqsave(&permission_db->lock, flags);
    if (permission_db->length >= PERMISION_DATABASE_LEN) {
        spin_unlock_irqrestore(&permission_db->lock, flags);    
        return -1;
    }
    int solt = permission_database_freesolt();
    if (solt < 0) {
        spin_unlock_irqrestore(&permission_db->lock, flags);    
        return -1;
    }
    permission_data_init(&permission_db->datasets[solt], attr, str);
    permission_db->length++;
    spin_unlock_irqrestore(&permission_db->lock, flags);    
    return solt;
}

int permission_database_delete(uint32_t index)
{
    unsigned long flags;
    spin_lock_irqsave(&permission_db->lock, flags);
    if (index >= PERMISION_DATABASE_LEN) {
        spin_unlock_irqrestore(&permission_db->lock, flags);    
        return -1;
    }
    permission_data_init(&permission_db->datasets[index], 0, NULL);
    permission_db->length--;
    spin_unlock_irqrestore(&permission_db->lock, flags);    
    return 0;
}

int permission_database_delete_by_data(char *str)
{
    unsigned long flags;
    spin_lock_irqsave(&permission_db->lock, flags);
    
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (!strcmp(permission_db->datasets[i].str, str)) {
            permission_data_init(&permission_db->datasets[i], 0, NULL);
            permission_db->length--;
            spin_unlock_irqrestore(&permission_db->lock, flags);            
            return 0;
        }
    }
    spin_unlock_irqrestore(&permission_db->lock, flags);    
    return -1;
}

permission_data_t *permission_database_select(char *str)
{
    unsigned long flags;
    spin_lock_irqsave(&permission_db->lock, flags);
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        permission_data_t *data = &permission_db->datasets[i];
        if (data->attr && !strcmp(data->str, str)) {
            spin_unlock_irqrestore(&permission_db->lock, flags);        
            return data;
        }
    }
    spin_unlock_irqrestore(&permission_db->lock, flags);    
    return NULL;
}

permission_data_t *permission_database_select_by_index(uint32_t index)
{
    if (index >= PERMISION_DATABASE_LEN) {
        return NULL;   
    }
    return &permission_db->datasets[index];
}

void permission_database_foreach(void (*callback)(void *, void *) , void *arg)
{
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (permission_db->datasets[i].attr)
            callback(arg, &permission_db->datasets[i]);
    }
}

int permission_database_sync()
{
    unsigned long flags;
    spin_lock_irqsave(&permission_db->lock, flags);
    
    /* TODO: 根据权限数据库内容，把他们写入到文件中去 */
    
    spin_unlock_irqrestore(&permission_db->lock, flags);    
    return 0;
}

int permission_database_load()
{
    /* TODO: 从文件中加载到数据库中 */

    /* 动态加载 */
    char *str;
    str = "disk0"; permission_database_insert(PERMISION_ATTR_DEVICE | PERMISION_ATTR_RDWR, str);
    str = "disk1"; permission_database_insert(PERMISION_ATTR_DEVICE | PERMISION_ATTR_RDWR, str);
    str = "/account"; permission_database_insert(PERMISION_ATTR_FILE | PERMISION_ATTR_RDWR, str);
    str = "/sbin/init"; permission_database_insert(PERMISION_ATTR_FILE | PERMISION_ATTR_RDWR, str);
    
    return 0;
}
