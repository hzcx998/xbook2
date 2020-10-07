/*
 ***************************************************************************
 *       Filename:  des.c
 *
 *    Description:  DES encryption and decryption implementation, with
 *                  DC(differential cryptanalysis) attacking to 3-round DES.
 *
 *        Version:  1.0
 *        Created:  10/25/2007 04:50:01 PM CST
 *       Revision:  none
 *       Compiler:  gcc 4.1.3
 *
 *       The DES de/encryption algorithm
 *         Author: Jim Gillogly, May 1977
 *       Modified: 8/84 by Jim Gillogly and Lauren Weinstein to compile with
 *                 post-1977 C compilers and systems
 *
 *       Modified and added DC attacking algorithm, 10/25/2007
 *         Author:  Wenbo Yang <http://solrex.cn>
 *        Company:  the State Key Laboratory of Information Security
 *                  CAS, Beijing, China.
 *
 * This program is now officially in the public domain, and is available for
 * any non-profit use as long as the authorship line is retained.
 **************************************************************************
 */

/* Permutation algorithm:
 *	The permutation is defined by its effect on each of the 16 nibbles
 *	of the 64-bit input.  For each nibble we give an 8-byte bit array
 *	that has the bits in the input nibble distributed correctly.  The
 *	complete permutation involves ORing the 16 sets of 8 bytes designated
 *	by the 16 input nibbles.  Uses 16*16*8 = 2K bytes of storage for
 *	each 64-bit permutation.  32-bit permutations (P) and expansion (E)
 *	are done similarly, but using bytes instead of nibbles.
 *	Should be able to use long ints, adding the masks, at a
 *	later pass.  Tradeoff: can speed 64-bit perms up at cost of slowing 
 *	down expansion or contraction operations by using 8K tables here and
 *	decreasing the size of the other tables.
 * The compressions are pre-computed in 12-bit chunks, combining 2 of the
 *	6->4 bit compressions.
 * The key schedule is also precomputed. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // read

/* structure to store plaintext and ciphertext blocks in pair(DC) */
struct text_pair{
  unsigned char first[8];
  unsigned char second[8];
};
/* structure to store S box input in pair(DC) */
struct sin_pair{
  unsigned char first[6];
  unsigned char second[6];
};

/* structure to store S box output in pair(DC) */
struct sout_pair{
  unsigned char first[4];
  unsigned char second[4];
};

/* Some useful macros used as function arguments(DC) */
enum {
  NO_IP = 0,
  HAVE_IP = 1,
  NO_LAST_SWAP = 0,
  LAST_SWAP = 1,
  BIN_M = 0,
  HEX_M = 1,
  DEC_M = 2,
};

/* identical, final and  inital permutations */
unsigned char Iperm[16][16][8];
unsigned char iperm[16][16][8];
unsigned char fperm[16][16][8]; 

unsigned char s[4][4096];      /* S1 thru S8 precomputed */
unsigned char p32[4][256][4];  /* for permuting 32-bit f output */
unsigned char rp32[4][256][4]; /* for reverse permuting 32-bit f output */
unsigned char kn[16][6];       /* key selections */
unsigned char J[8][64];        /* J matrix to determine key(DC) */

