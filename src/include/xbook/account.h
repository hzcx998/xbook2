#ifndef _XBOOK_ACCOUNT_H
#define _XBOOK_ACCOUNT_H

#include <stdint.h>
#include <stddef.h>
#include <xbook/spinlock.h>
#include <xbook/mutexlock.h>

// #define CONFIG_ACCOUNT_CHECK

/* 最大允许的账户数量 */
#define ACCOUNT_NR  8

#define ACCOUNT_NAME_LEN    32
#define ACCOUNT_PASSWORD_LEN    128

#define ROOT_ACCOUNT_NAME      "root"
#define ROOT_ACCOUNT_PASSWORD  "1234"

#define ACCOUNT_FILE_NAME       "acct"
#define PERMISSION_FILE_NAME    "perm"

#define ACCOUNT_LEVEL_ROOT  1
#define ACCOUNT_LEVEL_USER  2
#define ACCOUNT_LEVEL_MASK  0xff

#define ACCOUNT_FLAG_USED       (1 << 31)
#define ACCOUNT_FLAG_LOGINED    (1 << 16)

#define PERMISION_STR_LEN   32
#define PERMISION_DATABASE_LEN   32

/* 0-9 */
#define PERMISION_ATTR_DEVICE       (1 << 0)
#define PERMISION_ATTR_FILE         (1 << 1)    // 有子路径概念
#define PERMISION_ATTR_FIFO         (1 << 2)

#define PERMISION_ATTR_TYPE_MASK    0xffff
/* rw-h */
#define PERMISION_ATTR_READ         (1 << 16)
#define PERMISION_ATTR_WRITE        (1 << 17)
#define PERMISION_ATTR_RDWR         (PERMISION_ATTR_READ | PERMISION_ATTR_WRITE)
#define PERMISION_ATTR_EXEC         (1 << 18)
#define PERMISION_ATTR_HOME         (1 << 19) // 用户主页

typedef struct {
    uint32_t attr;      /* 0~15: 类型。16~31：属性 */
    char str[PERMISION_STR_LEN];
} permission_data_t;

typedef struct {
    mutexlock_t lock;    /* 用于维护数据库操作的锁 */
    uint32_t length;    /* 当前存放的数据数量 */
    permission_data_t datasets[PERMISION_DATABASE_LEN];
} permission_database_t;

int permission_database_init();
void permission_database_dump();
int permission_database_insert(uint32_t attr, char *data);
int permission_database_delete(uint32_t index);
int permission_database_delete_by_data(char *str);
permission_data_t *permission_database_select_by_index(uint32_t index);

int permission_database_sync();
int permission_database_load();
void permission_database_foreach(void (*callback)(void *, void *) , void *arg);
permission_data_t *permission_database_select(char *str);

typedef struct {
    char name[ACCOUNT_NAME_LEN];    
    char password[ACCOUNT_PASSWORD_LEN];
    /* 0~7: 账户等级。16~30：账户状态。31：账户表使用情况 */
    uint32_t flags;
    uint32_t index_len;                             /* 索引长度，表明有多少个索引 */
    spinlock_t lock;                                /* 用于维护账户操作的锁 */
    int32_t data_index[PERMISION_DATABASE_LEN];     /* 数据库索引，访问权限数据库 */
} account_t;

int account_add_index(account_t *account, uint32_t index);
int account_del_index(account_t *account, uint32_t index);

int account_check_permission(account_t *account, char *str, uint32_t attr);
int account_selfcheck_permission(char *str, uint32_t attr);

int account_manager_init();
int account_login(const char *name, char *password);
int account_logout(const char *name);
int account_register(const char *name, char *password, uint32_t flags);
int account_unregister(const char *name);

int account_sync();
int account_bind_perm(account_t *account);
int account_debind_perm(account_t *account);

int sys_account_login(const char *name, char *password);
int sys_account_register(const char *name, char *password);
int sys_account_name(char *buf, size_t buflen);
int sys_account_verify(char *password);

#endif   /* _XBOOK_ACCOUNT_H */
