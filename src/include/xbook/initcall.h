

#ifndef _XBOOK_INITCALL_H
#define _XBOOK_INITCALL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*initcall_t)(void);
typedef void (*exitcall_t)(void);

#define __init __attribute__ ((__section__ (".init.text")))
#define __exit __attribute__ ((__section__ (".exit.text")))

#define __define_initcall(level, fn, id) \
	static const initcall_t __initcall_##fn##id \
	__attribute__((__used__, __section__(".initcall_" level ".text"))) = fn

#define __define_exitcall(level, fn, id) \
	static const exitcall_t __exitcall_##fn##id \
	__attribute__((__used__, __section__(".exitcall_" level ".text"))) = fn

#define driver_initcall(fn)		__define_initcall("0", fn, 0)
#define filter_initcall(fn)		__define_initcall("1", fn, 1)

#define driver_exitcall(fn)		__define_exitcall("0", fn, 0)
#define filter_exitcall(fn)		__define_exitcall("1", fn, 1)

void do_initcalls(void);
void do_exitcalls(void);

#ifdef __cplusplus
}
#endif

#endif /* _XBOOK_INITCALL_H */