/* Const string for printing binary numers. */
static const unsigned char *bin[16] = { 
  "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
  "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};

static const unsigned char I[64] = {   /* identical permutation P */
   1,  2,  3,  4,  5,  6,  7,  8,
   9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32,
  33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48,
  49, 50, 51, 52, 53, 54, 55, 56,
  57, 58, 59, 60, 61, 62, 63, 64 };

static const unsigned char ip[64] = {  /* initial permutation P */
  58, 50, 42, 34, 26, 18, 10,  2,
  60, 52, 44, 36, 28, 20, 12,  4,
  62, 54, 46, 38, 30, 22, 14,  6,
  64, 56, 48, 40, 32, 24, 16,  8,
  57, 49, 41, 33, 25, 17,  9,  1,
  59, 51, 43, 35, 27, 19, 11,  3,
  61, 53, 45, 37, 29, 21, 13,  5,
  63, 55, 47, 39, 31, 23, 15,  7 };

static const unsigned char fp[64] = {  /* final permutation F */
  40,  8, 48, 16, 56, 24, 64, 32,
  39,  7, 47, 15, 55, 23, 63, 31,
  38,  6, 46, 14, 54, 22, 62, 30,
  37,  5, 45, 13, 53, 21, 61, 29,
  36,  4, 44, 12, 52, 20, 60, 28,
  35,  3, 43, 11, 51, 19, 59, 27,
  34,  2, 42, 10, 50, 18, 58, 26,
  33,  1, 41,  9, 49, 17, 57, 25 };

static const unsigned char pc1[56] = { /* PC-1 (key) */
  57, 49, 41, 33, 25, 17,  9,
   1, 58, 50, 42, 34, 26, 18,
  10,  2, 59, 51, 43, 35, 27,
  19, 11,  3, 60, 52, 44, 36,

  63, 55, 47, 39, 31, 23, 15,
   7, 62, 54, 46, 38, 30, 22,
  14,  6, 61, 53, 45, 37, 29,
  21, 13,  5, 28, 20, 12,  4 };

static const unsigned char rpc1[64] = {/* reverse PC-1 (key, DC) */
   8, 16, 24, 56, 52, 44, 36,  0,
   7, 15, 23, 55, 51, 43, 35,  0,
   6, 14, 22, 54, 50, 42, 34,  0,
   5, 13, 21, 53, 49, 41, 33,  0,
   4, 12, 20, 28, 48, 40, 32,  0,
   3, 11, 19, 27, 47, 39, 31,  0,
   2, 10, 18, 26, 46, 38, 30,  0,
   1, 9,  17, 25, 45, 37, 29,  0 };

static const unsigned char totrot[16] = {/* number left rotations of PC-1 */
  1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28 };

static unsigned char pc1m[56];     /* place to modify pc1 into */
static unsigned char pcr[56];      /* place to rotate pc1 into */

static const unsigned char pc2[48] = { /* PC-2 (key)  */
  14, 17, 11, 24,  1,  5,
   3, 28, 15,  6, 21, 10,
  23, 19, 12,  4, 26,  8,
  16,  7, 27, 20, 13,  2,
  41, 52, 31, 37, 47, 55,
  30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53,
  46, 42, 50, 36, 29, 32 };

static const unsigned char rpc2[56] = { /* reverse PC-2 (key, DC) */
   5, 24,  7, 16,  6, 10, 20, 18,
   0, 12,  3, 15, 23,  1,  9, 19,
   2,  0, 14, 22, 11,  0, 13,  4,
   0, 17, 21,  8, 47, 31, 27, 48,
  35, 41,  0, 46, 28,  0, 39, 32,
  25, 44,  0, 37, 34, 43, 29, 36,
  38, 45, 33, 26, 42,  0, 30, 40 };

static const unsigned char si[8][64] = {   /* 48->32 bit compression tables*/
          /* S[1]       */
  14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
   0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
   4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
  15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
          /* S[2]       */
  15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
   3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
   0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
  13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
          /* S[3]       */
  10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
  13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
  13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
   1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
          /* S[4]       */
   7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
  13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
  10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
   3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
          /* S[5]       */
   2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
  14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
   4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
  11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
          /* S[6]       */
  12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
  10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
   9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
   4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
          /* S[7]       */
   4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
  13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
   1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
   6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
          /* S[8]       */
  13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
   1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
   7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
   2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11  };

static const unsigned char p32i[32] = { /* P in F function */
  16,  7, 20, 21,
  29, 12, 28, 17,
   1, 15, 23, 26,
   5, 18, 31, 10,
   2,  8, 24, 14,
  32, 27,  3,  9,
  19, 13, 30,  6,
  22, 11,  4, 25 };

static const unsigned char rp32i[32] = {   /* reverse P in F(DC) */
   9, 17, 23, 31,
  13, 28,  2, 18,
  24, 16, 30,  6,
  26, 20, 10,  1,
   8, 14, 25,  3,
   4, 29, 11, 19,
  32, 12, 22,  7,
   5, 27, 15, 21 };

static int bytebit[8] = {      /* bit 0 is left-most in byte */
  0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1 };

static int nibblebit[4] = { 0x8,0x4,0x2,0x1 };

/* permute inblock with perm, result into outblock,64 bits */
void permute(unsigned char *inblock,
             unsigned char perm[16][16][8],/* 2K bytes defining perm.  */
             unsigned char *outblock)
{
  register int i,j;
  register char *ib, *ob, *p, *q;

  for(i=0, ob=outblock; i<8; i++) *ob++ = 0;   /* clear output block    */
  ib = inblock;
  for (j = 0; j < 16; j += 2, ib++) {  /* for each input nibble  */
    ob = outblock;
    p = perm[j][(*ib >> 4) & 0xf];
    q = perm[j + 1][*ib & 0xf];
    for (i = 0; i < 8; i++)    /* and each output byte  */
      *ob++ |= *p++ | *q++;    /* OR the masks together*/
  }
}

/* 32-bit permutation at end of the f crypto function*/
void perm32(unsigned char inblock[4],
            unsigned char perm[4][256][4],
            unsigned char outblock[4])
{
  register int j;
  register char *ib, *ob, *q;

  ob = outblock;       /* clear output block */
  *ob++ = 0; *ob++ = 0; *ob++ = 0; *ob++ = 0;
  ib = inblock;        /* ptr to 1st byte of input */
  for (j=0; j<4; j++, ib++) {  /* for each input byte */
    q = perm[j][*ib & 0xff];
    ob = outblock;    /* and each output byte */
    *ob++ |= *q++;    /* OR the 16 masks together */
    *ob++ |= *q++;
    *ob++ |= *q++;
    *ob++ |= *q++;
  }
}

/* 32 to 48 bits with E oper, right is 32, bigright 48  */
void expand(unsigned char right[4], unsigned char bigright[6])
{
  register char *bb, *r, r0, r1, r2, r3;

  bb = bigright;
  r = right; r0 = *r++; r1 = *r++; r2 = *r++; r3 = *r++;
  *bb++ = ((r3 & 0x1) << 7) |  /* 32             */
    ((r0 & 0xf8) >> 1) |       /* 1 2 3 4 5      */
    ((r0 & 0x18) >> 3);        /* 4 5            */
  *bb++ = ((r0 & 0x7) << 5) |  /* 6 7 8          */
    ((r1 & 0x80) >> 3) |       /* 9              */
    ((r0 & 0x1) << 3) |        /* 8              */
    ((r1 & 0xe0) >> 5);        /* 9 10 11        */
  *bb++ = ((r1 & 0x18) << 3) | /* 12 13          */
    ((r1 & 0x1f) << 1) |       /* 12 13 14 15 16 */
    ((r2 & 0x80) >> 7);        /* 17             */
  *bb++ = ((r1 & 0x1) << 7) |  /* 16             */
    ((r2 & 0xf8) >> 1) |       /* 17 18 19 20 21 */
    ((r2 & 0x18) >> 3);        /* 20 21          */
  *bb++ = ((r2 & 0x7) << 5) |  /* 22 23 24       */
    ((r3 & 0x80) >> 3) |       /* 25             */
    ((r2 & 0x1) << 3) |        /* 24             */
    ((r3 & 0xe0) >> 5);        /* 25 26 27       */
  *bb++ = ((r3 & 0x18) << 3) | /* 28 29          */
    ((r3 & 0x1f) << 1) |       /* 28 29 30 31 32 */
    ((r0 & 0x80) >> 7);        /* 1              */
}

/* contract f from 48 to 32 bits, using 12-bit pieces into bytes */
void contract(unsigned char in48[6], unsigned char out32[4])  
{  
  register char *c;
  register char *i;
  register int i0, i1, i2, i3, i4, i5;

  i = in48;
  i0 = *i++; i1 = *i++; i2 = *i++; i3 = *i++; i4 = *i++; i5 = *i++;
  c = out32;      /* do output a byte at a time   */
  *c++ = s[0][0xfff & ((i0 << 4) | ((i1 >> 4) & 0xf ))];
  *c++ = s[1][0xfff & ((i1 << 8) | ( i2  & 0xff ))];
  *c++ = s[2][0xfff & ((i3 << 4) | ((i4 >> 4) & 0xf ))];
  *c++ = s[3][0xfff & ((i4 << 8) | ( i5  & 0xff ))];
}

/* critical cryptographic trans, num: index number of this iter */
void f(unsigned char right[4],int num, unsigned char fret[4])      
{
  register char *kb, *rb, *bb;  /* ptr to key selection &c  */
  char bigright[6];    /* right expanded to 48 bits  */
  char result[6];      /* expand(R) XOR keyselect[num] */
  char preout[4];      /* result of 32-bit permutation */

  kb = kn[num];        /* fast version of iteration  */
  bb = bigright;
  rb = result;
  expand(right,bb);    /* expand to 48 bits    */
  *rb++ = *bb++ ^ *kb++;       /* expanded R XOR chunk of key  */
  *rb++ = *bb++ ^ *kb++;
  *rb++ = *bb++ ^ *kb++;
  *rb++ = *bb++ ^ *kb++;
  *rb++ = *bb++ ^ *kb++;
  *rb++ = *bb++ ^ *kb++;
  contract(result,preout);     /* use S fns to get 32 bits  */
  perm32(preout, p32, fret);   /* and do final 32-bit perm  */
}

/* 1 churning operation, num: i.e. the num-th one */
void iter(int num, unsigned char inblock[8], unsigned char outblock[8])  
{
  char fret[4];        /* return from f(R[i-1],key)  */
  register char *ib, *ob, *fb;

  ob = outblock; ib = &inblock[4];
  f(ib, num, fret);    /* the primary transformation   */
  *ob++ = *ib++;       /* L[i] = R[i-1]    */
  *ob++ = *ib++;
  *ob++ = *ib++;
  *ob++ = *ib++;
  ib = inblock; fb = fret;  /* R[i]=L[i] XOR f(R[i-1],key)  */
  *ob++ = *ib++ ^ *fb++;
  *ob++ = *ib++ ^ *fb++;
  *ob++ = *ib++ ^ *fb++;
  *ob++ = *ib++ ^ *fb++;
}

/* initialize a perm array, p: 64-bit, either init or final */
void perminit(unsigned char perm[16][16][8], const char p[64])
{
  register int l, j, k;
  int i,m;

  for (i=0; i<16; i++)         /* each input nibble position */
    for (j=0; j<16; j++)       /* all possible input nibbles */
      for (k=0; k<8; k++)      /* each byte of the mask */
        perm[i][j][k]=0;       /* clear permutation array */
  for (i=0; i<16; i++)         /* each input nibble position */
    for (j = 0; j < 16; j++)   /* each possible input nibble */
      for (k = 0; k < 64; k++){/* each output bit position  */
        l = p[k] - 1;          /* where does this bit come from*/
        if ((l >> 2) != i)     /* does it come from input posn?*/
          continue;            /* if not, bit k is 0 */
        if (!(j & nibblebit[l & 3]))
          continue;            /* any such bit in input? */
        m = k & 0x7;           /* which bit is this in the byte*/
        perm[i][j][k>>3] |= bytebit[m];
      }
}

/* 64 bits (will use only 56)   */
void kinit(unsigned char key[8])        /* initialize key schedule array*/
{
  register int i,j,l;
  int m;

  for (j=0; j<56; j++){        /* convert pc1 to bits of key   */
    l=pc1[j]-1;                /* integer bit location    */
    m = l & 07;                /* find bit      */
  /* find which key byte l is in and which bit of that byte, and store 1-bit
   * result */
    pc1m[j]=(key[l>>3] & bytebit[m]) ? 1 : 0;
  }
  for (i=0; i<16; i++)         /* for each key sched section */
    for (j=0; j<6; j++)        /* and each byte of the kn */
      kn[i][j]=0;              /* clear it for accumulation */
  for (i=0; i<16; i++){        /* key chunk for each iteration */
    for (j=0; j<56; j++)       /* rotate pc1 the right amount */
      /* rotate left and right halves independently */
      pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];
    for (j=0; j<48; j++)       /* select bits individually  */
      if (pcr[pc2[j]-1]){      /* check bit that goes to kn[j] */
        l= j & 0x7;
        kn[i][j>>3] |= bytebit[l]; /* mask it in if it's there */
      }
  }
}

