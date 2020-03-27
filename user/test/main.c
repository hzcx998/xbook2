#include <usrmsg.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

int main(int argc, char *argv[])
{
    int i;
    char *p;
    for (i = 0; i < argc; i++) {
        p = (char *)argv[i];
        while (*p++);
    }
    //func(1000);
    
    define_umsg(msg);
    umsg_set_type(msg, UMSG_OPEN);
    umsg_set_arg0(msg, "con0");
    umsg(msg);

    umsg_set_type(msg, UMSG_WRITE);
    char *s = "hello, xbook!\n";
    umsg_set_arg0(msg, "con0");
    umsg_set_arg1(msg, 0);
    umsg_set_arg2(msg, s);
    umsg_set_arg3(msg, 15);
    umsg(msg);
    
    umsg_set_type(msg, UMSG_PUTC);
    umsg_set_arg0(msg, "con0");
    umsg_set_arg1(msg, 'A');
    umsg(msg);
    
    umsg_set_type(msg, UMSG_CLOSE);
    umsg_set_arg0(msg, "con0");
    umsg(msg);
    
    umsg_set_type(msg, UMSG_PUTC);
    umsg_set_arg0(msg, "con0");
    umsg_set_arg1(msg, 'B');
    umsg(msg);
    
    while (1);
    return 0;
}