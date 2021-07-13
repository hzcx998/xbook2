#ifndef _SH_H
#define _SH_H

#include <stddef.h>

/* ====SHELL CONFIG START ====*/
// #define _HAS_EXECPTION
#define _HAS_ENVIRON
// #define _HAS_FPRINTF
// #define _HAS_ACCOUNTNAME
// #define _HAS_STRCAT
// #define _HAS_IOCTL

/* ====SHELL CONFIG END ====*/

#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16

typedef int (*cmd_func_t) (int argc, char **argv);

/* buildin cmd struct */
struct buildin_cmd {
    char *name;
    cmd_func_t cmd_func;
};

//func

void update_cwdcache();
void print_prompt();
int cmd_parse(char * cmd_str, char **argv, char token);
void readline( char *buf, size_t count);
int execute_cmd(int argc, char **argv);
#ifdef _HAS_EXECPTION
void sh_exit_handler(unsigned int code);
#endif
void print_logo();

#endif /* _SH_H */