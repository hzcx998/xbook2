#include "test.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

typedef int (*main_t) (int , char **);
typedef struct {
    char *name;
    main_t func;
} testfunc_t;

testfunc_t test_table[] = {
    {"file", test_file},
    {"time", test_time},
    {"misc", test_misc},
    {"pid", test_pid},
    {"uid", test_uid},
    {"reboot", test_reboot},
    {"xattr", test_xattr},
    {"kill", test_kill},
    {"sched", test_sched},
    {"signal", oscamp_signal},
};

int main(int argc, char *argv[])
{
    printf("testing start...\n");
    
    int retval = -1;
    if (argc == 1) {
        retval = test_table[0].func(argc, argv);  
        printf("\ntest %s demo done.\n", test_table[0].name);
    } else if (argc == 2) {
        char *p = argv[1];
        int i;
        for (i = 0; i < ARRAY_SIZE(test_table); i++)
            if (!strcmp(p, test_table[i].name)) {
                retval = test_table[i].func(argc, argv);
                printf("\ntest %s demo done.\n", test_table[i].name);
                break;
            }
        if (i >= ARRAY_SIZE(test_table))
            printf("test %s demo not found!\n", p);
    }    
    return retval;
}