/* 1 compression value for sinit*/
int getcomp(int k,int v)        
{
  register int i,j;            /* correspond to i and j in FIPS */

  i=((v&0x20)>>4)|(v&1);       /* first and last bits make row */
  j=(v&0x1f)>>1;               /* middle 4 bits are column  */
  return (int) si[k][(i<<4)+j];/* result is ith row, jth col */
}

/* initialize s1-s8 arrays    */
void sinit()         
{
  register int i,j;

  for (i=0; i<4; i++)       /* each 12-bit position */
    for (j=0; j<4096; j++)  /* each possible 12-bit value */
      /* store 2 compressions per char*/
      s[i][j]=(getcomp(i*2,j>>6)<<4) | (0xf&getcomp(i*2+1,j&0x3f));
}

/* initialize 32-bit permutation */
void p32init(unsigned char perm[4][256][4], const unsigned char p[32])
{  
  register int l, j, k;
  int i,m;

  for (i=0; i<4; i++)      /* each input byte position */
    for (j=0; j<256; j++)  /* all possible input bytes */
      for (k=0; k<4; k++)  /* each byte of the mask */
        perm[i][j][k]=0;   /* clear permutation array */
  for (i=0; i<4; i++)      /* each input byte position */
    for (j=0; j<256; j++)  /* each possible input byte */
      for (k=0; k<32; k++){    /* each output bit position */
        l=p[k]-1;              /* invert this bit (0-31) */
        if ((l>>3)!=i)         /* does it come from input posn?*/
          continue;            /* if not, bit k is 0 */
        if (!(j&bytebit[l&0x7]))
          continue;            /* any such bit in input? */
        m = k & 0x7;           /* which bit is it? */
        perm[i][j][k>>3] |= bytebit[m];
      }
}

