; 线程执行入口，启动时会先进入这个入口，再进入用户线程例程执行
[bits 32]
[section .text]

extern __pthread_exit     ; 导入函数

global __pthread_entry  ; 导出函数
__pthread_entry:
    push ebx            ; 参数入栈
    call ecx            ; 跳转到线程入口
    push eax            ; 退出状态入栈
    call __pthread_exit    ;调用线程退出
    jmp $               ; 退出失败