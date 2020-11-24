#include <xbook/account.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <fsal/path.h>

/*
磁盘数据库设计

账户索引文件：
账户名：密码：账户属性：权限文档
account
admin,1234,0
jason,4567,02
zhuyu,abcd,02

权限库文件：
dbdata
资源类型，属性：数据
01,/home/jason
02,disk0
03,disk1
*/

account_t *account_table;
account_t *account_current; /* 当前登录的账户 */
DEFINE_MUTEX_LOCK(account_mutex_lock);

extern permission_database_t *permission_db;

static void account_init(account_t *account)
{
    memset(account->name, 0, ACCOUNT_NAME_LEN);
    memset(account->password, 0, ACCOUNT_PASSWORD_LEN);
    account->flags = 0;
    int i;
    for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        account->data_index[i] = -1;
    }
    account->index_len = 0;
    spinlock_init(&account->lock);
}

static account_t *account_alloc()
{
    mutex_lock(&account_mutex_lock);
    account_t *account;
    int i;
    for (i = 0; i < ACCOUNT_NR; i++) {
        account = account_table + i;
        if (!(account->flags & ACCOUNT_FLAG_USED)) {
            account_init(account);
            account->flags |= ACCOUNT_FLAG_USED;
            mutex_unlock(&account_mutex_lock);
            return account;
        }
    }
    mutex_unlock(&account_mutex_lock);
    return NULL;
}

static int account_free(account_t *account)
{
    if (!account)
        return -1;
    mutex_lock(&account_mutex_lock);
    account->flags = 0;
    mutex_unlock(&account_mutex_lock); 
    return 0;
}

void account_dump()
{
    int i;
    for (i = 0; i < ACCOUNT_NR; i++) {
        account_t *account = account_table + i;
        if (account->flags & ACCOUNT_FLAG_USED)
            pr_dbg("Account:%s pwd:%s attr:%x\n", account->name, account->password, account->flags);
    }
}

static account_t *account_find_by_name(const char *name)
{
    mutex_lock(&account_mutex_lock);
    account_t *account;
    int i;
    for (i = 0; i < ACCOUNT_NR; i++) {
        account = account_table + i;
        if ((account->flags & ACCOUNT_FLAG_USED) && (!strcmp(name, account->name))) {
            mutex_unlock(&account_mutex_lock);
            return account;
        }
    }
    mutex_unlock(&account_mutex_lock);
    return NULL;
}

int account_push(const char *name, char *password, uint32_t attr)
{
    account_t *account = account_alloc();
    if (!account)
        return -1;
    memcpy(account->name, name, min(strlen(name), ACCOUNT_NAME_LEN - 1));
    account->name[ACCOUNT_NAME_LEN - 1] = '\0';
    memcpy(account->password, password, min(strlen(password), ACCOUNT_PASSWORD_LEN - 1));
    account->password[ACCOUNT_PASSWORD_LEN - 1] = '\0';
    account->flags |= attr;
    return 0;
}

int account_pop(const char *name)
{
    account_t *account = account_find_by_name(name);
    if (!account)
        return -1;
    account_free(account);
    return 0;
}

/* 读取配置，存放到账户库 */
int account_read_config()
{
    // TODO:read config file
    // load account to account table
    
    // default admin account
    if (account_push(ADMIN_ACCOUNT_NAME, ADMIN_ACCOUNT_PASSWORD, ACCOUNT_LEVEL_ADMIN) < 0)
        return -1;

    return 0;
}

int account_sync_data()
{
    /* TODO: 根据账户表内存情况，把数据写入到磁盘文件中去  */
    return 0;
}

int account_sync()
{
    /* 同步账户数据库 */
    if (account_sync_data() < 0)
        return -1;

    /* 同步权限数据库 */
    if (permission_database_sync() < 0)
        return -1;
    return 0;
}

