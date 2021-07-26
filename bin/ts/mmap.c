#include "test.h"
#include <sys/mman.h>

int test_mmap(int argc, char *argv[])
{
    char *mapaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapaddr == NULL) {
        printf("mmap failed!\n");
        return -1;
    }
    printf("mmap ok %p.\n", mapaddr);
    memset(mapaddr, 0x5a, 4096);

    mapaddr = mmap(NULL, 4096 * 10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapaddr == NULL) {
        printf("mmap failed!\n");
        return -1;
    }
    printf("mmap ok %p.\n", mapaddr);
    memset(mapaddr, 0x5a, 4096 * 10);
    
    return 0;
}
