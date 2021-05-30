# riscv64开发环境搭建

## Linux环境搭建
riscv64开发需要交叉编译环境，于是需要下载riscv64对应的工具链，请下载(SiFive GCC 8.3.0-2020.04.0) 8.3.0  https://github.com/loboris/ktool/tree/master/kendryte-toolchain/bin

下载后，需要将kendryte-toolchain/bin配置到环境变量中，可以用文本编辑器打开/etc/profile，然后在末尾添加环境变量：
```bash
sudo vim /etc/profile
```
```bash
PATH=$PATH:yourpath/kendryte-toolchain/bin
example: PATH=$PATH:/usr/local/kendryte-toolchain/bin
```
编写好后，需要使这个配置生效，执行命令：
```bash
source /etc/profile
```
现在环境变量配置好了，还需要安装qemu虚拟机：
Ubuntu/Kali Linux: 
```bash
sudo apt-get install qemu-system-misc
```
Red hat/Fedora/Centos: 
```bash
sudo yum install qemu-system-misc
```
现在编译环境和虚拟机调试环境就搭建好了。

## 编译时build后直接run即可（可加-jn参数多线程编译，n是线程数。）：

！注意：如果要在虚拟机上面跑，则需要将sripts/localenv.mk中的ENV_REMOTE_TEST设置为no，默认是为socomp远程测试机做配置的，只能编译出在k210测试机上面跑的程序。

```bash
make build    # 构建环境
make run      # 编译并运行，默认使用qemu虚拟机运行
```
注意! 如果是要在开发板子上面跑，则需要在make build后，把sd卡插入读卡器，把读卡器插入电脑，然后用make sdcard SD="SD卡设备路径"把生成的FAT32文件系统写入sdcard。例如：
```
make sdcard SD=/dev/sdb
```
然后，再把SD卡插入k210开发板子，一边用数据线连接板子，另一边把USB接口插入电脑，然后输入make run即可，后面需要添加串口设备路径，默认设备路径是/dev/ttyUSB0。
```
make run K210_SERIALPORT=/dev/ttyUSB0
```

由于支持跨平台编译，所以编译时需要指定工具链和处理器平台。
```
make build CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=riscv64-k210
make run CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=riscv64-k210
```

如果是默认的toolchain和platform，则可以不添加CROSS_COMPILE和PLATFORM。可以在scripts/localenv.mk中查看。
默认工具链和平台：
```
CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=riscv64-k210
```
支持的PLATFORM有：
```
riscv64-k210 # 在k210开发板上跑测试程序
riscv64-qemu # 在qemu虚拟机上跑测试程序
```


## 编译时可用的命令：
```bash
make all      # 只编译内核源码
make build    # 构建环境
make debuild  # 清理环境
make run      # 编译并运行，默认使用qemu虚拟机运行
make qemu     # 使用qemu虚拟机运行
make clean    # 清除编译产生的对象文件以及可执行文件
make user     # 只编译用户程序（在开发应用时常用）
make user_clean     # 只清除用户态生成的内容
```

[回到首页](../../README.md)