int account_login(const char *name, char *password)
{
    if (!name || !password)
        return -1;
    account_t *account = account_find_by_name(name);
    if (!account) {
        pr_err("account %s not exist!\n", name);
        return -1;
    }
    if (account == account_current) {
        pr_err("account %s had logined!\n", name);        
        return -1;
    }
    /* 之前登陆过，但是没有退出登录，强制退出其权限  */
    if (account->flags & ACCOUNT_FLAG_LOGINED) {
        account_debind_perm(account);
        account->flags &= ~ACCOUNT_FLAG_LOGINED;
    }
    if (strcmp(account->password, password) != 0) {
        pr_err("password %s not match!\n", name);
        return -1;
    }
    /* 登录时才从权限数据库中把权限加载到账户的权限数据索引表 */
    account_bind_perm(account);
    mutex_lock(&account_mutex_lock);
    account_current = account;
    account_current->flags |= ACCOUNT_FLAG_LOGINED;
    mutex_unlock(&account_mutex_lock);
    return 0;   /* login success */
}

int account_logout(const char *name)
{
    if (!name)
        return -1;
    account_t *account = account_find_by_name(name);
    if (!account) {
        pr_err("account %s not exist!\n", name);
        return -1;
    }
    if (!account_current) {
        pr_err("Current account is null, can't unregister!\n");
        return -1;
    }
    if (account != account_current) {
        pr_err("account %s not current account!\n", name);        
        return -1;   
    }
    mutex_lock(&account_mutex_lock);
    if (account_current != account) {
        pr_err("account %s not current!\n", name);
        mutex_unlock(&account_mutex_lock);
        return -1;
    }      
    /* 退出时情况权限数据索引表 */  
    account_debind_perm(account_current);
    account_current->flags &= ~ACCOUNT_FLAG_LOGINED;
    account_current = NULL;
    mutex_unlock(&account_mutex_lock);
    return 0;   /* logout success */
}

int account_register(const char *name, char *password, uint32_t flags)
{
    if (!name || !password)
        return -1;
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (account) {
        pr_err("account %s had existed!\n", name);
        return -1;
    }
    if (account_push(name, password, flags) < 0) {
        pr_err("account %s push into account table failed!\n", name);
        return -1;
    }
    /* 创建一个账户对应的主页路径权限 */
    char str[42] = {0};
    strcpy(str, HOME_DIR_PATH);
    strcat(str, "/");
    strcat(str, name);
    int index = permission_database_insert(PERMISION_ATTR_HOME | PERMISION_ATTR_RDWR | PERMISION_ATTR_FILE, str);
    if (index < 0) {
        pr_err("account %s insert home failed!\n", name);
        account_pop(name);
        return -1;
    }

    if (account_sync() < 0) {
        pr_err("account sync failed!\n", name);
        permission_database_delete(index);
        account_pop(name);
        return -1;
    }
    return 0;
}

int account_unregister(const char *name)
{
    if (!name)
        return -1;
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (!account)
        return -1;

    if (!account_current) {
        pr_err("Current account is null, can't unregister!\n");
        return -1;
    }

    if (account == account_current) {
        pr_err("Account %s using, can't unregister!\n", account_current->name);
        return -1;
    }
    mutex_lock(&account_mutex_lock);
    /* 必须要更高的权限才能进行注销 */
    int cur_level = account_current->flags & ACCOUNT_LEVEL_MASK;
    int acc_level = account->flags & ACCOUNT_LEVEL_MASK;
    if (cur_level >= acc_level) {
        pr_err("Account %s no permission to unregister account %s!\n", 
            account_current->name, account->name);
        mutex_unlock(&account_mutex_lock);
        return -1;
    }
    mutex_unlock(&account_mutex_lock);
    
    /* 删除一个主页路径权限 */
    char str[42] = {0};
    strcpy(str, HOME_DIR_PATH);
    strcat(str, "/");
    strcat(str, name);
    permission_data_t *data_ptr = permission_database_select(str); // 备份数据
    if (!data_ptr) {
        pr_err("Account %s home data not found failed!\n", name);
        return -1;
    }
    permission_data_t data_backup = *data_ptr;
    if (permission_database_delete_by_data(str) < 0) {
        pr_err("Account %s delete data from database failed!\n", name);
        return -1;
    }

    account_t account_backup = *account;    // 备份账户
    if (account_pop(name) < 0) {
        pr_err("Account %s pop from account table failed!\n", name);
        permission_database_insert(data_backup.attr, data_backup.str); // 恢复数据
        return -1;
    }
    
    if (account_sync() < 0) {
        pr_emerg("Sync account when unregister failed!\n");
        account_push(account_backup.name, account_backup.password,
                account_backup.flags & ACCOUNT_LEVEL_MASK);   //恢复账户
        permission_database_insert(data_backup.attr, data_backup.str);
        return -1;
    }
    return 0;
}

