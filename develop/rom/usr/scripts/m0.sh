echo 'mount device start'
mkfs -v -t fat16 /dev/ram0
mkdir /newfs
mount -v /dev/ram0 /newfs
ls /newfs
cp /bin/ls /newfs/ls2
ls /newfs
unmount -v /dev/ram0
rmdir /newfs
echo 'mount device done'