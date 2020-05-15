/*
 * stream filter to change 8 bit bytes into printable ascii
 * computes the number of bytes, and three kinds of simple checksums
 * assumes that int is 32 bits
 * incoming bytes are collected into 32-bit words, then printed in base 85
 *  exp(85,5) > exp(2,32)
 * the ASCII characters used are between ' ' and 't'
 * 'z' encodes 32-bit zero; 'x' is used to mark the end of encoded data.
 *
 *  Paul Rutter
 *  philabs!per
 */

#include <stdio.h>

#define reg register

#define MAXPERLINE 78

int Ceor = 0;
int Csum = 0;
int Crot = 0;

int ccount = 0;
int bcount = 0;
int word;

#define EN(c) ((c) + ' ')

encode(c) reg c;
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

  word <<= 8;
  word |= c;
  if (bcount == 3) {
    wordout(word);
    bcount = 0;
  }
  else {
    bcount += 1;
  }
}

wordout(word) reg word;
{
  if (word == 0) {
    charout('z');
  }
  else {
    /*first division must be unsigned*/;
    charout((int) EN((unsigned) word / (unsigned)(85 * 85 * 85 * 85)));
    word = (unsigned) word % (unsigned)(85 * 85 * 85 * 85);
    charout(EN(word / (85 * 85 * 85)));
    word %= (85 * 85 * 85);
    charout(EN(word / (85 * 85)));
    word %= (85 * 85);
    charout(EN(word / 85));
    word %= 85;
    charout(EN(word));
  }
}

charout(c) {
  putchar(c);
  ccount += 1;
  if (ccount == MAXPERLINE) {
    putchar('\n');
    ccount = 0;

  }
}

main(argc,argv) char **argv;
{
  reg c, n;
  if (argc != 1) {
    fprintf(stderr,"bad args to %s\n", argv[0]);
    exit(2);
  }
  printf("xbtoa Begin\n");
  n = 0;
  while ((c = getchar()) != EOF) {
    encode(c);
    n += 1;
  }
  while (bcount != 0) {
    encode(0);
  }
  /* n is written twice as crude cross check*/
  printf("\nxbtoa End N %d %x E %x S %x R %x\n", n, n, Ceor, Csum, Crot);
}
