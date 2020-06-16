#ifndef __SGI_FONT_H__ /* font */
#define __SGI_FONT_H__

#include <sys/list.h>

/*
字体管理：
注册字体，注销字体。（添加到字体管理系统，从字体管理系统中删除）
加载字体，设置字体。（获取一个字体，设置字体到窗口环境中）
打开字体，关闭字体。（从文件系统加载到内存，从内存中释放）
*/

#define SGI_FONT_DEFAILT_NAME   "standard-8*16"
#define SGI_FONT_DEFAILT_COPYRIGHT   "Linux"



#define SGI_FONT_NAME_LEN 24
#define SGI_FONT_COPYRIGHT_NAME_LEN 24

typedef struct _SGI_FontInfo {
    list_t  list;                                   /* 字体链表 */
	char    name[SGI_FONT_NAME_LEN];	            /* 字体名字 */
	char    copyright[SGI_FONT_COPYRIGHT_NAME_LEN];	/* 字体版权 */
	unsigned char   *addr;          /* 字体数据地址 */
	unsigned int    width;          /* 单字宽度 */
    unsigned int    height;		    /* 单字宽度 */
} SGI_FontInfo;

#endif  /* __SGI_FONT_H__ */