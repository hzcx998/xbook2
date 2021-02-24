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

## <2020.10.8>
1. 多次对win程序进行最大化和最小化，会出现执行出错的故障

## <2020.12.2>
1. AHCI驱动不能再qemu中正常工作。（解决思路：查找一个可以执行AHCI的版本，来做内容的比较）

## <2021.1.29>
1. xtk 关闭窗口时出现bug，怀疑是激活图层的问题。 # fixed

## <2021.2.1>
1. 视图刷新时，如果配置了ALPHA计算，在调整视图大小时，如果同时调整宽度和高度，会出现调整后直接变成透明的问题。
## <2021.2.24>
1. FATFS锁会导致同时打开的文件数量限制，需要修复该BUG，目前已经关闭了文件锁。
