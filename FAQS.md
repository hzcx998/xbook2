# 一些疑问与解答！
Q: ld: Error: unable to disambiguate: -no-pie (did you mean --no-pie ?)  
A: 这是因为新版本的ld不支持该参数导致，因此需要降低ld版本，由于ld属于Binutils中，所以要降低Binutils版本，参考如下方式：
```
# 下载binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.gz
tar zxvf  binutils-2.30.tar.gz
# 进入源码目录编译安装
cd binutils-2.30
./configure
make
make install
# 替换原来的ld
cd /usr/bin
mv ld ld.copy #备份旧版本ld
cd /usr/local/bin
ln -s ld /usr/bin/ld
# 验证ld版本是否降下来了
ld --version
```