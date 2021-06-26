# 内核中的各种id的概念以及其功能

## UID和GID
uid: 进程的实际用户ID,真实用户ID
euid: 进程的有效用户ID
gid: 进程的实际组ID,真实组ID
egid: 进程的有效用户组ID, 有效用户组ID

Linux进程有两个ID，一个就是用户ID，为每个用户的唯一标识符；另一个是组ID，为用户组的唯一标识符
UID：UID的值为0时，表示系统管理员；（1-99）为系统预设账号；（100-499）保留给一些服务使用；（500-65535）给一般使用者使用

## PID, PPID和PGID
pid: 进程的ID
ppid: 进程的父进程ID
pgid: 进程的组ID
tid: 线程的TID

## SID
sid: 进程的会话ID