int account_add_index(account_t *account, uint32_t index)
{
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (account->data_index[i] == -1) {
            account->data_index[i] = index;
            break;
        }
    }
    if (i >= PERMISION_DATABASE_LEN)
        return -1;
    return 0;
}

int account_del_index(account_t *account, uint32_t index)
{
    if (index >= PERMISION_DATABASE_LEN)
        return -1;
    account->data_index[index] = -1;
    return 0;
}

void __account_bind_perm(void *arg, void *self)
{
    account_t *account = (account_t *) arg;
    permission_data_t *data = (permission_data_t *) self;
    if (data->attr & PERMISION_ATTR_HOME) {
        char *path = (char *) data->str;
        char *slash = strrchr(path, '/');
        if (slash) {
            slash++;
            if (!strcmp(slash, account->name)) {
                return; // 账户不屏蔽自己的主页
            }
        }
    }
    account_add_index(account, data - permission_db->datasets);
}

int account_bind_perm(account_t *account)
{
    // admin no perm limit
    if ((account->flags & ACCOUNT_LEVEL_MASK) < ACCOUNT_LEVEL_USER) {
        account->index_len = 0;
    } else {
        // 绑定所有限制，除开自己的home
        permission_database_foreach(__account_bind_perm, account);
    }
    return 0;
}

int account_debind_perm(account_t *account)
{
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        account->data_index[i] = -1;
    }
    return 0;
}
void account_dump_datasets(const char *name)
{
    account_t *account = account_find_by_name(name);
    if (!account)
        return;
    pr_dbg("Account datasets:\n");
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (account->data_index[i] >= 0) {
            pr_dbg("solt %d :index %d\n", i, account->data_index[i]);
        }
    }
}

/* 如果监测到数据有权限，返回0 */
int account_check_permission(account_t *account, char *str, uint32_t attr)
{
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (account->data_index[i] >= 0) {
            // 如果索引对应的数据的内容和要检测的一致，说明当前账户无权限访问该数据
            permission_data_t *data = permission_database_select_by_index(account->data_index[i]);
            if (data) {
                if (!strncmp(data->str, str, strlen(data->str)) && 
                ((data->attr & PERMISION_ATTR_TYPE_MASK) == (attr & PERMISION_ATTR_TYPE_MASK))) {
                    return 0;
                }
            }
        }
    }
    return -1;
}

int account_selfcheck_permission(char *str, uint32_t attr)
{
    if (!account_current)
        return -1;
    return account_check_permission(account_current, str, attr);
}

int sys_account_login(const char *name, char *password)
{
    if (!name)
        return -1;
    if (!password) {
        return account_logout(name);
    }
    return account_login(name, password);
}

int sys_account_register(const char *name, char *password)
{
    if (!name)
        return -1;
    if (!password) {
        return account_unregister(name);
    }
    return account_register(name, password, ACCOUNT_LEVEL_USER);
}

int account_manager_init()
{
    account_table = mem_alloc(sizeof(account_t) * ACCOUNT_NR); 
    if (!account_table) {
        panic("account: mem alloc for account table failed!\n");
    }
    int i;
    for (i = 0; i < ACCOUNT_NR; i++) {
        account_init(account_table + i);
    }
    account_current = NULL;
    account_read_config();
    permission_database_init();

    if (account_login("admin", "1234") < 0)
        panic("account: login admin failed!\n");
    
    account_register("jason", "1234", ACCOUNT_LEVEL_USER);

    if (account_login("jason", "1234") < 0)
        panic("account: login jason failed!\n");
    
    #if 0
    if (account_logout("jason") < 0)
        panic("account: logout jason failed!\n");
    
    if (account_login("admin", "1234") < 0)
        panic("account: login admin failed!\n");
    
    printk(KERN_NOTICE "unregister jason!\n");
    account_unregister("jason");
    #endif
    printk(KERN_INFO "login admin: OK!\n");
    return 0;
}
