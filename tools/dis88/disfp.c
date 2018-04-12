static char *sccsid =
   "@(#) disfp.c, Ver. 2.1 created 00:00:00 87/09/01";

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *                                                         *
  *  Copyright (C) 1987 G. M. Harding, all rights reserved  *
  *                                                         *
  * Permission to copy and  redistribute is hereby granted, *
  * provided full source code,  with all copyright notices, *
  * accompanies any redistribution.                         *
  *                                                         *
  * This file contains handler routines for the numeric op- *
  * codes of the 8087 co-processor,  as well as a few other *
  * opcodes which are related to 8087 emulation.            *
  *                                                         *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "dis.h"              /* Disassembler declarations  */

#define FPINT0 0xd8           /* Floating-point interrupts  */
#define FPINT1 0xd9
#define FPINT2 0xda
#define FPINT3 0xdb
#define FPINT4 0xdc
#define FPINT5 0xdd
#define FPINT6 0xde
#define FPINT7 0xdf

                              /* Test for floating opcodes  */
#define ISFLOP(x) \
   (((x) >= FPINT0) && ((x) <= FPINT7))

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *                                                         *
  * This is the  handler for the escape  family of opcodes. *
  * These opcodes place the contents of a specified  memory *
  * location on the system bus,  for access by a peripheral *
  * or by a co-processor such as the 8087. (The 8087 NDP is *
  * accessed  only  via bus  escapes.)  Due to a bug in the *
  * PC/IX assembler,  the "esc" mnemonic is not recognized; *
  * consequently,  escape opcodes are disassembled as .byte *
  * directives,  with the appropriate  mnemonic and operand *
  * included as a comment.  FOR NOW, those escape sequences *
  * corresponding  to 8087  opcodes  are  treated as simple *
  * escapes.                                                *
  *                                                         *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
eshand(j)

   register int j;            /* Pointer to optab[] entry   */

{/* * * * * * * * * *  START OF eshand()  * * * * * * * * * */

   register char *a;
   register int k;

   objini(j);

   FETCH(k);

   a = mtrans((j & 0xfd),(k & 0xc7),TR_STD);

   mtrunc(a);

   printf("\t.byte\t0x%02.2x\t\t| esc\t%s\n",j,a);

   for (k = 1; k < objptr; ++k)
      printf("\t.byte\t0x%02.2x\n",objbuf[k]);

}/* * * * * * * * * * * END OF eshand() * * * * * * * * * * */

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *                                                         *
  * This is the handler routine for floating-point opcodes. *
  * Since PC/IX must  accommodate  systems with and without *
  * 8087 co-processors, it allows floating-point operations *
  * to be  initiated  in either of two ways:  by a software *
  * interrput whose type is in the range 0xd8 through 0xdf, *
  * or by a CPU escape sequence, which is invoked by an op- *
  * code in the same range.  In either case, the subsequent *
  * byte determines the actual numeric operation to be per- *
  * formed.  However,  depending  on the  method of access, *
  * either  one or two code bytes will  precede  that byte, *
  * and the fphand()  routine has no way of knowing whether *
  * it was invoked by  interrupt or by an escape  sequence. *
  * Therefore, unlike all of the other handler routines ex- *
  * cept dfhand(),  fphand() does not initialize the object *
  * buffer, leaving that chore to the caller.               *
  *                                                         *
  * FOR NOW,  fphand()  does not disassemble floating-point *
  * opcodes to floating  mnemonics,  but simply outputs the *
  * object code as .byte directives.                        *
  *                                                         *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Only first order approximately right */

