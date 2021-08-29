/*
 * Take care, this is included in assembler
 */
/* syscall with SYS in front its number then a comment for the number of args */
#define S_EXIT		1	/* special 1 __exit */
#define	S_FORK		2	/* 0 */
#define S_READ		3	/* 3 */
#define S_WRITE		4	/* 3 */
#define S_OPEN		5	/* 2 */
#define S_CLOSE		6	/* 1 */
#define S_WAIT		7	/* special 2 weird return rules */
#define	S_CREAT		8	/* 2 */
#define	S_LINK		9	/* 2 */
#define	S_UNLINK	10	/* 1 */
#define	S_EXEC		11	/* 2 */
#define S_CHDIR		12	/* 1 */
#define S_TIME		13	/* special 0 return to passed in pointer */
#define S_MKNOD		14	/* 3 */
#define	S_CHMOD		15	/* 2 */
#define	S_CHOWN		16	/* 3 */
#define S_BRK		17	/* special 1 brk and sbrk */
#define S_STAT		18	/* 2 */
#define S_LSEEK		19	/* 4 */
#define S_GETPID	20	/* 0 */
#define S_MOUNT		21	/* 3 */
#define	S_UMOUNT	22	/* 1 */
#define	S_SETUID	23	/* 1 */
#define	S_GETUID	24	/* 0 */
#define	S_STIME		25	/* special 2 weird return */
#define	S_PTRACE	26	/* 4 */
#define S_ALARM		27	/* 1 */
#define S_FSTAT		28	/* 2 */
#define S_PAUSE		29	/* 0 */
#define S_UTIME		30	/* 2 */
#define S_ACCESS	33	/* 2 */
#define	S_NICE		34	/* 1 */
#define	S_FTIME		35	/* 1 */
#define	S_SYNC		36	/* 0 */
#define	S_KILL		37	/* 2 */
/* 38-40 */
#define	S_DUP		41	/* 1 dup2 has 2, but to generate right we need only 1 for plain dup */
#define	S_PIPE		42	/* special 2 returns to a pointer */
#define	S_TIMES		43	/* 1 */
#define S_PROFIL	44	/* 4 */
/* 45 sem operations, atypical encoding */
#define	S_SETGID	46	/* 1 */
#define	S_GETGID	47	/* 0 */
#define	S_SIGNAL	48	/* special batshit crazy */
#define	S_SDATA		49	/* 3 */
#define	S_SUSPEND	50	/* 2 */
/* 51 */
#define	S_PHYS		52	/* 3 */
#define	S_LOCK		53	/* 1 */
#define	S_IOCTL		54	/* 3 */
#define	S_EXECE		59	/* 3 */
#define	S_UMASK		60	/* 1 */
#define	S_CHROOT	61	/* 1 */
#define	S_LOCKING	64	/* special like lseek */
