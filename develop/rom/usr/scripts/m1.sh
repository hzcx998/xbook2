echo 'process1'
mkdir /newfs
mount -o loop /tmp/d.img /newfs
ls newfs
unmount /newfs
rmdir /newfs
echo 'process2'
mkdir /newfs
mount -o loop /tmp/d.img /newfs
ls newfs
unmount /newfs
rmdir /newfs
echo 'process3'
mkdir /newfs
mount -o loopback=/dev/loop3 /tmp/d.img /newfs
ls newfs
unmount /newfs
rmdir /newfs
echo 'process error'
mkdir /newfs
mount -o loopback2=/dev/loop3 /tmp/d.img /newfs
ls newfs
unmount /newfs
rmdir /newfs
echo 'mount test done!'