/* initialize all des arrays  */
void desinit(unsigned char *key)        
{
  perminit(Iperm,I);       /* identical permutation */
  perminit(iperm,ip);      /* initial permutation */
  perminit(fperm,fp);      /* final permutation */
  kinit(key);              /* key schedule */
  sinit();                 /* compression functions */
  p32init(p32, p32i);      /* 32-bit permutation in f */
  p32init(rp32, rp32i);    /* reverse 32-bit permutation in f */
}

/* encrypt 64-bit inblock  */
void endes(unsigned char *inblock, unsigned char *outblock,
           int round, int have_ip, int last_swap)
{
  char iters[17][8];       /* workspace for each iteration */
  char swap[8];            /* place to interchange L and R */
  register int i;
  register char *s;

  if(have_ip == HAVE_IP)
    permute(inblock,iperm,iters[0]);   /* apply initial permutation */
  else
    permute(inblock,Iperm,iters[0]);   /* don't apply initial permutation */

  /* don't re-copy to save space  */
  for (i=0; i<3; i++)      /* 16 churning operations  */
    iter(i,iters[i],iters[i+1]);

  s = swap;
  if(last_swap == LAST_SWAP) {              /* do last swap left and right */
    /* interchange left and right. */
    for(i=4;i<8;i++) *s++ = iters[3][i];  
    for(i=0;i<4;i++) *s++ = iters[3][i];  
  } else {                     /* don't do last swap left and right */
    for(i=0;i<8;i++) *s++ = iters[3][i];  
  }

  if(have_ip == HAVE_IP)
    permute(swap,fperm,outblock);   /* apply final permutation  */
  else
    permute(swap,Iperm,outblock);   /* don't apply final permutation  */
}

