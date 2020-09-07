#ifndef _X86_CONFIG_H
#define _X86_CONFIG_H

/*
debug methods：
1 -> in text mode, in os.
2 -> serial send to host machine console
*/
#define CONFIG_DEBUG_METHOD 2

/* 把控制台的信息同时输出到串口，用于调试 */
#define CONFIG_CONS_TO_SERIAL 1

// #define CONFIG_PRINT_CONSOLE

#endif  /* _X86_CONFIG_H */