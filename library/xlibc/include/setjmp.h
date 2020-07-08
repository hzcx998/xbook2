#ifndef _XLIBC_SETJMP_H
#define _XLIBC_SETJMP_H
/*my_setjmp.h*/
/*
 * c程序不允许函数的嵌套定义。
 * 所以除了返回调用哪个函数的表达式外，c语言没有提供其他将控制权转移到一个函数之外的简单方法。
 * c语言使用库函数实现非本地控制转移。
 * jmp_buf类型，标量数据对象类型。
 * longjmp函数，用来实现非本地控制转移。
 * setjmp宏，把当前调用的上下文信息存储到一个jmp_buf类型的数据对象中，并在你想把控制权传递给相应的
 * longjmp调用的地方做标记。
 * 两个潜在危险:包含setjmp宏达表达式；在执行setjmp的函数中声明的动态存储空间。
 *
 * 一般电脑有一定数量的寄存器，在对表达式进行求值的时候，寄存器用来保存中间结果。
 * 但在计算一个非常复杂的表达式时，这些寄存器可能不够用，这时用户就会迫使代码生成器把中间结果存储在动态存储空间中。
 * c标准规定把包含setjmp的表达式作为子表达式使用。为了排除某些可能把中间结果存储在setjmp未知的动态存储空间中的表达式。
 * 所以可编写switch(setjmp(buf)...,if(2<setjmp(buf))...if(!setjmp(buf))...setjmp(buf).
 * 不能可靠把setjmp的值赋给其他的变量，如n=setjmp(buf).标准未定义。
 *
 * 宏setjmp的调用应该只出现在下面某个上下文环境中。
 * 1,一个选择或者循环语句的整个控制表达式。
 * 2,关系运算符或者等于运算符的其中一个操作数，另一个操数是一个整值常量表达式，它的结果表达式是一个选择或循环语言的整个控制表达式。
 * 3,一元操作符!的操作数，它的结果表达式是一个选择或循环语句的整个控制表达式。
 * 4,是一个表达式语句(可能强制转换为void类型)的整个表达式。
 * 函数longjmp使用相应的jmp_buf参数来恢复程序的相同调用中宏setjmp的最近一次调用保存的环境。
 * 因为它能绕过常规的函数调用和返回机制，所以函数longjmp可以在中断/信号和其他相关的函数的上下文环境中正确地执行。
 * 函数longjmp不能让宏setjmp返回0，如果val为0，这宏setjmp返回1.
 * 
 * 建议以以下标准方式使用:
 * 1,把每个对setjmp的调用分离到一个独立的小函数中。那样就会使出现动态声明的数据对象被longjmp调用恢复的情况减少到最少。
 * 2,在一个switch语句的控制表达式中调用setjmp。
 * 3,在switch语句的case 0(process)中调用的函数中执行所以实际的过程。
 * 4,通过执行longjmp(1)调用，在任意位置报告错误并且重新启动process。
 * 5,通过执行longjmp(2)调用，在任意位置报告错误并终止process。
 *
 * jmp_buf是一个数组类型。
 * 在信号处理器的内部调用longjmp，会出警告。
 *
 * 唯一可靠的实现方式是汇编来实现。
 *
 */
#ifndef MY_SETJMP_H_
#define MY_SETJMP_H_
/*
#ifndef _YVALS_H_
#include "yvals.h"
#endif
*/

#define _ILONG 1
#define _CSIGN 1
#define _MBMAX 2

#define _JBFP   1
#define _JBMOV  60
#define _JBOFF  4
#define _NSETJMP    17

#define _SIGABRT 6
#define _SIGMAX 32

typedef int jmp_buf[_NSETJMP];

int setjmp(jmp_buf env);
//void longjmp(jmpbuf, int);
void longjmp(jmp_buf env, int val);
#endif

#endif  /* _XLIBC_SETJMP_H */
