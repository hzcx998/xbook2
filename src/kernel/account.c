#include <xbook/account.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sconf.h>
#include <xbook/path.h>
#include <xbook/safety.h>
#include <xbook/fs.h>

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
    dbgprint("Account List:\n");
    int i;
    for (i = 0; i < ACCOUNT_NR; i++) {
        account_t *account = account_table + i;
        if (account->flags & ACCOUNT_FLAG_USED)
            dbgprint("Account:%s pwd:%s attr:%x\n", account->name, account->password, account->flags);
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

#define IS_VALID_ACCOUNT_WORD(w) (('a' <= w && w <= 'z') \
        || ('Z' <= w && w <= 'Z') \
        || ('0' <= w && w <= '9') \
        || (w == '_'))

#define IS_VALID_PASSWORD_WORD(w) (('a' <= w && w <= 'z') \
        || ('Z' <= w && w <= 'Z') \
        || ('0' <= w && w <= '9') \
        || (w == '_') \
        || (w == '$') \
        || (w == '@') \
        || (w == '!') \
        || (w == '%') \
        || (w == '*') \
        || (w == '#') \
        || (w == '~') \
        || (w == '?'))

int account_name_check(const char *name)
{
    char *p = (char *) name;
    while (*p) {
        if (!IS_VALID_ACCOUNT_WORD(*p)) {
            return -1;
        }
        p++;
    }
    return 0;
}

int account_password_check(const char *password)
{
    char *p = (char *)password;
    while (*p) {
        if (!IS_VALID_PASSWORD_WORD(*p)) {
            return -1;
        }
        p++;
    }
    return 0;
}

int account_scan_line(char *line)
{
    //dbgprint("account: line: %s\n", line);
    char *q = line;
    uint32_t attr = 0;
    /* 解析权限 */
    char attr_buf[32] = {0};
    q = sconf_read(q, attr_buf, 32);
    if (!q) {
        errprint("account: get attr %s failed\n", attr_buf);
        return -1;
    }
    sconf_trim(attr_buf);
    if (attr_buf[0] == 'S') {
        attr |= ACCOUNT_LEVEL_ROOT;
    } else if (attr_buf[0] == 'U') {
        attr |= ACCOUNT_LEVEL_ROOT;
    }
    /* 解析账户名 */
    char name_buf[32] = {0};
    q = sconf_read(q, name_buf, 32);
    if (!q) {
        errprint("account: get name: %s failed\n", name_buf);
        return -1;
    }
    sconf_trim(name_buf);
    /* 解析密码 */
    char pwd_buf[32] = {0};
    q = sconf_read(q, pwd_buf, 32);
    if (!q) {
        errprint("account: get password: %s failed\n", pwd_buf);
        return -1;
    }
    sconf_trim(pwd_buf);
    return account_push(name_buf, pwd_buf, attr);
}

