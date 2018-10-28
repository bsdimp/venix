/*
 *	Version 86/2.0		May 27, 1983
 *		Edited:	5/10/84
 *
 * (C)Copyright by VenturCom Inc. 1982,1983,1984
 * All rights reserved: VENTURCOM INC. 1982,1983,1984
 *
 *
 * The `panic' messages.
 */
char MP_init[]	= "Initialization I/O error";
char MP_btab[]	= "Bad system table (getfs)";
char MP_bdev[]	= "Bad device number (getblk)";
char MP_null[]	= "Null d_tab entry";
char MP_swap[]	= "Swap error";
char MP_iget[]	= "Bad system table (iget)";
char MP_npro[]	= "Bad system table (newproc)";
char MP_miss[]	= "Missing entry (unlink)";
char MP_ksig[]	= "Kernel signal";
char MP_recu[]	= "Recursive system call";

/*
 * The `prdev' messages.
 */
char MR_nspa[]	= "No space";
char MR_bblk[]	= "Bad block";
char MR_iout[]	= "Out of inodes";
char MR_bsbc[]	= "Bad superblock count";
char MR_erro[]	= "Error";

/*
 * The `printf' messages.
 */
char MF_fast[]	= "Fast alarm overflow\n";
char MF_file[]	= "In core file table full\n";
char MF_tabl[]	= "In core inode table full\n";
char MF_revl[]	= "VENIX/86   Version 2.1\n";
char MF_size[]	= "Total memory %dkb, Available %dkb, ";
char MF_pani[]	= "PANIC => %s\n";
char MF_err1[]	= "\n%s on the %s, unit %d\n";
char MF_read[]	= "reading";
char MF_writ[]	= "writing";
char MF_err2[]	= "%s while %s block number %d.  Status 0%o\n";
char MF_swap[]	= "Out of swap space\n";
char MF_segf[]	= "Segment table full\n";
char MF_mpar[]	= "Memory parity error (ignored)\n";
char MF_intr[]	= "Unknown interrupt (ignored)\n";	
char MF_sign[]	= "Signal %d\n";
