#include <xbook/path.h>
#include <string.h>
#include <unistd.h>
#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <assert.h>
#include <xbook/fs.h>

fsal_path_t *fsal_path_table;
fsal_path_t *fsal_master_path;
DEFINE_SPIN_LOCK(fsal_path_table_lock);

int fsal_path_init()
{
    fsal_path_table = mem_alloc(FSAL_PATH_TABLE_SIZE);
    if (fsal_path_table == NULL) 
        return -1;
    memset(fsal_path_table, 0, FSAL_PATH_TABLE_SIZE);
    fsal_master_path = NULL;
    return 0;
}

int fsal_path_insert(void *path, char *alpath, fsal_t *fsal)
{
    if (path == NULL || alpath == NULL || fsal == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(alpath, 0);
    if (fpath)
        return -1;
    fpath = fsal_path_alloc();
    if (fpath == NULL)
        return -1;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    fpath->fsal = fsal;
    strcpy(fpath->path, path);
    fpath->path[strlen(path)] = '\0';
    strcpy(fpath->alpath, alpath);
    fpath->alpath[strlen(alpath)] = '\0';
    if (fsal_master_path == NULL)
        fsal_master_path = fpath;
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return 0;
}

fsal_path_t *fsal_path_alloc()
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    fsal_path_t *fpath;
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal == NULL) {
            spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
            return fpath;
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return NULL;
}

int fsal_path_remove(void *path)
{
    char *p = (char *) path;
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            if (!strcmp(p, fpath->path)) {
                fpath->fsal     = NULL;
                memset(fpath->path, 0, FASL_PATH_LEN);
                spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
                return 0;
            }
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    return -1;
}

/**
 * 查找路径对应的路径结构
 * @alpath: 抽象层路径
 * @inmaster: 是否在主路径中去查找
 * @成功返回路径结构地址，失败返回NULL
 */
fsal_path_t *fsal_path_find(void *alpath, int inmaster)
{
    char *p = strchr(alpath, '/');
    if (p == NULL) {
        return NULL;
    }
    if (*(p + 1) == 0 && inmaster)
        return fsal_master_path;
    char name[FASL_PATH_LEN] = {0, };
    p++;
    p = strchr(p, '/');
    if (p) {
        memcpy(name, alpath, p - (char *)alpath);
    } else {
        strcpy(name, alpath);
    }
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal) {
            if (!strcmp(name, fpath->alpath)) {
                spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
                return fpath;
            }
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
    if (inmaster)
        return fsal_master_path;
    return NULL;
}

/**
 * fsal_path_switch - 将抽象路径转换成具体路径
 * fatfs: /root/bin -> 1:/bin
 *      /root -> 1:
 */
int fsal_path_switch(fsal_path_t *fpath, char *new_path, char *old_path)
{
    if (!fpath || !new_path || !old_path)
        return -1;
    char *start = strchr(old_path, '/');
    if (start == NULL)
        return -1;
    strcpy(new_path, fpath->path);
    char *top_level = start;
    start++;
    start = strchr(start, '/');
    if (start == NULL) {
        if (fpath == fsal_master_path && *(top_level + 1) && strcmp(fpath->alpath, top_level)) {
            strcat(new_path, top_level);
        }
        return 0;
    } else {
        if (*(start + 1) == 0) {
            *start = 0;
            if (fpath == fsal_master_path && *(top_level + 1)  && strcmp(fpath->alpath, top_level)) {
                strcat(new_path, top_level);
            }
            return 0;
        }
        if (fsal_path_find(top_level, 0) == NULL) {
            start = top_level;
        }
    }
    strcat(new_path, start);
    return 0;
}

void fsal_path_print()
{
    printk("%s: fsal path info:\n", FS_MODEL_NAME);
    fsal_path_t *fpath;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_path_table_lock, irq_flags);
    int i;
    for (i = 0; i < FASL_PATH_NR; i++) {
        fpath = &fsal_path_table[i];
        if (fpath->fsal > 0) {
            printk("fasl alpath=%s path=%s fsal=%x\n", fpath->alpath, fpath->path, fpath->fsal);
        }
    }
    spin_unlock_irqrestore(&fsal_path_table_lock, irq_flags);
}


static char *parse_path_afterward(char *path, char *name)
{
    if (path[0] == '/') {
        /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
        while(*(++path) == '/');
    }
    while (*path != '/' && *path != 0) {
        *name++ = *path++;
    }
    if (path[0] == 0) {
        return NULL;
    }
    return path; 
}

void wash_path(char *old_path, char *new_path)
{
    ASSERT(old_path[0] == '/');
    char name[MAX_PATH] = {0};    
    char* sub_path = old_path;
    sub_path = parse_path_afterward(sub_path, name);
    if (name[0] == 0) {
        new_path[0] = '/';
        new_path[1] = 0;
        return;
    }
    new_path[0] = 0;	   // 避免传给new_path的缓冲区不干净
    strcat(new_path, "/");
    while (name[0]) {
        if (!strcmp("..", name)) {
	        char* slash_ptr =  strrchr(new_path, '/');
	        if (slash_ptr != new_path) {
	            *slash_ptr = 0;
	        } else {
	            *(slash_ptr + 1) = 0;
	        }
        } else if (strcmp(".", name)) {
	        if (strcmp(new_path, "/")) {
	            strcat(new_path, "/");
	        }
	        strcat(new_path, name);
        }
        memset(name, 0, MAX_PATH);
        if (sub_path) {
	        sub_path = parse_path_afterward(sub_path, name);
        }
    }
}

static void make_abs_path(const char *path, char *abspath)
{
    if (*path != '/') {
        if (!sys_getcwd(abspath, MAX_PATH)) {
            char *p = strchr(abspath, '/');
            if (p != NULL) {
                if (!((p[0] == '/') && (p[1] == 0))) {
                    strcat(abspath, "/");
                }
            }
        }
    }
    if (path[0] == '/' && path[1] == '\0') {
        abspath[0] = '/';
        abspath[1] = '\0';
    } else {
        strcat(abspath, path);
        if (strchr(abspath, '/') == NULL) {
            strcat(abspath, "/");
        }
    }
}

void build_path(const char *path, char *out_path)
{
    char abs_path[MAX_PATH] = {0};
    make_abs_path(path, abs_path);
    char *p = strchr(abs_path, '/');
    wash_path(p, out_path);
}