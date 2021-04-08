#include <stdio.h>

#define ECHO_VERSION "0.1"

int main(int argc, char *argv[])
{
    /* 选项解析 */
    if (argc == 1) {
        return 0;
    }
    /* 没有换行 */
    char no_newline = 0;
    /* 转义 */
    char trope = 0;
    
    /* 第一个参数位置 */
    int first_arg = 1;

    char *p = argv[first_arg];
    
    /* 有选项 */
    if (*p == '-') {
        p++;
        switch (*p)
        {
        case 'n':   /* 输出后不换行 */
            no_newline = 1;
            break;
        case 'e':   /* 转义字符 */
            trope = 1;
            break;
        case 'h':   /* 帮助 */
            printf("Usage: echo [option] [str1] [str2] [str3] ...\n");
            printf("Option:\n");
            printf("  -n    No new line after output. Example: echo -n hello!\n");
            printf("  -e    Escape a string. Example: echo -e this\\nis a string.\\n \n");
            printf("  -v    Print version. Example: echo -v \n");
            printf("  -h    Print usage help. Example: echo -h \n");
            printf("No option:\n");
            printf("  Print what you input. Example: echo a b 123 \n");
            
            return 0;
        case 'v':   /* 版本 */
            printf("version: %s\n", ECHO_VERSION);
            return 0;
        default:
            fprintf(stderr,"echo: unknown option!\n");
            return -1;
        }
        /* 指向下一个参数 */
        first_arg++;
    }
    int i;
    /* 输出参数 */
    for (i = first_arg; i < argc; i++) {
        if (trope) {
            /* 有转义就需要把转义字符输出成转义符号 */
            p = argv[i];
            while (*p) {    
                /* 如果是'\'字符，后面是转义符号 */
                if (*p == '\\') {
                    switch (*(p + 1)) {
                    case 'b':   /* '\b' */
                        p += 2;
                        putchar('\b');
                        break;
                    case 'n':   /* '\n' */
                        p += 2;
                        putchar('\n');                    
                        break;
                    case 'c':   /* '\c' */
                        p += 2;
                        /* 不换行 */
                        no_newline = 1;
                        break;
                    case 't':   /* '\t' */
                        p += 2;
                        putchar('\t');                    
                        break;
                    case '\\':   /* '\\' */
                        p += 2;
                        putchar('\\');
                        break;                
                    default:
                        /* 不是可识别的转义字符，直接输出 */
                        putchar(*p++);
                        break;
                    }
                } else {
                    /* 不是转义字符开头，就直接显示 */
                    putchar(*p++);
                }   
            }
        } else {    /* 没有转义就直接输出 */
            printf("%s", argv[i]);
        }
        /* 不是最后才输出一个空格 */
        if (i < argc - 1)
            putchar(' ');
            
    }

    /* 换行 */
    if (!no_newline) {
        putchar('\n');
    }
    
	return 0;
}
