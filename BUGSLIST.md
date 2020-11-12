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

## <2020.9.1>
2. 强制结束窗口程序时可能会出现不能反回终端的状态。

## <2020.9.28>
2. 鼠标在linux环境qemu启动有故障

## <2020.10.8>
1. 多次对win程序进行最大化和最小化，会出现执行出错的故障

## <2020.11.13>
1. tty终止前端进程出错。
