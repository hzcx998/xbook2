#ifndef _XBOOK_ACCOUNT_H
#define _XBOOK_ACCOUNT_H

#include <stdint.h>
#include <xbook/spinlock.h>

/* 最大允许的账户数量 */
#define ACCOUNT_NR  4

#define ACCOUNT_NAME_LEN    32
#define ACCOUNT_PASSWORD_LEN    128

/* 建立权限数据库
当用户打开某个资源时，需要在数据库中比对，看当前登录的用户是否有权限来访问该资源。

可以访问，但需要赋权：
访问、申请权限（获取口令）、访问时携带口令

不可以访问，需要管理员才能访问。

用户访问资源，无权限
-》申请资源访问
-》询问是否允许、输入访问口令、输入管理员密码
-》允许则可以访问

管理员有权限访问
-》访问
-》通过

不同的账户有不同的权限，低权限账户的权限数据库可以被高权限的数据库修改
创建的时候绑定一个数据库索引。
根据数据库索引，可以获取权限，然后再访问资源。

权限数据库：
权限数据：数据类型，属性，值


属性：访问（打开，读取，检测存在，复制），修改（写入，删除）
路径：属性：/home/jason
设备：属性：disk0
FIFO: abc

*/

#define ADMIN_ACCOUNT_NAME      "admin"
#define ADMIN_ACCOUNT_PASSWORD  "1234"


#define ACCOUNT_LEVEL_ROOT  1
#define ACCOUNT_LEVEL_ADMIN 2
#define ACCOUNT_LEVEL_USER  3
#define ACCOUNT_LEVEL_MASK  0xf

#define ACCOUNT_FLAG_USED   (1 << 31)


#define PERMISION_DATABASE_LEN   32

typedef struct {
    uint32_t attr;      /* 数据的类型，访问权限等 */
    uint32_t datalen;    /* 总是为数据长度+1 */
    uint8_t *data;         /* 通过内存分配器分配的数据 */
} permission_data_t;

typedef struct {
    spinlock_t lock;    /* 用于维护数据库操作的锁 */
    uint32_t length;    /* 当前存放的数据数量 */
    permission_data_t datasets[PERMISION_DATABASE_LEN];
} permission_database_t;

int permission_database_init();
int permission_database_insert(uint32_t attr, uint8_t *data);
int permission_database_delete(uint32_t index);
int permission_database_update(uint32_t index, uint32_t attr, uint8_t *data);
int permission_database_select(uint32_t index, permission_data_t *data);

typedef struct {
    char name[ACCOUNT_NAME_LEN];    
    char password[ACCOUNT_PASSWORD_LEN];
    uint32_t flags;
    uint32_t index_len;                          /* 索引长度，表明有多少个索引 */
    spinlock_t lock;                            /* 用于维护账户操作的锁 */
    uint32_t data_index[PERMISION_DATABASE_LEN];   /* 数据库索引，访问权限数据库 */
} account_t;

int account_add_index(account_t *account, uint32_t index);
int account_del_index(account_t *account, uint32_t index);

int account_check_permission(account_t *account, uint8_t *data);
int account_selfcheck_permission(uint8_t *data);

/* 初始化超级管理员账户，加载账户管理配置文件，加载超级管理员账户与密码，加载已经注册的用户和密码。*/
int account_manager_init();
int account_login(const char *name, char *password);
int account_logout(const char *name);
int account_register(const char *name, char *password, uint32_t flags);
int account_unregister(const char *name);

#endif   /* _XBOOK_ACCOUNT_H */
