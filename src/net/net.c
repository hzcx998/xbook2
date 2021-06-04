
#include <xbook/net.h>
#include <xbook/netif.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/fsal.h>
#include <xbook/netif.h>

/**
 * netin:
 * 网络输入线程，从网卡读取数据包，如果有数据就返回，没有数据就阻塞。
 * 这样可以减轻CPU负担。
 */
void netin_kthread(void *arg) 
{
    infoprint("[net] starting...\n");
    /* init netif */
    network_interface_init();

    while(1) {
        /* 检测输入，如果没有收到数据就会阻塞。 */
        network_interface_input();
        task_yield();
    }
}

void network_init(void)
{
    if (netcard_manager_init() < 0) {
        warnprint("[net] init netcard manager driver failed!\n");
        return;
    }
    
    /* 打开一个线程来读取网络数据包 */
    task_t * netin = kern_thread_start("netin", TASK_PRIO_LEVEL_NORMAL, netin_kthread, NULL);
    if (netin == NULL) {
        errprint("[net] start kthread netin failed!\n");
    }
}

