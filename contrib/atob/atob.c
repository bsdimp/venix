/*
 * stream filter to change printable ascii from "btoa" back into 8 bit bytes
 * if bad chars, or Csums do not match: exit(1) [and NO output]
 * assumes that int is 32 bits
 *
 *  Paul Rutter
 *  philabs!per
 */

#include <stdio.h>

#define reg register

#define streq(s0, s1)  strcmp(s0, s1) == 0

int Ceor = 0;
int Csum = 0;
int Crot = 0;
int bcount = 0;
int word = 0;

fatal() {
  fprintf(stderr, "bad format or Csum to atob\n");
  exit(1);
}

#define DE(c) ((c) - ' ')

decode(c) reg c;
{
  if (c == 'z') {
    if (bcount != 0) {
      fatal();
    }
    else {
      byteout(0);
      byteout(0);
      byteout(0);
      byteout(0);
    }
  }
  else if ((c >= ' ') && (c < (' ' + 85))) {
    if (bcount == 0) {
      word = DE(c);
      ++bcount;
    }
    else if (bcount < 4) {
      word *= 85;
      word += DE(c);
      ++bcount;
    }
    else {
      word = ((unsigned) word * (unsigned) 85) + DE(c);
      byteout((word >> 24) & 255);
      byteout((word >> 16) & 255);
      byteout((word >> 8) & 255);
      byteout(word & 255);
      word = 0;
      bcount = 0;
    }
  }
  else {
    fatal();
  }
}

FILE *tmpfile;

byteout(c) reg c;
{
  Ceor ^= c;
  Csum += c;
  Csum += 1;
  if ((Crot & 0x80000000)) {
    Crot <<= 1;
    Crot += 1;
  }
  else {
    Crot <<= 1;
  }
  Crot += c;
  putc(c, tmpfile);
}

main(argc, argv) char **argv;
{
  reg c, i;
  char tmpname[100];
  char buf[100];
  int n1, n2, oeor, osum, orot;
  if (argc != 1) {
    fprintf(stderr,"bad args to %s\n", argv[0]);
    exit(2);
  }
  sprintf(tmpname, "/usr/tmp/atob.%x", getpid());
  tmpfile = fopen(tmpname, "w+");
  if (tmpfile == NULL) {
    fatal();
  }
  /*search for header line*/
  for (;;) {
    if (fgets(buf, sizeof buf, stdin) == NULL) {
      fatal();
    }
    if (streq(buf, "xbtoa Begin\n")) {
      break;
    }
  }

  while ((c = getchar()) != EOF) {
    if (c == '\n') {
      continue;
    }
    else if (c == 'x') {
      break;
    }
    else {
      decode(c);

    }
  }
  if (scanf("btoa End N %d %x E %x S %x R %x\n", &n1, &n2, &oeor, &osum, &orot) != 5) {
    fatal();
  }
  if ((n1 != n2) || (oeor != Ceor) || (osum != Csum) || (orot != Crot)) {
    fatal();
  }
  else {
    /*copy OK tmp file to stdout*/;
    fseek(tmpfile, 0, 0);
    for (i = n1; --i >= 0;) {
      putchar(getc(tmpfile));
    }
    unlink(tmpname);
  }
}