char *fptab[] =
{
	"\tfadd",			/* 0xd8 xxx00 0xxx */
	"\tfmul",			/* 0xd8 xxx00 1xxx */
	"\tfcom",			/* 0xd8 xxx01 0xxx */
	"\tfcomp",			/* 0xd8 xxx01 1xxx */
	"\tfsub",			/* 0xd8 xxx10 0xxx */
	"\tfsubr",			/* 0xd8 xxx10 1xxx */
	"\tfdiv",			/* 0xd8 xxx11 0xxx */
	"\tfdivr",			/* 0xd8 xxx11 1xxx */

	"\tfld",			/* 0xd9 xxx00 0xxx */
	"\tfst",			/* 0xd9 xxx00 1xxx */
	"\tfstp",			/* 0xd9 xxx01 0xxx */
	"\tfstcw",			/* 0xd9 xxx01 1xxx */
	"\tfldenv",			/* 0xd9 xxx10 0xxx */
	"\tfldcw",			/* 0xd9 xxx10 1xxx */
	"\tfstenv",			/* 0xd9 xxx11 0xxx */
	"\tfstcw",			/* 0xd9 xxx11 1xxx */

	"\tfiadd",			/* 0xda xxx00 0xxx */
	"\tfimul",			/* 0xda xxx00 1xxx */
	"\tficom",			/* 0xda xxx01 0xxx */
	"\tficomp",			/* 0xda xxx01 1xxx */
	"\tfisub",			/* 0xda xxx10 0xxx */
	"\tfisubr",			/* 0xda xxx10 1xxx */
	"\tfidiv",			/* 0xda xxx11 0xxx */
	"\tfidivr",			/* 0xda xxx11 1xxx */

	"\tfild",			/* 0xdb xxx00 0xxx */
	"\tfist",			/* 0xdb xxx00 1xxx */
	"\tfistp",			/* 0xdb xxx01 0xxx */
	"\tfistcw-xx",			/* 0xdb xxx01 1xxx */
	"\tfildenv-xx",			/* 0xdb xxx10 0xxx */
	"\tfld",			/* 0xdb xxx10 1xxx */
	"\tfildcw-xx",			/* 0xdb xxx11 0xxx */
	"\tfistcw-xx",			/* 0xdb xxx11 1xxx */

	"\tfadd",			/* 0xdc xxx00 0xxx */
	"\tfmul",			/* 0xdc xxx00 1xxx */
	"\tfcom",			/* 0xdc xxx01 0xxx */
	"\tfcomp",			/* 0xdc xxx01 1xxx */
	"\tfsub",			/* 0xdc xxx10 0xxx */
	"\tfsubr",			/* 0xdc xxx10 1xxx */
	"\tfdiv",			/* 0xdc xxx11 0xxx */
	"\tfdivr",			/* 0xdc xxx11 1xxx */

	"\tfld",			/* 0xdd xxx00 0xxx */
	"\tfst",			/* 0xdd xxx00 1xxx */
	"\tfstp",			/* 0xdd xxx01 0xxx */
	"\tfstcw",			/* 0xdd xxx01 1xxx */
	"\tfldenv",			/* 0xdd xxx10 0xxx */
	"\tfstenv",			/* 0xdd xxx10 1xxx */
	"\tfldcw",			/* 0xdd xxx11 0xxx */
	"\tfstcw-xxx",			/* 0xdd xxx11 1xxx */

	"\tfiadd",			/* 0xde xxx00 0xxx */
	"\tfimul",			/* 0xde xxx00 1xxx */
	"\tficom",			/* 0xde xxx01 0xxx */
	"\tficomp",			/* 0xde xxx01 1xxx */
	"\tfisub",			/* 0xde xxx10 0xxx */
	"\tfisubr",			/* 0xde xxx10 1xxx */
	"\tfidiv",			/* 0xde xxx11 0xxx */
	"\tfidivr",			/* 0xde xxx11 1xxx */

	"\tfild",			/* 0xdf xxx00 0xxx */
	"\tfist",			/* 0xdf xxx00 1xxx */
	"\tfistp",			/* 0xdf xxx01 0xxx */
	"\tfistp",			/* 0xdf xxx01 1xxx */
	"\tfbld",			/* 0xdf xxx10 0xxx */
	"\tfbstp",			/* 0xdf xxx10 1xxx */
	"\tfild",			/* 0xdf xxx11 0xxx */
	"\tfistcw-xxx",			/* 0xdf xxx11 1xxx */

};

void
fphand(j)

   register int j;            /* Pointer to optab[] entry   */

{/* * * * * * * * * *  START OF fphand()  * * * * * * * * * */

	register int k, op;
	char *a;

   segflg = 0;

   FETCH(k);

   a = mtrans((j & 0xfd),(k & 0xc7),TR_STD);

   mtrunc(a);

   op = (j - 0xd8) << 3 |
       (k & 0x38) >> 3;

   printf("%s\t%s\n",fptab[op],a);

}/* * * * * * * * * * * END OF fphand() * * * * * * * * * * */

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  *                                                         *
  * This is the  handler for  variable  software  interrupt *
  * opcodes.  It is included in this file because PC/IX im- *
  * plements its software floating-point emulation by means *
  * of interrupts.  Any interrupt in the range 0xd8 through *
  * 0xdf is an  NDP-emulation  interrupt,  and is specially *
  * handled by the assembler.                               *
  *                                                         *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
inhand(j)

   register int j;            /* Pointer to optab[] entry   */

{/* * * * * * * * * *  START OF inhand()  * * * * * * * * * */

   register int k;

   objini(j);

   FETCH(k);

#ifndef VENIX
   if (ISFLOP(k))
      {
      fphand(k);
      return;
      }
#endif

   printf("%s\t$%02x\n",optab[j].text,k);

   objout();

}/* * * * * * * * * * * END OF inhand() * * * * * * * * * * */


