SYS=0xf1
	
indir=0
exit=1
fork=2
read=3
write=4
open=5
close=6
wait=7
creat=8
link=9
unlink=10
exec=11
chdir=12
time=13
mknod=14
chmod=15
chown=16
break=17
stat=18
lseek=19
getpid=20
mount=21
umount=22
setuid=23
getuid=24
stime=25
ptrace=26
alarm=27
fstat=28
pause=29
utime=30
	/ 31 gtty
	/ 32 stty
access=33
nice=34
ftime=35
sync=36
kill=37
	/ 38 csw
	/ 39 setpgrp
	/ 40 -- nothing --
dup=41
pipe=42
times=43
profil=44
sema=45
setgid=46
getgid=47
signal=48
sdata=49 / not pdp-11
suspend=50 / not pdp-11
	/ 51 acct
phys=52
lock=53
ioctl=54
	/ 55 reboot
	/ 56 mpx
	/ 57
	/ 58
exece=59 / setinf
umask=60 / pdp-11 also has getinf!
chroot=61
