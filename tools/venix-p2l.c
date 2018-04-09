/* Quick program to read venix floppies */
/*
 * The images are in physical format with T0H0 T0H1 T1H0 T1H1
 * and need to be in T0H0 T1H0 T2H0 .... T79H0 T0H1 T1H1...
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/param.h>

#define NSECTORS 15
#define NHEADS 2
#define SECTOR_LEN 512
#define NTRACKS 80

uint8_t buffer[SECTOR_LEN * NTRACKS * NHEADS * NSECTORS];
struct iovec iov[NTRACKS * NHEADS];

void
usage(void)
{
	errx(1, "[-i infile] [-o outfile]");
}

int
main(int argc, char **argv)
{
	int ch, i, in, out;
	ssize_t len;
	int offset;

	in = STDIN_FILENO;
	out = STDOUT_FILENO;
	while ((ch = getopt(argc, argv, "i:o:")) != -1) {
		switch (ch) {
		case 'i':
			if (in != STDIN_FILENO)
				close(in);
			in = open(optarg, O_RDONLY);
			if (in == -1)
				err(1, "Can't open %s for reading\n", optarg);
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
	len = read(in, buffer, sizeof(buffer));
	if (len == -1)
		err(1, "read");
	if (len != sizeof(buffer))
		errx(1, "read wrong size %zd", len);
	for (int track = 0; track < NTRACKS; track++) {
		for (int head = 0; head < NHEADS; head++) {
			int a, b;

			a = track * NHEADS + head;	/* old offset (input) */
//			b = head * NTRACKS + track;	/* new offset (output) */
			if (head == 0)
				b = track;
			else
				b = NTRACKS * 2 - track - 1;
			iov[b].iov_base = buffer +  a * SECTOR_LEN * NSECTORS;
			iov[b].iov_len = SECTOR_LEN * NSECTORS;
//			fprintf(stderr, "Head %d track %d in slot %d (buffer offset %d) (block start %d moves to %d\n", head, track, b, a, a * NSECTORS, b * NSECTORS);
		}
	}
	len = writev(out, iov, NTRACKS * NHEADS);
	if (len == -1)
		err(1, "write");
	if (len != sizeof(buffer))
		errx(1, "write wrong size %zd", len);
	if (in != STDIN_FILENO)
		close(in);
	if (out != STDOUT_FILENO)
		close(out);
}
