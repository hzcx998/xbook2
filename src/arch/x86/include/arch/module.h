#ifndef _X86_MODULE_H
#define _X86_MODULE_H

#include <const.h>

#define MODULE_INFO_ADDR 0x3F1000

#define MAX_MODULES_NUM 1
#define MAX_MODULES_SIZE (1 * MB)

enum {
	// Unknown type
	MODULE_UNKNOWN = 0,
	// Initrd image type
	MODULE_INITRD = 1,
};

struct modules_info_block {
	unsigned int modules_num;
	unsigned int modules_size;
	struct {
		unsigned int type;
		unsigned int size;
		unsigned int start;
		unsigned int end;
	} modules[MAX_MODULES_NUM];
} __attribute__ ((packed));

static inline void module_info_init()
{
   struct modules_info_block *modules_info = (struct modules_info_block *)MODULE_INFO_ADDR;
   modules_info->modules_num = 0;
}

#endif /* _X86_MODULE_H */