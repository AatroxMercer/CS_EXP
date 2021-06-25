# debootsrap (Failed)

## get ready

```bash
# enter work dir
cd ~/Desktop/_workspace/cs
# install ubuntu 18.04 base system
sudo debootstrap bionic bionic
# download linux kernel code and busybox code
curl -o linux-4.20.tar.gz http://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.20.tar.gz
curl -o busybox-1.31.1.tar.bz2 https://busybox.net/downloads/busybox-1.31.1.tar.bz2
# extract tar file into base system
sudo tar -xvzf linux-4.20.tar.gz -C ./bionic/root/
sudo tar -xvjf busybox-1.31.1.tar.bz2 -C ./bionic/root/

# change root to base system
sudo chroot ./bionic

# change apt source and update
vim /etc/apt/sources.list
apt update
apt full-upgrade
apt autoremove

# install gcc-arm-linux-gnueabi then qemu
apt install gcc-arm-linux-gnueabi
apt install qemu
arm-linux-gnueabi-gcc --target

# install components
apt install -y vim gcc make bc libncurses-dev libelf-dev g++ gperf bison flex texinfo help2man libncurses5-dev python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip patch libstdc++6

exit
```

```bash
# sources.list

deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-backports main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-proposed main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-security main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-updates main multiverse restricted universe
```

## build QEMU

```bash
# change root to base system
cd ~/Desktop/_workspace/cs
sudo chroot ./bionic

# compile busybox 
cd /root/busybox-1.31.1
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- install
# You will probably need to make your busybox binary setuid root to ensure all configured applets will work properly.

# complete virtual system root
cd /root/busybox-1.31.1/_install
mkdir etc proc sys tmp dev lib
	# complete dev dir
	cd /root/busybox-1.31.1/dev
	mknod -m 666 tty1 c 4 1; mknod -m 666 tty2 c 4 2; mknod -m 666 tty3 c 4 3; mknod -m 666 tty4 c 4 4; mknod -m 666 console c 5 1; mknod -m 666 null c 1 3; echo done;
    # complete etc
    cd /root/busybox-1.31.1/_install/etc
    vim ./inittab
    vim ./fstab
    mkdir init.d
    vim ./init.d/rcS
    chmod 777 ./init.d/rcS
    
    # find dependency lib
    cd /root/busybox-1.31.1
    arm-linux-gnueabi-readelf -d busybox | grep NEEDED; arm-linux-gnueabi-objdump -x busybox | grep NEEDED; strings busybox | grep ^lib;
    cp /usr/arm-linux-gnueabi/lib/ld-linux.so.3 _install/lib/; cp /usr/arm-linux-gnueabi/lib/libc.so.6 _install/lib/; cp /usr/arm-linux-gnueabi/lib/libm.so.6 _install/lib/; cp /usr/arm-linux-gnueabi/lib/libresolv.so.2 _install/lib/; echo done;

# compile core
cd /root/linux-4.20
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- vexpress_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- modules_install INSTALL_MOD_PATH=/root/busybox-1.31.1/_install/
ls /root/busybox-1.31.1/_install/lib/modules
	
# convenient copy
mkdir /root/arm
cd /root/arm
cp /root/linux-4.20/arch/arm/boot/zImage ./
cp /root/linux-4.20/arch/arm/boot/dts/vexpress-v2p-ca9.dtb ./

# create root file system image file
cd /root/arm
dd if=/dev/zero of=rootfs.ext3 bs=1M count=32
mkfs.ext3 rootfs.ext3
mkdir fs
mount -o loop rootfs.ext3 ./fs
# mount: ./fs mount failed: Operation not permitted
# chroot 之后 mount失败，取消chroot后成功. (可能是因为缺少了proc sys run dev下等信息)
	exit
	cd bionic/root/arm
	mount -o loop rootfs.ext3 ./fs
	cp -rf ../busybox-1.31.1/_install/* ./fs/
	umount ./fs

	# 解决方法：在chroot之前，将主系统的proc, sys, dev挂载进base system


# convenient run bash
sudo chroot ./bionic
cd /root/arm
vim run.sh
chmod +x run.sh

exit
```

```bash
# run.sh

qemu-system-arm \
        -M vexpress-a9 \
        -kernel ./zImage \
        -nographic \
        -m 512M \
        -smp 4 \
        -sd ./rootfs.ext3 \
        -dtb ./vexpress-v2p-ca9.dtb \
        -append "init=/linuxrc root=/dev/mmcblk0 rw rootwait earlyprintk console=ttyAMA0"
```

