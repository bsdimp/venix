This directory contains the raw floppies from Venix. The images are in
'physical' order, not the logical order that you'll sometimes
see. They have been compressed with xz. They don't use teledisk.

They are standard RX-50 disks: 10 sectors per track, single sided.
They are in physical order, not logical order.

The standard distribution is here, as well as the Boston Software
Works edition, which has a number of enhancments (it's the one you
want to run).

To install the standard version, boot one of the vxfer.flp disks (they
are all basically the same). This will install onto an RD-50
drive. That's your only choice.

To install the BSW version, use one of the vbswx.flp disks.

For both installations, you'll be prompted for disks to install, as
well as how to partition the drive. It's better if you partition with
something like wsutil (and may be required for this to be
bootable). It works well enough on real hardware, but mame still has a
few issues.

After the first disk, they are all uncompressed tar balls, but Venix
has a weird interleave.
