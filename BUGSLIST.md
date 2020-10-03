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
1. trigger捕捉用户态程序BUG
2. 强制结束窗口程序时可能会出现不能反回终端的状态。

## <2020.9.3>
1. 图层的隐藏和显示还是有一些bug。隐藏时，有时候会导致鼠标在任务栏重影。 [fixed]

## <2020.9.27>
1. 鼠标一直移动就会出现BUG

## <2020.9.28>
2. 鼠标在linux环境qemu启动有故障

## <2020.9.30>
1. IDE驱动程序不能在用户态进行读写？

## <2020.10.4>
1. 使用新的调度组合结构和，磁盘读写变慢
