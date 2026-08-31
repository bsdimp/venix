/*
 * Define the Boot Record/Partition Table on Block 0 of the XT winnie.
 */

struct xp {
	char	xp_code[446];		/* bootstrap record */
	struct {
		unsigned char	xp_boot;		/* boot indicator */
		unsigned char	xp_s_h, xp_s_s, xp_s_c;	/* starting address */
		unsigned char	xp_sys;			/* system type */
		unsigned char	xp_e_h, xp_e_s, xp_e_c;	/* ending address */
		long		xp_start;		/* starting sector */
		long		xp_size;		/* size in sectors */
	} xp_tab[4];				/* 4 partition tables */
	short	xp_sig;				/* signature */ 
};

#define	XP_BOOT		0x80		/* boot indicator */

#define	XP_UNKNOWN	0x00		/* unknown system type */
#define	XP_DOS		0x01		/* DOS system */
#define	XP_SYS		0x40		/* VENIX system/swap area */
#define	XP_TMP		0x41		/* VENIX temp/pipe area */
#define	XP_USR		0x42		/* VENIX user area */

#define	XP_SIG		0xaa55		/* magic constant */

