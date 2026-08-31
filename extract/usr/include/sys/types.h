typedef	unsigned int	daddr_t;	/* disk address */
typedef	char *		caddr_t;	/* core address */
typedef	int		ino_t;		/* i-node number */
typedef	long		time_t;		/* a time */
typedef	int		dev_t;		/* device code */
typedef	long		off_t;		/* offset */

#define	major(x)	(int)(((unsigned)x>>8))
#define	minor(x)	(int)(x&0377)
#define	makedev(x,y)	(dev_t)((x)<<8|(y))
