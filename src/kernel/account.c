#include <xbook/account.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>

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

权限索引文件：
dbindex
admin,0,1,2,5,78
jason,0,1,2,5,78
*/

account_t *account_table;
account_t *account_current; /* 当前登录的账户 */
DEFINE_MUTEX_LOCK(account_mutex_lock);

static void account_init(account_t *account)
{
    memset(account->name, 0, ACCOUNT_NAME_LEN);
    memset(account->password, 0, ACCOUNT_PASSWORD_LEN);
    account->flags = 0;
    memset(account->data_index, 0, PERMISION_DATABASE_LEN);
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

static void account_dump()
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

/* 加载一个账户到账户库 */
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

/* 把一个账户从账户库存到配置文件 */
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

int account_sync()
{
    /* TODO: 将账户信息同步到文件中 */

    return 0;
}

int account_login(const char *name, char *password)
{
    if (!name || !password)
        return -1;
    account_t *account = account_find_by_name(name);
    if (!account) {
        pr_dbg("account %s not exist!\n", name);
        return -1;
    }
        
    if (strcmp(account->password, password) != 0) {
        pr_dbg("password %s not match!\n", name);
        return -1;
    }
    mutex_lock(&account_mutex_lock);
    account_current = account;
    mutex_unlock(&account_mutex_lock);
    return 0;   /* login success */
}

int account_logout(const char *name)
{
    if (!name)
        return -1;
    account_t *account = account_find_by_name(name);
    if (!account) {
        pr_dbg("account %s not exist!\n", name);
        return -1;
    }
    mutex_lock(&account_mutex_lock);
    if (account_current != account) {
        pr_dbg("account %s not current!\n", name);
        mutex_unlock(&account_mutex_lock);
        return -1;
    }        
    account_current = NULL;
    mutex_unlock(&account_mutex_lock);
    return 0;   /* logout success */
}

/* 注册账号成功后，会保存到配置文件中 */
int account_register(const char *name, char *password, uint32_t flags)
{
    if (!name || !password || !account_current)
        return -1;
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (account) {
        pr_dbg("account %s had existed!\n", name);
        return -1;
    }
        
    /* 先加载到内存账户中 */
    if (account_push(name, password, flags) < 0) {
        pr_dbg("account %s push into account table failed!\n", name);
        return -1;
    }
    
    /* 将账户信息同步到文件系统中 */
    if (account_sync() < 0) {
        pr_dbg("account sync failed!\n", name);
        account_pop(name);
        return -1;
    }
    return 0;
}

int account_unregister(const char *name)
{
    if (!name || !account_current)
        return -1;
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (!account)
        return -1;

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
    if (account_pop(name) < 0) {
        pr_err("Account %s pop from account table failed!\n", name);
        return -1;
    }
    if (account_sync() < 0) {
        pr_emerg("Sync account when unregister failed!\n");
        return -1;
    }
    return 0;
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
    // 加载账户文件，读取管理员账户信息
    account_read_config();

    // 登录账户
    if (account_login("admin", "1234") < 0)
        panic("account: login admin failed!\n");
    printk(KERN_INFO "current account: %x\n", account_current);
    
    if (account_register("jason", "1234", ACCOUNT_LEVEL_USER) < 0) {
        pr_dbg("account register failed!\n");
    }
    account_dump();
    if (account_unregister("jason") < 0) {
        pr_dbg("account unregister failed!\n");
    }
    account_dump();

    if (account_logout("admin") < 0)
        panic("account: logout admin failed!\n");
    printk(KERN_INFO "current account: %x\n", account_current);

    printk(KERN_INFO "login admin: OK!\n");
    return 0;
}
