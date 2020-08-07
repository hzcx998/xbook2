# bugs list for kernel 

## <2020.4.27>
```
1. trigger handle with uthread failed! <<< bug fixed --2020.4.27

```
## <2020.7.2>
```
1. sendto can work only 1 time.

```
## <2020.7.11>
```
1. malloc problem, can't use it for work. <<< bug fixed -- 2020.7.13

```
## <2020.8.7>
```
1. 总是出现错误：
assert(!list_find(&task->list, &wait_queue->wait_list)) failed:
file: task/waitqueue.c
base_file: task/waitqueue.c
ln: 17
spinning in assertion failure()

```