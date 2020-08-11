# QEMU配置参考文档

## 配置AHCI硬盘
```
1.
[..]
-drive file=[YOUR IMAGE],if=none,id=[AN IMAGE ID] \
-device ich9-ahci,id=ahci \
-device ide-drive,drive=[AN IMAGE ID],bus=ahci.0 \
[..]

2.
-drive file=[YOUR IMAGE],if=none,id=[AN IMAGE ID] \
-device ahci,id=ahci \
-device ide-drive,drive=[AN IMAGE ID],bus=ahci.0 \

```