## test QEMU

```bash
# change root to base system
cd ~/Desktop/_workspace/cs
sudo chroot ./bionic

# test
cd /root/arm
./run.sh
# to quit: ctrl+a , then x

exit
```

## build crosstool-ng

```bash
# download crosstool-ng source code
cd ~/Desktop/_workspace/cs
git clone https://github.com/crosstool-ng/crosstool-ng.git
sudo cp crosstool-ng ./bionic/root/arm/

# change root to base system
sudo chroot ./bionic
cd /root

# install dependencies
apt install -y gcc g++ gperf bison flex texinfo help2man make libncurses5-dev python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip patch libstdc++6 git


# compile crosstool-ng
cd /root/crosstool-ng
./bootstrap; ./configure; make; make install;
ct-ng -v
ct-ng list-samples | grep arm-unknown-linux-gnueabi


# make the compiler
mkdir /root/arm/crosstool
cd /root/arm/crosstool
mkdir src bin
gcc --version
ldd --version

ct-ng arm-unknown-linux-gnueabi
ct-ng menuconfig

ct-ng build
# [ERROR]  You must NOT be root to run crosstool-NG
	echo -e "CT_EXPERIMENTAL=y\nCT_ALLOW_BUILD_AS_ROOT=y\nCT_ALLOW_BUILD_AS_ROOT_SURE=Y" >> .config

cat build.log
# [DEBUG]    /usr/bin/ld: cannot find -lstdc++
# [ERROR]    collect2: error: ld returned 1 exit status
```

## Failed

# VirtualBox

## get ready

```bash
# sources.list
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-backports main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-proposed main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-security main multiverse restricted universe
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu bionic-updates main multiverse restricted universe
```

```bash
# after virtualbox build
# change apt source and update
vim /etc/apt/sources.list
sudo apt update; sudo apt full-upgrade

# downlaod src codes
mkdir ~/exp ~/Downloads

cd ~/Downloads
curl -o linux-4.20.tar.gz http://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.20.tar.gz
curl -o busybox-1.31.1.tar.bz2 https://busybox.net/downloads/busybox-1.31.1.tar.bz2
# extract tar file into working directory
sudo tar -xvzf linux-4.20.tar.gz -C ~/exp
sudo tar -xvjf busybox-1.31.1.tar.bz2 -C ~/exp

# cd wd
cd ~/exp

# install gcc-arm-linux-gnueabi and qemu
sudo apt install gcc-arm-linux-gnueabi
sudo apt install qemu
arm-linux-gnueabi-gcc --target

# install components
sudo apt install -y vim gcc make bc libncurses-dev libelf-dev g++ gperf bison flex texinfo help2man libncurses5-dev python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip patch libstdc++6 git

sudo apt autoremove
```

## compile busybox

```bash
# etc/inittab
::sysinit:/etc/init.d/rcS
::askfirst:/bin/sh
::ctrlaltdel:/sbin/reboot
::shutdown:/sbin/swapoff -a
::shutdown:/bin/umount -a -r
::restart:/sbin/init
tty2::askfirst:/bin/sh
tty3::askfirst:/bin/sh
tty4::askfirst:/bin/sh
```

```bash
# etc/fstab
#device mount-point type option dump fsck order
proc  /proc proc  defaults 0 0
temps /tmp  rpoc  defaults 0 0
none  /tmp  ramfs defaults 0 0
sysfs /sys  sysfs defaults 0 0
mdev  /dev  ramfs defaults 0 0
```

```bash
# etc/init.d/rcS
mount -a
echo "/sbin/mdev" > /proc/sys/kernel/hotplug
/sbin/mdev -s       # 根据/etc/mdev.conf中的配置进行生成设备节点
mount -a
```

