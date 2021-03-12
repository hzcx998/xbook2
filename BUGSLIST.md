# bugs list for kernel 

## <2020.7.2>
```
1. sendto can work only 1 time.

```
## <2020.8.7>
```
1. 经常出现错误：
assert(!list_find(&task->list, &wait_queue->wait_list)) failed:
file: task/waitqueue.c
base_file: task/waitqueue.c
ln: 17
spinning in assertion failure()

```

## <2021.2.1>
1. 视图刷新时，如果配置了ALPHA计算，在调整视图大小时，如果同时调整宽度和高度，会出现调整后直接变成透明的问题。
## <2021.2.24>
1. FATFS锁会导致同时打开的文件数量限制，需要修复该BUG，目前已经关闭了文件锁。
## <2021.3.10>
1. 多次执行shell程序会产生bug。原因是AHCI驱动导致
## <2021.3.12>
1. 使用IDE驱动时，会产生大量的内存泄露，怀疑是内存管理的问题，以及任务的创建和销毁时内存映射问题。