static int account_load_from_file(char *filename)
{
    int fd = kfile_open(filename, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    int fsize = kfile_lseek(fd, 0, SEEK_END);
    if (fsize <= 0) {
        kfile_close(fd);
        return -1;
    }
    char *fbuf = mem_alloc(fsize);
    if (!fbuf) {
        kfile_close(fd);
        return -1;
    }
    memset(fbuf, 0, fsize);
    kfile_lseek(fd, 0, SEEK_SET);
    if (kfile_read(fd, fbuf, fsize) != fsize) {
        mem_free(fbuf);
        kfile_close(fd);
        return -1;
    }
    kfile_close(fd);

    char *p = fbuf;
    while (1) {
        char line[128] = {0};
        p = sconf_readline(p, line, 128);
        if (!p)
            break;
        if (account_scan_line(line) < 0)
            errprint("account: push account failed: %s\n", line);
    }
    mem_free(fbuf);
    return 0;
}

/* 读取配置，存放到账户库 */
int account_read_config()
{
    char buf[32] = {0};
    strcat(buf, ACCOUNT_DIR_PATH);
    strcat(buf, "/");
    strcat(buf, ACCOUNT_FILE_NAME);
    /* 如果文件不存在，则需要创建文件，并创建根账户 */
    int file_not_exist = 0;
    if (kfile_access(buf, F_OK) < 0) {
        int fd = kfile_open(buf, O_CREAT | O_RDWR);
        if (fd < 0) {
            errprint("create account file %s failed!\n", buf);
            return -1;
        }
        kfile_close(fd);
        file_not_exist = 1;
    }

    if (file_not_exist) {
        /* 创建唯一的账户 */
        if (account_push(ROOT_ACCOUNT_NAME, ROOT_ACCOUNT_PASSWORD, ACCOUNT_LEVEL_ROOT) < 0) {
            errprint("add a new account %s to table failed!\n", ROOT_ACCOUNT_NAME);
            return -1;
        }
        if (account_sync() < 0) {
            errprint("sync account failed!\n");
            return -1;
        }
    } else { /* 账户文件已经存在了，直接读取文件即可。 */ 
        if (account_load_from_file(buf) < 0) {
            errprint("load account from file failed!\n");
            return -1;
        }
    }
    return 0;
}

static void account_build_file_buf(account_t *account, char *buf)
{
    int level = (account->flags & ACCOUNT_LEVEL_MASK);
    char *level_str = (level == ACCOUNT_LEVEL_ROOT) ? "S":"U";
    sconf_write(buf, level_str);
    sconf_write(buf, account->name);
    sconf_write(buf, account->password);
    sconf_writeline(buf);
}

int account_sync_data()
{
    mutex_lock(&account_mutex_lock);

    char buf[32] = {0};
    strcat(buf, ACCOUNT_DIR_PATH);
    strcat(buf, "/");
    strcat(buf, ACCOUNT_FILE_NAME);
    int fd = kfile_open(buf, O_RDWR | O_TRUNC);
    if (fd < 0) {
        errprint("open account file %s failed!\n", buf);
        mutex_unlock(&account_mutex_lock);
        return -1;
    }
    int i; for (i = 0; i < ACCOUNT_NR; i++) {
        account_t *account = account_table + i;
        if ((account->flags & ACCOUNT_FLAG_USED)) {    
            /* U/S,name,password\n */
            char account_buf[ACCOUNT_NAME_LEN + ACCOUNT_PASSWORD_LEN + 8] = {0};
            account_build_file_buf(account, account_buf);
            
            // dbgprint("account buf:%s", account_buf);
            kfile_write(fd, account_buf, strlen(account_buf));
        }
    }
    kfile_close(fd);
    mutex_unlock(&account_mutex_lock);
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
    
    if (account_name_check(name) < 0) {
        return -EINVAL;
    }
    if (account_password_check(password) < 0) {
        return -EINVAL;
    }

    account_t *account = account_find_by_name(name);
    if (!account) {
        errprint("account %s not exist!\n", name);
        return -1;
    }
    if (strcmp(account->password, password) != 0) {
        errprint("password %s not match!\n", password);
        return -1;
    }
    /* 之前登陆过，但是没有退出登录，强制退出其权限  */
    if (account->flags & ACCOUNT_FLAG_LOGINED) {
        account_debind_perm(account);
        account->flags &= ~ACCOUNT_FLAG_LOGINED;
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
    if (account_name_check(name) < 0) {
        return -EINVAL;
    }

    account_t *account = account_find_by_name(name);
    if (!account) {
        errprint("account %s not exist!\n", name);
        return -1;
    }
    if (!account_current) {
        errprint("Current account is null, can't unregister!\n");
        return -1;
    }
    if (account != account_current) {
        errprint("account %s not current account!\n", name);        
        return -1;   
    }
    mutex_lock(&account_mutex_lock);
    if (account_current != account) {
        errprint("account %s not current!\n", name);
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
        return -EINVAL;
    if (account_name_check(name) < 0) {
        return -EINVAL;
    }
    if (account_password_check(password) < 0) {
        return -EINVAL;
    }

    if (account_current) {
        if ((account_current->flags & ACCOUNT_LEVEL_MASK) != ACCOUNT_LEVEL_ROOT) {
            errprint("account %s no permission to register account %s!\n", account_current->name, name);
            return -EPERM;
        }
    }
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (account) {
        errprint("account %s had existed!\n", name);
        return -1;
    }
    if (account_push(name, password, flags) < 0) {
        errprint("account %s push into account table failed!\n", name);
        return -1;
    }
    /* 创建一个账户对应的主页路径权限 */
    char str[42] = {0};
    strcpy(str, HOME_DIR_PATH);
    strcat(str, "/");
    strcat(str, name);
    int index = permission_database_insert(PERMISION_ATTR_HOME | PERMISION_ATTR_RDWR | PERMISION_ATTR_FILE, str);
    if (index < 0) {
        errprint("account %s insert home failed!\n", name);
        account_pop(name);
        return -1;
    }

    if (account_sync() < 0) {
        errprint("account sync failed!\n", name);
        permission_database_delete(index);
        account_pop(name);
        return -1;
    }

    if (kfile_mkdir(str, 0) < 0) {
        warnprint("create account %s home path %s failed or existed!\n", name, str);
    }
    return 0;
}

int account_unregister(const char *name)
{
    if (!name)
        return -1;
    if (account_name_check(name) < 0) {
        return -EINVAL;
    }
    if (account_current) {
        if ((account_current->flags & ACCOUNT_LEVEL_MASK) != ACCOUNT_LEVEL_ROOT) {
            errprint("account %s no permission to unregister account %s!\n", account_current->name, name);
            return -EPERM;
        }
    }
    /* TODO:验证是否是合格的用户名和密码 */
    
    /* 查看账户是否已经存在了，不存在才创建 */
    account_t *account = account_find_by_name(name);
    if (!account)
        return -1;

    if (!account_current) {
        errprint("Current account is null, can't unregister!\n");
        return -1;
    }

    if (account == account_current) {
        errprint("Account %s using, can't unregister!\n", account_current->name);
        return -1;
    }
    mutex_lock(&account_mutex_lock);
    /* 必须要更高的权限才能进行注销 */
    int cur_level = account_current->flags & ACCOUNT_LEVEL_MASK;
    int acc_level = account->flags & ACCOUNT_LEVEL_MASK;
    if (cur_level >= acc_level) {
        errprint("Account %s no permission to unregister account %s!\n", 
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
        errprint("Account %s home data not found failed!\n", name);
        return -1;
    }
    permission_data_t data_backup = *data_ptr;
    if (permission_database_delete_by_data(str) < 0) {
        errprint("Account %s delete data from database failed!\n", name);
        return -1;
    }

    account_t account_backup = *account;    // 备份账户
    if (account_pop(name) < 0) {
        errprint("Account %s pop from account table failed!\n", name);
        permission_database_insert(data_backup.attr, data_backup.str); // 恢复数据
        return -1;
    }
    
    if (account_sync() < 0) {
        emeprint("Sync account when unregister failed!\n");
        account_push(account_backup.name, account_backup.password,
                account_backup.flags & ACCOUNT_LEVEL_MASK);   //恢复账户
        permission_database_insert(data_backup.attr, data_backup.str);
        return -1;
    }
    
    if (kfile_rmdir(str) < 0) {
        warnprint("delect account %s home path %s failed or not existed!\n", name, str);
    }
    return 0;
}

int account_add_index(account_t *account, uint32_t index)
{
    unsigned long flags;
    spin_lock_irqsave(&account->lock, flags);
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (account->data_index[i] == -1) {
            account->data_index[i] = index;
            account->index_len++;
            break;
        }
    }
    spin_unlock_irqrestore(&account->lock, flags);
    if (i >= PERMISION_DATABASE_LEN)
        return -1;
    return 0;
}

int account_del_index(account_t *account, uint32_t index)
{
    if (index >= PERMISION_DATABASE_LEN)
        return -1;
    unsigned long flags;
    spin_lock_irqsave(&account->lock, flags);
    if (account->data_index[index] >= 0) {
        account->data_index[index] = -1;
        account->index_len--;
    }
    spin_unlock_irqrestore(&account->lock, flags);
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
    // root no perm limit
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
    dbgprint("Account datasets:\n");
    int i; for (i = 0; i < PERMISION_DATABASE_LEN; i++) {
        if (account->data_index[i] >= 0) {
            dbgprint("solt %d :index %d\n", i, account->data_index[i]);
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
    #ifdef CONFIG_ACCOUNT_CHECK
    if (!account_current)
        return -1;
    return account_check_permission(account_current, str, attr);
    #else
    return 0;
    #endif
}

int sys_account_login(const char *name, char *password)
{
    if (!name)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *)name, ACCOUNT_NAME_LEN) < 0) {
        return -EINVAL;
    }
    if (!password) {
        return account_logout(name);
    }
    if (mem_copy_from_user(NULL, (void *)password, ACCOUNT_PASSWORD_LEN) < 0) {
        return -EINVAL;
    }
    return account_login(name, password);
}

int sys_account_register(const char *name, char *password)
{
    if (!name)
        return -EINVAL;
    if (mem_copy_from_user(NULL, (void *)name, ACCOUNT_NAME_LEN) < 0)
        return -EINVAL;
    if (!password) {
        return account_unregister(name);
    }
    if (mem_copy_from_user(NULL, (void *)password, ACCOUNT_PASSWORD_LEN) < 0)
        return -EINVAL;
    return account_register(name, password, ACCOUNT_LEVEL_USER);
}

int sys_account_name(char *buf, size_t buflen)
{
    if (!account_current)
        return -EFAULT;
    return mem_copy_to_user((void *)buf, account_current->name, min(buflen, strlen(account_current->name)));
}

int sys_account_verify(char *password)
{
    if (!account_current)
        return -EFAULT;
    if (mem_copy_from_user(NULL, password, ACCOUNT_PASSWORD_LEN) < 0)
        return -EFAULT;
    if (account_password_check(password) < 0) {
        return -EINVAL;
    }
    mutex_lock(&account_mutex_lock);
    if (strcmp(account_current->password, password) != 0) {
        mutex_unlock(&account_mutex_lock);
        return -1;
    }
    mutex_unlock(&account_mutex_lock);
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
    
    permission_database_init();
    if (account_read_config() < 0)
        panic("account manager read config failed!\n");

    /* default login root */
    
    if (account_login(ROOT_ACCOUNT_NAME, ROOT_ACCOUNT_PASSWORD) < 0)
        panic("account: login root failed!\n");
    
    keprint(PRINT_INFO "account init: done.\n");

    return 0;
}
