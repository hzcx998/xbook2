#include <sys/syscall.h>

int main(void) {
    return syscall0(int, SYS_SHUTDOWN);
}