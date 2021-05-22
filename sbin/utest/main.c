
int main()
{
    char *v = (char *)0x000000;
    *v = 0x1000;
    asm ("ecall");
    while (1) {
        
    }
    return 0;
}