```bash
cd ~/exp/busybox-1.31.1
sudo su
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- install
exit
# You will probably need to make your busybox binary setuid root to ensure all configured applets will work properly.

# complete virtual system root
cd ./_install
sudo mkdir etc proc sys tmp dev lib
	# complete dev dir
	cd ./dev
	mknod -m 666 tty1 c 4 1; mknod -m 666 tty2 c 4 2; mknod -m 666 tty3 c 4 3; mknod -m 666 tty4 c 4 4; mknod -m 666 console c 5 1; mknod -m 666 null c 1 3; echo done;
	
    # complete etc
    cd ../etc
    sudo su
        vim ./inittab
        vim ./fstab
        mkdir init.d; vim ./init.d/rcS; chmod 777 ./init.d/rcS
    exit
    
    # find dependency lib
    cd ~/exp/busybox-1.31.1
    arm-linux-gnueabi-readelf -d busybox | grep NEEDED; echo; arm-linux-gnueabi-objdump -x busybox | grep NEEDED; echo; strings busybox | grep ^lib;
    
    cd ./_install/lib
    sudo su
        cp /usr/arm-linux-gnueabi/lib/ld-linux.so.3 .
        cp /usr/arm-linux-gnueabi/lib/libc.so.6 .
        cp /usr/arm-linux-gnueabi/lib/libm.so.6 .
        cp /usr/arm-linux-gnueabi/lib/libresolv.so.2 .
	exit
```

## compile Linux core

```bash
cd ~/exp/linux-4.20
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- vexpress_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- modules_install INSTALL_MOD_PATH=/home/sb/exp/busybox-1.31.1/_install/

ls /home/sb/exp/busybox-1.31.1/_install/lib/modules
	
# convenient copy
cd ~/exp/; mkdir arm;;  cd arm
cp /../linux-4.20/arch/arm/boot/zImage .
cp /../linux-4.20/arch/arm/boot/dts/vexpress-v2p-ca9.dtb .
```

## build QEMU

```makefile
run:
    qemu-system-arm \
            -M vexpress-a9 \
            -kernel ./zImage \
            -nographic \
            -m 512M \
            -smp 4 \
            -sd ./rootfs.ext3 \
            -dtb ./vexpress-v2p-ca9.dtb \
            -append "init=/linuxrc root=/dev/mmcblk0 rw rootwait earlyprintk console=ttyAMA0"

mount:
    mount -o loop rootfs.ext3 root
    
umount:
    umount root
```



```bash
# create root file system image file
cd ~/exp/arm
dd if=/dev/zero of=rootfs.ext3 bs=1M count=32
mkfs.ext3 rootfs.ext3
mkdir root
sudo su
    mount -o loop rootfs.ext3 root
    cp -rf ../busybox-1.31.1/_install/* ./fs/
    umount root
exit

# convenient run bash
vim run.sh
chmod +x run.sh
./run.sh

# to quit: ctrl+a , then x
```

## compile crosstool-ng

```bash
# download crosstool-ng source code
cd ~/Downloads
git clone https://github.com/crosstool-ng/crosstool-ng.git
cp crosstool-ng ../exp/ -r

# compile crosstool-ng
cd /root/crosstool-ng
./bootstrap; ./configure; make
sudo make install

ct-ng -v
ct-ng list-samples | grep arm-unknown-linux-gnueabi
```

## build cross compiling tool

```bash
# make the compiler
cd ~/exp/arm
mkdir crosstool; cd crosstool
ct-ng arm-unknown-linux-gnueabi
ct-ng menuconfig
	Paths
		Debug crosstool-NG
			intermediate steps
		Number of parallel jobs 
	Target option
		Target Architecture - "arm"
		Architecture level - "ARMv7"
		Emit assembly for CPU - cortex-a7
	Toolchain option
		Tuple's vendor string - "sb"

ct-ng build
# take a break

ln -s /home/sb/x-tools/arm-sb-linux-gnueabi/bin .
```

## test cross compiling tool

```bash
# Makefile
DIR_WORK = /home/sb/x-tools/arm-sb-linux-gnueabi/bin
PATH_GCC = ${DIR_WORK}/arm-sb-linux-gnueabi-gcc
DIR_SRC  = /home/sb/exp/arm/src
DIR_DST  = /home/sb/exp/arm/root/workspace/

.PHONY: hello_world bf run mount umount

qemu:
	./run.sh

hello_world: ${DIR_SRC}/hello_world.c
	${PATH_GCC} -o hello_world ${DIR_SRC}/hello_world.c
	sudo mount -o loop rootfs.ext3 root
	sudo cp hello_world ${DIR_DST}
	sudo umount root

mount:
	sudo mount -o loop rootfs.ext3 root
umount:
	sudo umount root
```

```c
// hello_world.c
#include <stdio.h>

int main() {
	printf("Hello, world!\n");
	printf("[%d]", EOF);
	return 0;
}
```

```bash
cd ~/exp/arm

# convinient soft link
ln -s /home/sb/x-tools/arm-sb-linux-gnueabi/bin .
mkdir src

vim Makefile
vim ./src/hello_world.c

make hello_world

# in qemu
cd /workspace
./hello_world
```

