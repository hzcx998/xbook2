# 安装tap0虚拟网卡流程
1. 安装tap-windows-9.9.2_3程序
2. 进入wintap-driver目录，选择系统版本，32位还是64位
3. 管理员模式打开cmd，执行addtap.bat就可以安装成功。
4. 在网络适配器中找到新添加的虚拟网卡，描述为TAP-WINDOWS Adapter V9 #2，将该网卡改名为tap0
5. 设置该网卡的ipv4属性：
    ip: 192.168.0.104
    mask: 255.255.0.0
    gatway: 192.168.0.1
6. 在xbook2的makefile的QEMU的配置中添加网卡配置。
7. 启动虚拟机即可自动连接tap0虚拟网卡。虚拟机通过tap0网卡和主机通信。
8. 注意，需要关闭windows的防火墙，xbook2才能ping通tap0网卡的ip地址。
