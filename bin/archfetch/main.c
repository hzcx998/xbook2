#include <stdio.h>
#include <string.h>
#include <sys/proc.h>
#include <sys/sys.h>
#include <unistd.h>
#include <sys/vmm.h>
#include <const.h>

#define CPU_DEV "/dev/cpu0"

void print_logo(char *);
void print_uname();
void print_cpuinfo();
void print_mem();

// figlet xbook2
char *xbook2_logo[] = {"      _                 _    ____  ",
                       "__  _| |__   ___   ___ | | _|___ \\ ",
                       "\\ \\/ / '_ \\ / _ \\ / _ \\| |/ / __) |",
                       " >  <| |_) | (_) | (_) |   < / __/ ",
                       "/_/\\_\\_.__/ \\___/ \\___/|_|\\_\\_____|"};


int main(int argc, char *argv[])
{
    print_logo((char*)xbook2_logo);
    printf("+----------------------------------------------------+\n");
    print_uname();
    print_cpuinfo();
    print_mem();
    printf("+----------------------------------------------------+\n");
    return 0;
}

void print_logo(char *logo)
{
    int logo_length = sizeof(xbook2_logo) / sizeof(xbook2_logo[0]); // strlen(xbook2_logo)
    int i;
    for (i = 0; i < logo_length; i++)
    {
        printf("%s\n", xbook2_logo[i]);
    }
}

void print_uname()
{
    char buf[SYS_VER_LEN] = {0};
    getver(buf, SYS_VER_LEN);
    printf("  os: %s\n",buf);
}

void print_cpuinfo()
{
    int fd = open(CPU_DEV, O_RDONLY);
    if (fd < 0) {
        close(fd);
        return;
    }
    char buf[51] = {0};   // brand
    if (read(fd, buf, 50) < 0)  {
        close(fd);
        return;
    }
    printf(" cpu: %s\n", buf);
    close(fd);
}

void print_mem()
{
    mstate_t ms;
    mstate(&ms);
    int gbInt = ms.ms_total / GB;
    char *msg = " mem: ";
    if (gbInt >= 1) {
        printf("%s%dGB / %dGB", msg, ms.ms_used / GB,gbInt);
    } else {
        printf("%s%dMB / %dMB", msg, ms.ms_used / MB, ms.ms_total / MB);
    }
    printf("\n");
}