/* decrypt 64-bit inblock  */
void dedes(unsigned char *inblock, unsigned char *outblock)      
{
  char iters[17][8];    /* workspace for each iteration */
  char swap[8];        /* place to interchange L and R */
  register int i;
  register char *s, *t;

  permute(inblock,iperm,iters[0]); /* apply initial permutation  */
  for (i=0; i<16; i++)             /* 16 churning operations  */
    iter(15-i,iters[i],iters[i+1]);
  /* reverse order from encrypting */
  s = swap; t = &iters[16][4];     /* interchange left    */
  *s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
  t = &iters[16][0];               /* and right      */
  *s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
  permute(swap,fperm,outblock);    /* apply final permutation  */
}

/* End of DES algorithm */

void print_array(unsigned char *in, int length, int mode)
{
  int i;
  if(mode == BIN_M){ /* print array in binary */
    for (i=0;i<length;i++) {
      if (i%8==0) {
        if (i!=0) printf("\n");
        printf("\t");
      }
      printf("%s%s ", bin[in[i]>>4], bin[in[i]&0xf]);
    }
  } else if(mode == HEX_M){    /* print array in hex */
    for (i=0;i<length;i++) {
      if (i%8==0) {
        if (i!=0) printf("\n");
        printf("\t");
      }
      printf("%02x", in[i]);
    }
  } else if(mode == DEC_M){    /* print array in decimal */
    for (i=0;i<length;i++) {
      if (i%16==0) {
        if (i!=0) printf("\n");
        printf("\t");
      }
      printf("%d ", in[i]);
    }
  }
 printf("\n");

}
typedef unsigned long ULONG;

