#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

#define TBLOCK	512
#define NBLOCK	20
#define NAMSIZ	100
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
	} dbuf;
} dblock, tbuf[NBLOCK];

int recno, first, mt, chksum;
int	nblock = 1;
struct stat stbuf;
char	*usefile;

static int
checksum(void)
{
	int i;
	char *cp;

	for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return(i);
}

#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

static void
v7select(int *pairp, struct stat *st)
{
	int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	printf("%c", *ap);
}

static void
pmode(struct stat *st)
{
	int **mp;

	for (mp = &m[0]; mp < &m[9];)
		v7select(*mp++, st);
}

static void
longt(struct stat *st)
{
	char *cp;
	char *ctime();

	pmode(st);
	printf("%3d/%1d", st->st_uid, st->st_gid);
	printf("%7zd", st->st_size);
	cp = ctime(&st->st_mtime);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

static void
done(int n)
{
	exit(n);
}

static void
copy(char *to, char *from)
{
	int i;

	i = TBLOCK;
	do {
		*to++ = *from++;
	} while (--i);
}

static int
readtape(char *buffer)
{
	int i, j;

//a	printf("Recno %d nblock %d\n", recno, nblock);
	if (recno >= nblock || first == 0) {
		if (first == 0 && nblock == 0)
			j = NBLOCK;
		else
			j = nblock;
//a		printf("Reading %d bytes from mt\n", TBLOCK * j);
		if ((i = read(mt, tbuf, TBLOCK*j)) < 0) {
			fprintf(stderr, "Tar: tape read error\n");
			return -1;
		}
		if (i == 0)
			return -1;
		if (first == 0) {
			if ((i % TBLOCK) != 0) {
				fprintf(stderr, "Tar: tape blocksize error\n");
				return -1;
			}
			i /= TBLOCK;
			if (i != nblock && i != 1) {
				fprintf(stderr, "Tar: blocksize = %d\n", i);
				nblock = i;
			}
		}
		recno = 0;
	}
	first = 1;
	copy(buffer, (char *)&tbuf[recno++]);
	return(TBLOCK);
}

static int
getdir(void)
{
	struct stat *sp;
	int i;

	if (readtape( (char *) &dblock) <= 0)
		return -1;
	if (dblock.dbuf.name[0] == '\0')
		return 0;
	sp = &stbuf;
	sscanf(dblock.dbuf.mode, "%o", &i);
	sp->st_mode = i;
	sscanf(dblock.dbuf.uid, "%o", &i);
	sp->st_uid = i;
	sscanf(dblock.dbuf.gid, "%o", &i);
	sp->st_gid = i;
	sscanf(dblock.dbuf.size, "%llo", &sp->st_size);
	sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
	sscanf(dblock.dbuf.chksum, "%o", &chksum);
	if (chksum != checksum())
		return 0;
	return 1;
}

static void
dotable(void)
{
	for (;;) {
		int rv;
		off_t where;

		where = lseek(mt, 0, SEEK_CUR) - (nblock - recno - 1) * TBLOCK;
		rv = getdir();
		if (rv == -1)
			break;
		if (rv == 0)
			continue;
//		printf("[0x%08lx-0x%08lx) ", where, where + (howmany(stbuf.st_size, TBLOCK) + 1) * TBLOCK);
		printf("%5lld-%5lld ", where / TBLOCK, where / TBLOCK + howmany(stbuf.st_size, TBLOCK) + 1);
		longt(&stbuf);
		printf("%s", dblock.dbuf.name);
		if (dblock.dbuf.linkflag == '1')
			printf(" linked to %s", dblock.dbuf.linkname);
		printf (" %lld data blocks", howmany(stbuf.st_size, TBLOCK));
		printf("\n");
	}
}

static void
usage(void)
{
	fprintf(stderr, "v7tarls fn\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	if (argc != 2)
		usage();

	usefile = argv[1];
	if (strcmp(usefile, "-") == 0) {
		mt = dup(0);
		nblock = 1;
	}
	else if ((mt = open(usefile, 0)) < 0) {
		fprintf(stderr, "tar: cannot open %s\n", usefile);
		done(1);
	}
	dotable();
}
