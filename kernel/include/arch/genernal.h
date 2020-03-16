#ifndef _ARCH_GENERNAL_H
#define _ARCH_GENERNAL_H

#include <xbook/config.h>

#ifdef CONFIG_ARCH_X86
#include "../../arch/x86/core/arch.h"
#endif

#ifdef CONFIG_ARCH_X64
#include "../../arch/x64/core/arch.h"
#endif

#endif  /* _ARCH_GENERNAL_H */
