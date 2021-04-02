#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define MAP_FIXED       0x10       /* 映射固定位置 */
#define MAP_PRIVATE     0x00       /* 映射成私有，NOTE: 内核未实现该功能 */
#define MAP_SHARED      0x80       /* 映射成共享内存 */
#define MAP_REMAP       0x100      /* 强制重写映射 */

/* protect flags */
#define PROT_NONE        0x0       /* page can not be accessed */
#define PROT_READ        0x1       /* page can be read */
#define PROT_WRITE       0x2       /* page can be written */
#define PROT_EXEC        0x4       /* page can be executed */
#define PROT_KERN        0x8       /* page in kernel */
#define PROT_USER        0x10      /* page in user */
#define PROT_REMAP       0x20      /* page remap */

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_MMAN_H */
