# 常用命令

## 反汇编一个程序
```
objdump -M intel -D hello > hello.dump
@ -M: 指定生成的文件汇编格式
        intel：intel格式汇编
@ -D: 表示要反汇编
@ hello: 要反汇编的文件
@ > : 重定向到一个文件
@ hello.dump： 反汇编内容存放的文件
```
