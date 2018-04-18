# Installing Venix 86R 2.0 in MESS

This documents how to install Venix 86R in MESS running the Rainbow
emulation. You're on your own to get mame/mess, as well as translating
the keys to your variant.

## TL;DR Version

1. Install the hard drive / create the virtual hard drive
1. Low level format and partition the drive with wutil
1. Boot the Venix Boston Software Works extraction disk (vbswx1.img)
1. Partition the drive (again) as guided
1. Feed the floppys
1. Reboot (I forget, do I need to redo the bootable stuff with wutil here?)
1. Profit! You have a Venix 86R 2.0 BSW Edition system (the root password is gnomes).

## Original Documentaiton

The file VENIX_Install.pdf has the original instructions, scanned in
all their glory.  They are thin on the 'partition it' part, but
otherwise contain a good overview of the process. If you are
installing BSW, and you really want to install that unless you want to
be limited to the 10MB RD51 that's the only supported vanilla Venix
drive, then there will be additional prompts at the end for the 4 BSW
disks with the enhance UUCP, kermit, improved venix kernel, and bug
fixes to libc and the compiler.

## The Long, Plodding (but complete) Version

TBD, alas

### Obtain MAME/MESS

You're on your own for this. There's a chance we'll be patching the
Rainbow's BIOS slightly so we may need pull from git + patch + build
instructions here.

### Create the hard drive image

Mame stores its hard drive image in 'chd' format. This is a simple
wrapper around the raw file with data for the drivers inside of mame
to find the data properly. It has to know the disk geometry to do the
right CHS to LBA calcuations.

This is well documented in MAME, but there's some Caveats for the
Rainbow. First, the Rainbow controller only supports 16 sectors per
track. So all disks have that geometry. Next, due to limitations in
the boot software, sectors have to be 512 bytes (though the on-disk
format for describing the partition structure and boot structure is
still limited by the 256 byte sectors the DECmate II had, so if you go
looking at the boot code, you'll see that odd dicotomy).

Drive | Size | Sec/Track | Track/Cyl | Cyl | Form | Rebranded Drive / Notes
----- | ---- | --------- | --------- |---- | ---- | ----------------------
RD50 | 5M | 16 (17) | 4 | 153 | FH 5.25" | Seagate ST506
RD51 | 10M | 16 (17) | 4 | 306 | FH 5.25" | Seagate ST412
RD52 | 31M | 16 (18) | 7 | 480 | FH 5.25" | Quantum 540 / ATASI 3046
RD53 | 67M (71MB) | 16 (18) | 8 | 1024 | FH 5.25" | Microp 1325 (or 1335) w/ jumper at J7
RD31 | 20M | 16 (17) | 4 | 615 | HH 5.25" | Seagate ST225
RD32 | 40M | 16 (17) | 6 | 820 | HH 5.25" | Seagate ST251(-1)

Limitations: Controller has only room for 16 heads in registers, can
do only 1024 cylinders, and only 8 heads. And there's issue with the
impedance of the data lines if you have more than 6 heads in the
original HD cables. Venix (and MS-DOS) have 16-bit disk address
numbers, so can only handle logical partitions up to 32MiB/33MB. Also,
vanilla Venix supports only the RD51. BSW Venix supports what the
controller will support and also supports up to two drives.

There's three controllers that were available. DEC had the PC100RD51
and Suitable Solutions had a clone whose model number escapes me. CHS
systems had another clone that allowed two drives instead of one. All
the controllers had these limitations. Since the ROMs in the Rainbow
knew how to talk to the same WD2xxx chip that was in the PC100RD51,
they were all limited by its limitations.

So, for this walk through, we'll install onto a RD32/ST251 drive.
```
chdman createhd -o venix-rd32.chd -chd 820,6,16
```
Since we're installing the BSW version, this will be fine. If you want
to install the vanilla version, just make an RD51:
```
chdman createhd -o venix-rd51.chd -chd 306,4,16
```
