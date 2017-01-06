#ifndef __CRYPTOMD4_H__
#define __CRYPTOMD4_H__

/* MD4 context. */
typedef struct  _MD4_CTX_   {
    ULONG   state[4];        /* state (ABCD) */
    ULONG   count[2];        /* number of bits, modulo 2^64 (lsb first) */
    UCHAR   buffer[64];      /* input buffer */
}   MD4_CTX;

VOID MD4Init (MD4_CTX *);
VOID MD4Update (MD4_CTX *, PUCHAR, UINT);
VOID MD4Final (UCHAR [16], MD4_CTX *);

#endif //__MD4_H__