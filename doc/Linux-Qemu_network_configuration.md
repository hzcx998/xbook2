### Linux下qemu网络配置
>示例系统：ubuntu 18.04

1. 安装docker。
```bash
sudo apt install docker.io
```
2. 建立docker。
```bash
systemctl start docker
```
这时使用ifconfig，可以看到docker0。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200726004307579.png)

3. 下载安装网桥有关工具。
```bash
sudo apt-get install bridge-utils        # 虚拟网桥工具
sudo apt-get install uml-utilities       # UML（User-mode linux）工具
```

4. 创建tap设备，作为qemu端的一个接口。
```bash
sudo tunctl -t tap0                                          # 创建一个 tap0 接口
sudo brctl addif docker0 tap0                   # 在虚拟网桥中增加一个 tap0 接口
sudo ifconfig tap0 0.0.0.0 promisc up    # 启用 tap0 接口
brctl showstp docker0                       # 显示 docker 的各个接口
```
显示结果示例：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200726004209603.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNzY5NTcy,size_16,color_FFFFFF,t_70)

>此时tap0的状态为disable是正常的。
>打开虚拟机之后状态会变为forwording。

5. qemu的选项中添加：
```bash
-net nic,model=rtl8139 -net tap,ifname=tap0,script=no,downscript=no

参数意义：
①-net nic,model=rtl8139：表示希望qemu在虚拟机中模拟一张rtl8139虚拟网卡，默认会模拟一张e1000网卡。
②-net tap,ifname=tap0,script=no,downscript=no：qemu使用tap网络通信方式，并且指定了网卡接口名称为tap0。设置宿主机在启动客户机时自动执行的网络配置脚本和宿主机在客户机关闭时自动执行的网络配置脚本。由于qemu中运行自主系统，所以这里不使用系统脚本。
```
>qemu网络通信方式：
>1. User mode stack：用户协议栈方式，这种方式的大概原理是在 QEMU 进程中实现一个协议栈，这个协议栈可以被视为一个主机与虚拟机之间的 NAT 服务器，它负责将 QEMU 所模拟的系统网络请求转发到外部网卡上面，从而实现网络通信。但是不能将外面的请求转发到虚拟机内部，并且虚拟机 VLAN 中的每个接口必须放在 10.0.2.0 子网中。
>2. socket： 为 VLAN 创建套接字，并把多个 VLAN 连接起来。
>3. TAP/bridge：最重要的一种通信方式，我们想要实现 QEMU 虚拟机和外部通信就需要使用这种方式。
>4. VDE：也是用于连接 VLAN 的，如果没有 VLAN 连接需求基本用不到。

#### 配置过程中若出现错误需要删除网桥或网络接口可使用下列命令：
* 删除虚拟网卡接口tap0
```bash
tunctl -d <虚拟网卡名>
#示例：
#sudo tunctl -d tap0
```
* 刪除虚拟网桥
```bash
ifconfig <网桥名> down
brctl delbr <网桥名>
#示例：
#sudo ifconfig docker0 down
#sudo brctl -d docker0
```
* 将网卡tap0移出网桥docker0
```bash
sudo brctl delif docker0 tap0
```
### xbook2系统检测是否配置成功
将service/netsrv/main.c中的ip地址修改为和docker一个网段的地址。
修改示例：
```c
    #if CONFIG_LEVEL == 0
    IP4_ADDR(&ipaddr, 172,17,1,1);
    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,0, 0);
```
在xbook2目录下执行：
```bash
sudo make run
#使用tap通信方式会访问系统文件/dev/net/tun，所以使用sudo启动。
```
在宿主机浏览器中访问该配置的服务器ip地址。接收到数据包则配置成功。示例如下。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200726005523633.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNzY5NTcy,size_16,color_FFFFFF,t_70)
>在宿主机重启后之前配置的网桥和tap0接口会消失，需要重新配置。

>以上为个人配置。
>如出现错误或配置不成功可查阅其他资料。

>参考文献：
>[1] CataLpa. [QEMU 网络配置一把梭](https://wzt.ac.cn/2019/09/10/QEMU-networking/) 2019-09-10
>[2] jongwu3. [一种简单的qemu网络配置方法](https://blog.csdn.net/wujianyongw4/article/details/80497528) 2018-05-29
>[3] leoe_. [Ubuntu 删除虚拟网卡/网桥的命令](https://blog.csdn.net/LEoe_/article/details/78974079) 2018-01-04
