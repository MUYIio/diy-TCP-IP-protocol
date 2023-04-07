@REM 适用于windows
@REM https://blog.csdn.net/fantasy_wxe/article/details/21328217
@REM https://wiki.qemu.org/Documentation/Networking
@REM 两块网卡：一块和物理机器相连，另一块通过NAT模式连接外部网络
@REM qemu-system-x86_64   -m 1G  -monitor stdio -netdev user,id=mynet0 -cdrom fossapup64-9.5.iso -device e1000,netdev=mynet0
@REM start qemu-system-i386  -m 128M -s -S -monitor stdio -netdev user,id=mynet0 -device rtl8139,netdev=mynet0,mac=52:54:00:c9:18:27 -drive file=disk1.vhd,index=0,media=disk,format=raw -drive file=disk2.vhd,index=1,media=disk,format=raw
start qemu-system-i386  -m 128M -s -S -netdev tap,id=mynet0,ifname=tap -device rtl8139,netdev=mynet0,mac=52:54:00:c9:18:27 -netdev user,id=mynet1 -device rtl8139,netdev=mynet1,mac=52:54:00:c9:18:33 -drive file=disk1.vhd,index=0,media=disk,format=raw -drive file=disk2.vhd,index=1,media=disk,format=raw
