if [ -f "disk1.vhd" ]; then
    mv disk1.vhd disk1.dmg
fi

if [ -f "disk2.vhd" ]; then
    mv disk2.vhd disk2.dmg
fi

export DISK1_NAME=disk1.dmg

# 写boot区，定位到磁盘开头，写1个块：512字节
dd if=boot.bin of=$DISK1_NAME bs=512 conv=notrunc count=1

# 写loader区，定位到磁盘第2个块，写1个块：512字节
dd if=loader.bin of=$DISK1_NAME bs=512 conv=notrunc seek=1

# 写kernel区，定位到磁盘第100个块
dd if=kernel.elf of=$DISK1_NAME bs=512 conv=notrunc seek=100

# 写应用程序init，临时使用
# dd if=init.elf of=$DISK1_NAME bs=512 conv=notrunc seek=5000
# dd if=shell.elf of=$DISK1_NAME bs=512 conv=notrunc seek=5000

# 写应用程序，使用系统的挂载命令
export DISK2_NAME=disk2.dmg
export TARGET_PATH=mp
rm $TARGET_PATH
hdiutil attach $DISK2_NAME -mountpoint $TARGET_PATH
# cp -v init.elf $TARGET_PATH/init
# cp -v shell.elf $TARGET_PATH
# cp -v loop.elf $TARGET_PATH/loop
# cp -v snake.elf $TARGET_PATH/snake
cp -v *.elf $TARGET_PATH
hdiutil unmount $TARGET_PATH -verbose
