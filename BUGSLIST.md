# bugs list for kernel 

```
## <2020.8.7>
```
1. 经常出现错误：怀疑是进程阻塞和休眠的问题
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
## <2021.4.24>
1. unmount环回设备时不能正常close loop设备，因为fatfs文件模块没有对设备进行close操作的支持。
## <2021.5.25>
1. 当进行多次fork的时候，会出BUG
2. 当提高时钟切换频率时，fork时会出现BUG
