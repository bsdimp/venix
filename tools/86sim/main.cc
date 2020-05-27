#include <unistd.h>

#include "machos.h"
#ifdef VENIX
#include "pcvenix.h"
#else
#include "pcdos.h"
#endif

const char *prog;

void
usage()
{
	fprintf(stderr, "Usage: %s <program name>\n", prog);
        exit(0);
}

int
main(int argc, char *argv[])
{
	MachineOS *mos;
	int ch;

#ifdef VENIX
	mos = new Venix();
#else
	mos = new IBMPC_DOS();
#endif

	prog = argv[0];
	while ((ch = getopt(argc, argv, "b:")) != -1) {
		switch (ch) {
		case 'b':
			mos->set_emu_base(optarg);
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	mos->init();
	mos->load(argc, argv);
	mos->run();
}
