/*
* NOTE: This driver assumes that during booting, the firmware will
*       have setup the drive characteristics in the controller.
*/

#include	"at.h"

#include	"atstrategy.c"

#include	"atreread.c"

#include	"atstart.c"

#include	"atintr.c"

#include	"atcmd.c"

#include	"atread.c"

#include	"atwrite.c"

#include	"atioctl.c"
