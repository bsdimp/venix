#include "machos.h"
#ifdef VENIX
#include "pcvenix.h"
#else
#include "pcdos.h"
#endif

int run(MachineOS *mos);

int
main(int argc, char *argv[])
{
    MachineOS *mos;

#ifdef VENIX
    mos = new Venix();
#else
    mos = new IBMPC_DOS();
#endif
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <program name>\n", argv[0]);
        exit(0);
    }
    ram = (Byte*)alloc(0x100000);
    initialized = (Byte*)alloc(0x20000);
    mos->load(argc, argv);

    run(mos);
}