void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len) //初始化函数
{
    int i =0, j = 0;
    char k[256] = {0};
    unsigned char tmp = 0;
    for (i=0;i<256;i++) {
        s[i] = i;
        k[i] = key[i%Len];
    }
    for (i=0; i<256; i++) {
        j=(j+s[i]+k[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[i]和s[j]
        s[j] = tmp;
    }
 }

void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len) //加解密
{
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;
    for(k=0;k<Len;k++) {
        i=(i+1)%256;
        j=(j+s[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[x]和s[y]
        s[j] = tmp;
        t=(s[i]+s[j])%256;
        Data[k] ^= s[t];
     }
} 
void write_array(unsigned char *in, int length)
{
  int i;
      FILE *fp = NULL;
  fp=fopen("./out","a+");
    for (i=0;i<length;i++) {
      fprintf(fp,"%02x", in[i]);
    }
 fprintf(fp,"\n");
fclose(fp);

}

void print_des(unsigned char *in, unsigned char *out, int mode)
{
  printf("\tP: "); print_array(in, 8, mode);
  printf("\tC: "); print_array(out, 8, mode);
}

/* End of helper functions */

/* Test the J matrix, here we calculate the value with whole 8 S boxes,
 * not one by one. */
void test_j(unsigned char sindiff[6], unsigned char soutdiff[4],
            unsigned char sin[6])
{
  register int i,j;
  unsigned char in[2][6], out[2][4], outdiff[4];
  unsigned char result[4];
  unsigned char inS[8];

  /* get single S box's input from 48bits array to 8 char. */
  for(i=0;i<2;i++) {           /* inS[0] store input to S1, etc. */ 
    inS[4*i] = (sin[3*i]>>2) & 0x3f;
    inS[4*i+1] = ((sin[3*i]<<4) | (sin[3*i+1]>>4)) & 0x3f;
    inS[4*i+2] = ((sin[3*i+1]<<2) | (sin[3*i+2]>>6)) & 0x3f;
    inS[4*i+3] = sin[3*i+2] & 0x3f; 
  }
  for(i=0;i<64;i++) {          /* test input from 0 to 63 to every S box */
    for(j=0;j<6;j++)           /* clean input array(48bits) to store */
      in[0][j] = 0;            /* first part of a difference pair */
    for(j=0;j<2;j++) {         /* store i(6 bits) to items in input array */
      in[0][3*j] |= (i << 2) & 0xfc;
      in[0][3*j] |= (i >> 4) & 0x03;
      in[0][3*j+1] |= (i << 4) & 0xf0;
      in[0][3*j+1] |= (i >> 2) & 0x0f;
      in[0][3*j+2] |= (i << 6) & 0xc0;
      in[0][3*j+2] |= i & 0x3f;
    }
    for(j=0;j<6;j++)           /* XORed 1st part to get 2nd part in pair */
      in[1][j] = in[0][j] ^ sindiff[j];
    contract(in[0], out[0]);   /* get 1st part's S box output */
    contract(in[1], out[1]);   /* get 2nd part's S box output */
    for(j=0;j<4;j++) {
      /* calculate S box's output diff of pair */
      outdiff[j] = out[0][j] ^ out[1][j];
      /* XORed with the known outputdiff */
      result[j] = outdiff[j] ^ soutdiff[j];
      /* if Sx's outdiff equals soutdiff, the 4 bits in result array should 
       * be 0, so Jx array correspoding counter++. */
      if(((result[j]>>4)&0x0f) == 0) J[2*j][i^inS[2*j]]++;
      if((result[j]&0x0f) == 0) J[2*j+1][i^inS[2*j+1]]++;
    }
  }
}



/* End of DC algorithm */

int main(int argc, char *argv[])
{
  /* key for encryption */
  unsigned char key[8];/* = {
    0x4f, 0x73, 0x7a, 0x54, 0x67, 0x32, 0x54, 0x25
  };*/
  /* Chosen plaintext pairs. */
  struct text_pair in[3]= {{{0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x5f, 0x30, 0x7e},
      {0x63, 0x34, 0x4e, 0x79, 0x6f, 0x5f, 0x30, 0x7e}
    }, {
      {0x67, 0x33, 0x74, 0x6d, 0x79, 0x79, 0x79, 0x79},
      {0x4b, 0x45, 0x59, 0x6b, 0x79,0x79, 0x79, 0x79},
    }, {
      {0x74, 0x72, 0x79, 0x5f, 0x31, 0x74, 0x5e, 0x5e},
      {0x67, 0x33, 0x74, 0x21, 0x31, 0x74, 0x5e, 0x5e}
    }
  };
  struct text_pair out[3]; /* Store ciphertext. */
  printf("CAN YOU GUESS MY KEY?\n");
  read(0,key,9);
  int i,j;
  desinit(key);    /* set up tables for DES  */


  for(i=0;i<3;i++){ /* encryption */
    endes(in[i].first, out[i].first, 8, NO_IP, NO_LAST_SWAP);
    endes(in[i].second, out[i].second, 8, NO_IP, NO_LAST_SWAP);
    write_array(out[i].first,8);
    write_array(out[i].second, 8);
  }
  printf("if you have my key,you will have my flag!^^\n");
   unsigned char s[256] = {0}; //S-box
    char pData[512];
    read(0,pData,32);
    ULONG len = 8;
    
    rc4_init(s,key,8); //已经完成了初始化
    /*
    int i;
    for (i=0; i<256; i++)
    {
        printf("%-3d ",s[i]);
    }
    printf("\n");
    */
    rc4_crypt(s,(unsigned char *)pData,32);//加密
 int final[32]={0x85,0x3c,0xb1,0x81,0x3d,0x85,0x95,0x7a,0xf6,0x68,0xd8,0xfd,0x9f,0xdc,0xcd,0xd7,0x73,0x18,0x97,0x32,0xf1,0x50,0xe3,0xd8,0x07,0x79,0x01,0x4a,0x45,0xea,0x6e,0x42
};
    for(int cur=0;cur<32;++cur)
{
  if ((pData[cur]&0xff)!=final[cur])
{
   printf("try again!\n");
   // exit(0);
}
}
  printf("yes!you have my flag!");
  return 0;
}

     
  
