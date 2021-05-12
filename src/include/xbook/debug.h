#ifndef _XBOOK_DEBUG_H
#define _XBOOK_DEBUG_H

/* has lock for print?  */
#define PRINT_LOCK      1

#define PRINT_EMERG      "<0>"      /* system is unuseable */
#define PRINT_ERR        "<1>"      /* error condition */
#define PRINT_WARING     "<2>"      /* waring condition */
#define PRINT_NOTICE     "<3>"      /* normal significant */
#define PRINT_INFO       "<4>"      /* infomational */
#define PRINT_DEBUG      "<5>"      /* debug message */

#define DEFAULT_LOG_MIN   0
#define DEFAULT_LOG_MAX   5

#define DEFAULT_LOG_LEVEL  5

extern int print_gui_console;

int keprint(const char *fmt, ...);
#define print_fmt(fmt) fmt
#define emeprint(fmt, ...) \
    keprint(PRINT_EMERG print_fmt(fmt), ##__VA_ARGS__)
#define errprint(fmt, ...) \
    keprint(PRINT_ERR print_fmt(fmt), ##__VA_ARGS__)
#define warnprint(fmt, ...) \
    keprint(PRINT_WARING print_fmt(fmt), ##__VA_ARGS__)
#define noteprint(fmt, ...) \
    keprint(PRINT_NOTICE print_fmt(fmt), ##__VA_ARGS__)
#define infoprint(fmt, ...) \
    keprint(PRINT_INFO print_fmt(fmt), ##__VA_ARGS__)
#define dbgprint(fmt, ...) \
    keprint(PRINT_DEBUG fmt, ##__VA_ARGS__)
#define logprint(fmt, ...) \
        keprint("file:%s line:%d: " PRINT_DEBUG print_fmt(fmt), __FILE__, __LINE__, ##__VA_ARGS__)

#define endl "\n"

/* with new line */
#define keprintln(fmt, ...) keprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define emeprintln(fmt, ...) emeprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define errprintln(fmt, ...) errprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define warnprintln(fmt, ...) warnprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define noteprintln(fmt, ...) noteprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define infoprintln(fmt, ...) infoprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define dbgprintln(fmt, ...) dbgprint(print_fmt(fmt) endl, ##__VA_ARGS__)
#define logprintln(fmt, ...) logprint(print_fmt(fmt) endl, ##__VA_ARGS__)

void spin(char * func_name);
void panic(const char *fmt, ...);

void log_dump_value(unsigned long val);
void log_dump_buffer(void *buffer, unsigned long len, char factor);

#endif   /*_XBOOK_DEBUG_H*/
