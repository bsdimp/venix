/*
 * NOTE: This driver assumes that during booting, the firmware will
 *       have setup the drive characteristics in the controller.
 */

#include	"xt.h"

#include	"xtstrategy.c"

#include	"xtstart.c"

#include	"xtintr.c"

#include	"xtcmd.c"

#include	"xtread.c"

#include	"xtwrite.c"

#include	"xtioctl.c"
