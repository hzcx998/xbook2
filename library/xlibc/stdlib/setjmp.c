#include <setjmp.h>
/*
 * 演示版本，只为理解逻辑含义。
 * 假设它可以把栈的一个连续的空间复制到jmp_buf数据对象中，并且能保存足够数量的调用环境。
 * 它声明了很多register数据对象，希望借此可以强制报存所有具有调用上下文的重要的寄存器。
 * 它制造了一个调用dummy函数的假象，这样就可以骗过了一些优化器，这些优化器可能断定那些寄存器从没有被使用过。
 */
#include <string.h>

static void dummy(int a, int b, int c, int d, int e,
    int f, int g, int h, int i, int j)
{
}

static int getfp(void)
{
    static int arg;
    return ((int)(&arg + _JBFP));
}
int setjmp(jmp_buf env)
{
    register int a = 0, b = 0, c = 0, d = 0, e = 0;
    register int f = 0, g = 0, h = 0, i = 0, j = 0;

    if(a)
        dummy(a, b, c, d, e, f, g, h, i, j);
    env[1] = getfp();
    memcpy((char *)&env[2], (char *)env[1] + _JBOFF, _JBMOV);
    return 0;
}
