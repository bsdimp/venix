/*
 * Quick program to read a phyiscal RX-50 dump and turn it into a
 * logical dump that can be used.
 *
 * Tracks 0 and 1 have no interleave.
 * Tracks 2-79 have an interlave of 2, so the logical tracks are
 * stored in the following physical tracks:
 *	Physical	Logical
 *	1		1
 *	2		6
 *	3		2
 *	4		7
 *	5		3
 *	6		8
 *	7		4
 *	8		9
 *	9		5
 *	10		10
 *
 *	Physical	Logical
 *	1		1
 *	3		2
 *	5		3
 *	7		4
 *	9		5
 *	2		6
 *	4		7
 *	6		8
 *	8		9
 *	10		10
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

#define NSECTORS 10
#define SECTOR_LEN 512
#define SKIP_TRACKS 1
#define TOTAL_TRACKS 80

/*
 * If we have a logical dump, how do we turn it into a physical one?
 */
int p2l[NSECTORS] = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10};

/*
 * If we have a physical dump, how do we turn it into a logical one?
 */
int l2p[NSECTORS] = {1, 6, 2, 7, 3, 8, 4, 9, 5, 10};

uint8_t track_buffer[SECTOR_LEN * NSECTORS];

struct iovec twiddle_vect[NSECTORS];

int span = 7;

void
setup_twiddle_buf(struct iovec *iov, int *xlate, int n, int offset)
{
	int i, new;

	for (i = 0; i < n; i++) {
		/* Sectors are 1-based */
		new = xlate[(i + offset) % n] - 1;
//		fprintf(stderr, "Moving %d to %d\n", i, new);
		iov[new].iov_base = track_buffer + i * SECTOR_LEN;
		iov[new].iov_len = SECTOR_LEN;
//		fprintf(stderr, "%d offset %llx\n", new, (long long) ((uint8_t *)iov[new].iov_base - track_buffer));
	}
}

void
usage(void)
{
	errx(1, "[-i infile] [-o outfile] [-l]");
}

int
main(int argc, char **argv)
{
	int ch, i, in, out, lflag;
	ssize_t len;
	int offset;

	in = STDIN_FILENO;
	out = STDOUT_FILENO;
	lflag = 0;
	while ((ch = getopt(argc, argv, "i:lo:")) != -1) {
		switch (ch) {
		case 'i':
			if (in != STDIN_FILENO)
				close(in);
			in = open(optarg, O_RDONLY);
			if (in == -1)
				err(1, "Can't open %s for reading\n", optarg);
			break;
		case 'l':
			lflag = 1;
			break;
		case 'o':
			if (out != STDOUT_FILENO)
				close(out);
			out = open(optarg, O_WRONLY| O_CREAT);
			if (out == -1)
				err(1, "Can't open %s for reading\n", optarg);
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();
	fprintf(stderr, "Starting\n");
	for (i = 0; i < SKIP_TRACKS; i++) {
		fprintf(stderr, "Skipping track %d\n", i);
		read(in, track_buffer, sizeof(track_buffer));
	}

	offset = 0;
	for (; i < TOTAL_TRACKS; i++) {
		len = read(in, track_buffer, sizeof(track_buffer));
		if (len == -1)
			err(1, "bad read");
		else if (len != sizeof(track_buffer))
			errx(1, "short read");
		setup_twiddle_buf(twiddle_vect, l2p, NSECTORS, offset);
		len = writev(out, twiddle_vect, NSECTORS);
		if (len == -1)
			err(1, "bad write");
		else if (len != sizeof(track_buffer))
			errx(1, "short write");
		offset += span;
	}
	if (in != STDIN_FILENO)
		close(in);
	if (out != STDOUT_FILENO)
		close(out);
}
