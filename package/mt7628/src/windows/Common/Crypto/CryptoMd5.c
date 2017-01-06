/* This MD5 code is based on code from Dynamics -- HUT Mobile IP
 * Copyright (C) 1998-2001, Dynamics group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.

    Module Name:
    md5.c

    Abstract:
        contain MD5 and AES cipher algorithm
        
    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    jan     10-28-03        Initial
 */
#include "MtConfig.h"

#define SHR(x,n) (x >> n) /* SHR(x)^n, right shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTR(x,n,w) ((x >> n) | (x << (w - n))) /* ROTR(x)^n, circular right shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTL(x,n,w) ((x << n) | (x >> (w - n))) /* ROTL(x)^n, circular left shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTR32(x,n) ROTR(x,n,32) /* 32 bits word */
#define ROTL32(x,n) ROTL(x,n,32) /* 32 bits word */ 

/* Basic functions */
#define Ch(x,y,z) ((x & y) ^ ((~x) & z))
#define Maj(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define Parity(x,y,z) (x ^ y ^ z)

/* SHA256 functions */
#define Zsigma_256_0(x) (ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22))
#define Zsigma_256_1(x) (ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25))
#define Sigma_256_0(x)  (ROTR32(x,7) ^ ROTR32(x,18) ^ SHR(x,3))
#define Sigma_256_1(x)  (ROTR32(x,17) ^ ROTR32(x,19) ^ SHR(x,10))
/* SHA256 constants */
static const ULONG SHA256_K[64] = {
    0x428a2f98, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 
    0x3956c25b, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL, 
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL, 
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL, 
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL, 
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL, 
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL, 
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL, 
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL, 
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
static const UINT32 SHA256_DefaultHashValue[8] = {
    0x6a09e667UL, 0xbb67ae85UL, 0x3c6ef372UL, 0xa54ff53aUL,
    0x510e527fUL, 0x9b05688cUL, 0x1f83d9abUL, 0x5be0cd19UL
};
/**
 * md5_mac:
 * @key: pointer to the key used for MAC generation
 * @key_len: length of the key in bytes
 * @data: pointer to the data area for which the MAC is generated
 * @data_len: length of the data in bytes
 * @mac: pointer to the buffer holding space for the MAC; the buffer should
 * have space for 128-bit (16 bytes) MD5 hash value
 *
 * md5_mac() determines the message authentication code by using secure hash
 * MD5(key | data | key).
 */
void md5_mac(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac)
{
    MD5_CTX context;

    MD5Init(&context);
    MD5Update(&context, key, (unsigned long)key_len);
    MD5Update(&context, data, (unsigned long)data_len);
    MD5Update(&context, key, (unsigned long)key_len);
    MD5Final(mac, &context);
}

/**
 * hmac_md5:
 * @key: pointer to the key used for MAC generation
 * @key_len: length of the key in bytes
 * @data: pointer to the data area for which the MAC is generated
 * @data_len: length of the data in bytes
 * @mac: pointer to the buffer holding space for the MAC; the buffer should
 * have space for 128-bit (16 bytes) MD5 hash value
 *
 * hmac_md5() determines the message authentication code using HMAC-MD5.
 * This implementation is based on the sample code presented in RFC 2104.
 */
void hmac_md5(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac)
{
    MD5_CTX context;
    u8 k_ipad[65]; /* inner padding - key XORd with ipad */
    u8 k_opad[65]; /* outer padding - key XORd with opad */
    u8 tk[16];
    int i;

    //assert(key != NULL && data != NULL && mac != NULL);

    /* if key is longer than 64 bytes reset it to key = MD5(key) */
    if (key_len > 64) {
        MD5_CTX ttcontext;

        MD5Init(&ttcontext);
        MD5Update(&ttcontext, key, (unsigned long)key_len);
        MD5Final(tk, &ttcontext);
        //key=(PUCHAR)ttcontext.buf;
        key = tk;
        key_len = 16;
    }

    /* the HMAC_MD5 transform looks like:
     *
     * MD5(K XOR opad, MD5(K XOR ipad, text))
     *
     * where K is an n byte key
     * ipad is the byte 0x36 repeated 64 times
     * opad is the byte 0x5c repeated 64 times
     * and text is the data being protected */

    /* start out by storing key in pads */
    PlatformZeroMemory(k_ipad, sizeof(k_ipad));
    PlatformZeroMemory(k_opad,  sizeof(k_opad));
    //assert(key_len < sizeof(k_ipad));
    PlatformMoveMemory(k_ipad, key, (ULONG)key_len);
    PlatformMoveMemory(k_opad, key, (ULONG)key_len);

    /* XOR key with ipad and opad values */
    for (i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    /* perform inner MD5 */
    MD5Init(&context);                   /* init context for 1st pass */
    MD5Update(&context, k_ipad, 64);     /* start with inner pad */
    MD5Update(&context, data, (unsigned long) data_len); /* then text of datagram */
    MD5Final(mac, &context);             /* finish up 1st pass */

    /* perform outer MD5 */
    MD5Init(&context);                   /* init context for 2nd pass */
    MD5Update(&context, k_opad, 64);     /* start with outer pad */
    MD5Update(&context, mac, 16);        /* then results of 1st hash */
    MD5Final(mac, &context);             /* finish up 2nd pass */
}


/* ===== start - public domain MD5 implementation ===== */
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

//#ifndef WORDS_BIGENDIAN
#if 1
#define byteReverse(buf, len)   /* Nothing */
#else
void byteReverse(unsigned char *buf, unsigned longs);

//#ifndef ASM_MD5
#if 1
/*
 * Note: this code is harmless on little-endian machines.
 */
void byteReverse(unsigned char *buf, unsigned longs)
{
    u32 t;
        DBGPRINT(RT_DEBUG_TRACE, ("bbjanbb byte reversee\n"));

    do {
        t = (u32) ((unsigned) buf[3] << 8 | buf[2]) << 16 | ((unsigned) buf[1] << 8 | buf[0]);
        *(u32 *) buf = t;
        buf += 4;
    } while (--longs);
}
#endif
#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Update(struct MD5Context *ctx, unsigned char *buf, unsigned long len)
{
    u32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((u32) len << 3)) < t)
        ctx->bits[1]++;     /* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if (len < t) {
            PlatformMoveMemory(p, buf, len);
            return;
        }
        PlatformMoveMemory(p, buf, t);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += t;
        len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
        PlatformMoveMemory(ctx->in, buf, 64);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */

    PlatformMoveMemory(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        PlatformZeroMemory(p, count);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);

        /* Now fill the next block with 56 bytes */
        PlatformZeroMemory(ctx->in, 56);
    } else {
        /* Pad block to 56 bytes */
        PlatformZeroMemory(p,  count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((u32 *) ctx->in)[14] = ctx->bits[0];
    ((u32 *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (u32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    PlatformMoveMemory(digest, ctx->buf, 16);
    PlatformZeroMemory(ctx,  sizeof(ctx));  /* In case it's sensitive */
}

//#ifndef ASM_MD5
#if 1

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
    ( w += f(x, y, z) + data,  w =( w<<s | w>>(32-s))&0xffffffff,  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Transform(u32 buf[4], u32 in[16])
{
    register u32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
#endif

void SHAInit(SHA_CTX *ctx) {
    int i;
 
    ctx->lenW = 0;
    ctx->sizeHi = ctx->sizeLo = 0;
 
    /* Initialize H with the magic constants (see FIPS180 for constants)
     */
    ctx->H[0] = 0x67452301L;
    ctx->H[1] = 0xefcdab89L;
    ctx->H[2] = 0x98badcfeL;
    ctx->H[3] = 0x10325476L;
    ctx->H[4] = 0xc3d2e1f0L;
 
    for (i = 0; i < 80; i++)
        ctx->W[i] = 0;
 }

#define SHA_ROTL(X,n) ((((X) << (n)) | ((X) >> (32-(n)))) & 0xffffffffL)
 
static void SHAHashBlock(SHA_CTX *ctx) {
    int t;
    unsigned long A,B,C,D,E,TEMP;
 
    for (t = 16; t <= 79; t++)
        ctx->W[t] = SHA_ROTL(ctx->W[t-3] ^ ctx->W[t-8] ^ ctx->W[t-14] ^ ctx->W[t-16], 1);
 
    A = ctx->H[0];
    B = ctx->H[1];
    C = ctx->H[2];
    D = ctx->H[3];
    E = ctx->H[4];
 
    for (t = 0; t <= 19; t++) {
        TEMP = (SHA_ROTL(A,5) + (((C^D)&B)^D)     + E + ctx->W[t] + 0x5a827999L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
    }
    for (t = 20; t <= 39; t++) {
        TEMP = (SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0x6ed9eba1L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
    }
    for (t = 40; t <= 59; t++) {
        TEMP = (SHA_ROTL(A,5) + ((B&C)|(D&(B|C))) + E + ctx->W[t] + 0x8f1bbcdcL) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
    }
    for (t = 60; t <= 79; t++) {
        TEMP = (SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0xca62c1d6L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
    }
 
    ctx->H[0] += A;
    ctx->H[1] += B;
    ctx->H[2] += C;
    ctx->H[3] += D;
    ctx->H[4] += E;
}

void SHAUpdate(SHA_CTX *ctx, unsigned char *dataIn, int len) 
{
    int i;
 
    /* Read the data into W and process blocks as they get full
     */
    for (i = 0; i < len; i++) {
        ctx->W[ctx->lenW / 4] <<= 8;
        ctx->W[ctx->lenW / 4] |= (unsigned long)dataIn[i];
        if ((++ctx->lenW) % 64 == 0) {
            SHAHashBlock(ctx);
            ctx->lenW = 0;
        }
        ctx->sizeLo += 8;
        ctx->sizeHi += (ctx->sizeLo < 8);
    }
}


void SHAFinal(SHA_CTX *ctx, unsigned char hashout[20]) {
    unsigned char pad0x80 = 0x80;
    unsigned char pad0x00 = 0x00;
    unsigned char padlen[8];
    int i;
 
    /* Pad with a binary 1 (e.g. 0x80), then zeroes, then length
     */
    padlen[0] = (unsigned char)((ctx->sizeHi >> 24) & 255);
    padlen[1] = (unsigned char)((ctx->sizeHi >> 16) & 255);
    padlen[2] = (unsigned char)((ctx->sizeHi >> 8) & 255);
    padlen[3] = (unsigned char)((ctx->sizeHi >> 0) & 255);
    padlen[4] = (unsigned char)((ctx->sizeLo >> 24) & 255);
    padlen[5] = (unsigned char)((ctx->sizeLo >> 16) & 255);
    padlen[6] = (unsigned char)((ctx->sizeLo >> 8) & 255);
    padlen[7] = (unsigned char)((ctx->sizeLo >> 0) & 255);
    SHAUpdate(ctx, &pad0x80, 1);
    while (ctx->lenW != 56)
        SHAUpdate(ctx, &pad0x00, 1);
    SHAUpdate(ctx, padlen, 8);
 
    /* Output hash
     */
    for (i = 0; i < 20; i++) {
        hashout[i] = (unsigned char)(ctx->H[i / 4] >> 24);
        ctx->H[i / 4] <<= 8;
    }
 
    /*
     *  Re-initialize the context (also zeroizes contents)
     */
    SHAInit(ctx); 
}

/*
========================================================================
Routine Description:
    Initial SHA256_CTX_STRUC

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID SHA256_Init (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    PlatformMoveMemory(pSHA_CTX->HashValue, (PVOID)SHA256_DefaultHashValue, 
        sizeof(SHA256_DefaultHashValue));
    PlatformZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->MessageLen = 0;
    pSHA_CTX->BlockLen   = 0;
} /* End of SHA256_Init */


/*
========================================================================
Routine Description:
    SHA256 computation for one block (512 bits)

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID SHA256_Hash (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    UINT32 W_i,t;
    UINT32 W[64];
    UINT32 a,b,c,d,e,f,g,h,T1,T2;
    
    /* Prepare the message schedule, {W_i}, 0 < t < 15 */
    PlatformMoveMemory(W, pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    for (W_i = 0; W_i < 16; W_i++)
        W[W_i] = cpu2be32(W[W_i]); /* Endian Swap */
        /* End of for */
 
    /* SHA256 hash computation */
    /* Initialize the working variables */
    a = pSHA_CTX->HashValue[0];
    b = pSHA_CTX->HashValue[1];
    c = pSHA_CTX->HashValue[2];
    d = pSHA_CTX->HashValue[3];
    e = pSHA_CTX->HashValue[4];
    f = pSHA_CTX->HashValue[5];
    g = pSHA_CTX->HashValue[6];
    h = pSHA_CTX->HashValue[7];

    /* 64 rounds */
    for (t = 0;t < 64;t++) {
        if (t > 15) /* Prepare the message schedule, {W_i}, 16 < t < 63 */
            W[t] = Sigma_256_1(W[t-2]) + W[t-7] + Sigma_256_0(W[t-15]) + W[t-16];
            /* End of if */
        T1 = h + Zsigma_256_1(e) + Ch(e,f,g) + SHA256_K[t] + W[t];
        T2 = Zsigma_256_0(a) + Maj(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
     } /* End of for */

     /* Compute the i^th intermediate hash value H^(i) */
     pSHA_CTX->HashValue[0] += a;
     pSHA_CTX->HashValue[1] += b;
     pSHA_CTX->HashValue[2] += c;
     pSHA_CTX->HashValue[3] += d;
     pSHA_CTX->HashValue[4] += e;
     pSHA_CTX->HashValue[5] += f;
     pSHA_CTX->HashValue[6] += g;
     pSHA_CTX->HashValue[7] += h;

    PlatformZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->BlockLen = 0;
} /* End of SHA256_Hash */


/*
========================================================================
Routine Description:
    The message is appended to block. If block size > 64 bytes, the SHA256_Hash 
will be called.

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC
    message     Message context
    messageLen  The length of message in bytes

Return Value:
    None

Note:
    None
========================================================================
*/
VOID SHA256_Append (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    IN  const UCHAR Message[], 
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    ULONG diffLen   = 0;
    
    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pSHA_CTX->BlockLen + diffLen) <  SHA256_BLOCK_SIZE) {
            PlatformMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                (PVOID)(Message + appendLen), diffLen);
            pSHA_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        } 
        else
        {
            PlatformMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                (PVOID)(Message + appendLen), SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            appendLen += (SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            pSHA_CTX->BlockLen = SHA256_BLOCK_SIZE;
            SHA256_Hash(pSHA_CTX);
        } /* End of if */
    } /* End of while */
    pSHA_CTX->MessageLen += MessageLen;
} /* End of SHA256_Append */


/*
========================================================================
Routine Description:
    1. Append bit 1 to end of the message
    2. Append the length of message in rightmost 64 bits
    3. Transform the Hash Value to digest message

Arguments:
    pSHA_CTX        Pointer to SHA256_CTX_STRUC

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID SHA256_End (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    OUT UCHAR DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    /* Append bit 1 to end of the message */
    pSHA_CTX->Block[pSHA_CTX->BlockLen] = 0x80;

    /* 55 = 64 - 8 - 1: append 1 bit(1 byte) and message length (8 bytes) */
    if (pSHA_CTX->BlockLen > 55)
        SHA256_Hash(pSHA_CTX);
        /* End of if */

    /* Append the length of message in rightmost 64 bits */
    message_length_bits = pSHA_CTX->MessageLen*8;
    message_length_bits = cpu2be64(message_length_bits);       
    PlatformMoveMemory(&pSHA_CTX->Block[56], &message_length_bits, 8);
    SHA256_Hash(pSHA_CTX);

    /* Return message digest, transform the UINT32 hash value to bytes */
    for (index = 0; index < 8;index++)
        pSHA_CTX->HashValue[index] = cpu2be32(pSHA_CTX->HashValue[index]);
        /* End of for */
    PlatformMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA256_DIGEST_SIZE);
} /* End of SHA256_End */


/*
========================================================================
Routine Description:
    SHA256 algorithm

Arguments:
    message         Message context
    messageLen      The length of message in bytes

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID SHA256 (
    IN  const UCHAR Message[], 
    IN  UINT MessageLen, 
    OUT UCHAR DigestMessage[])
{
    SHA256_CTX_STRUC sha_ctx;

    PlatformZeroMemory(&sha_ctx, sizeof(SHA256_CTX_STRUC));
    SHA256_Init(&sha_ctx);
    SHA256_Append(&sha_ctx, Message, MessageLen);
    SHA256_End(&sha_ctx, DigestMessage);
} /* End of SHA256 */
/* forward S-box */

static uint32 FSb[256] =
{
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,
    0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
    0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC,
    0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A,
    0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
    0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B,
    0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85,
    0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
    0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17,
    0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88,
    0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
    0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9,
    0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6,
    0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
    0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94,
    0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68,
    0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

/* forward table */

#define FT \
\
    V(C6,63,63,A5), V(F8,7C,7C,84), V(EE,77,77,99), V(F6,7B,7B,8D), \
    V(FF,F2,F2,0D), V(D6,6B,6B,BD), V(DE,6F,6F,B1), V(91,C5,C5,54), \
    V(60,30,30,50), V(02,01,01,03), V(CE,67,67,A9), V(56,2B,2B,7D), \
    V(E7,FE,FE,19), V(B5,D7,D7,62), V(4D,AB,AB,E6), V(EC,76,76,9A), \
    V(8F,CA,CA,45), V(1F,82,82,9D), V(89,C9,C9,40), V(FA,7D,7D,87), \
    V(EF,FA,FA,15), V(B2,59,59,EB), V(8E,47,47,C9), V(FB,F0,F0,0B), \
    V(41,AD,AD,EC), V(B3,D4,D4,67), V(5F,A2,A2,FD), V(45,AF,AF,EA), \
    V(23,9C,9C,BF), V(53,A4,A4,F7), V(E4,72,72,96), V(9B,C0,C0,5B), \
    V(75,B7,B7,C2), V(E1,FD,FD,1C), V(3D,93,93,AE), V(4C,26,26,6A), \
    V(6C,36,36,5A), V(7E,3F,3F,41), V(F5,F7,F7,02), V(83,CC,CC,4F), \
    V(68,34,34,5C), V(51,A5,A5,F4), V(D1,E5,E5,34), V(F9,F1,F1,08), \
    V(E2,71,71,93), V(AB,D8,D8,73), V(62,31,31,53), V(2A,15,15,3F), \
    V(08,04,04,0C), V(95,C7,C7,52), V(46,23,23,65), V(9D,C3,C3,5E), \
    V(30,18,18,28), V(37,96,96,A1), V(0A,05,05,0F), V(2F,9A,9A,B5), \
    V(0E,07,07,09), V(24,12,12,36), V(1B,80,80,9B), V(DF,E2,E2,3D), \
    V(CD,EB,EB,26), V(4E,27,27,69), V(7F,B2,B2,CD), V(EA,75,75,9F), \
    V(12,09,09,1B), V(1D,83,83,9E), V(58,2C,2C,74), V(34,1A,1A,2E), \
    V(36,1B,1B,2D), V(DC,6E,6E,B2), V(B4,5A,5A,EE), V(5B,A0,A0,FB), \
    V(A4,52,52,F6), V(76,3B,3B,4D), V(B7,D6,D6,61), V(7D,B3,B3,CE), \
    V(52,29,29,7B), V(DD,E3,E3,3E), V(5E,2F,2F,71), V(13,84,84,97), \
    V(A6,53,53,F5), V(B9,D1,D1,68), V(00,00,00,00), V(C1,ED,ED,2C), \
    V(40,20,20,60), V(E3,FC,FC,1F), V(79,B1,B1,C8), V(B6,5B,5B,ED), \
    V(D4,6A,6A,BE), V(8D,CB,CB,46), V(67,BE,BE,D9), V(72,39,39,4B), \
    V(94,4A,4A,DE), V(98,4C,4C,D4), V(B0,58,58,E8), V(85,CF,CF,4A), \
    V(BB,D0,D0,6B), V(C5,EF,EF,2A), V(4F,AA,AA,E5), V(ED,FB,FB,16), \
    V(86,43,43,C5), V(9A,4D,4D,D7), V(66,33,33,55), V(11,85,85,94), \
    V(8A,45,45,CF), V(E9,F9,F9,10), V(04,02,02,06), V(FE,7F,7F,81), \
    V(A0,50,50,F0), V(78,3C,3C,44), V(25,9F,9F,BA), V(4B,A8,A8,E3), \
    V(A2,51,51,F3), V(5D,A3,A3,FE), V(80,40,40,C0), V(05,8F,8F,8A), \
    V(3F,92,92,AD), V(21,9D,9D,BC), V(70,38,38,48), V(F1,F5,F5,04), \
    V(63,BC,BC,DF), V(77,B6,B6,C1), V(AF,DA,DA,75), V(42,21,21,63), \
    V(20,10,10,30), V(E5,FF,FF,1A), V(FD,F3,F3,0E), V(BF,D2,D2,6D), \
    V(81,CD,CD,4C), V(18,0C,0C,14), V(26,13,13,35), V(C3,EC,EC,2F), \
    V(BE,5F,5F,E1), V(35,97,97,A2), V(88,44,44,CC), V(2E,17,17,39), \
    V(93,C4,C4,57), V(55,A7,A7,F2), V(FC,7E,7E,82), V(7A,3D,3D,47), \
    V(C8,64,64,AC), V(BA,5D,5D,E7), V(32,19,19,2B), V(E6,73,73,95), \
    V(C0,60,60,A0), V(19,81,81,98), V(9E,4F,4F,D1), V(A3,DC,DC,7F), \
    V(44,22,22,66), V(54,2A,2A,7E), V(3B,90,90,AB), V(0B,88,88,83), \
    V(8C,46,46,CA), V(C7,EE,EE,29), V(6B,B8,B8,D3), V(28,14,14,3C), \
    V(A7,DE,DE,79), V(BC,5E,5E,E2), V(16,0B,0B,1D), V(AD,DB,DB,76), \
    V(DB,E0,E0,3B), V(64,32,32,56), V(74,3A,3A,4E), V(14,0A,0A,1E), \
    V(92,49,49,DB), V(0C,06,06,0A), V(48,24,24,6C), V(B8,5C,5C,E4), \
    V(9F,C2,C2,5D), V(BD,D3,D3,6E), V(43,AC,AC,EF), V(C4,62,62,A6), \
    V(39,91,91,A8), V(31,95,95,A4), V(D3,E4,E4,37), V(F2,79,79,8B), \
    V(D5,E7,E7,32), V(8B,C8,C8,43), V(6E,37,37,59), V(DA,6D,6D,B7), \
    V(01,8D,8D,8C), V(B1,D5,D5,64), V(9C,4E,4E,D2), V(49,A9,A9,E0), \
    V(D8,6C,6C,B4), V(AC,56,56,FA), V(F3,F4,F4,07), V(CF,EA,EA,25), \
    V(CA,65,65,AF), V(F4,7A,7A,8E), V(47,AE,AE,E9), V(10,08,08,18), \
    V(6F,BA,BA,D5), V(F0,78,78,88), V(4A,25,25,6F), V(5C,2E,2E,72), \
    V(38,1C,1C,24), V(57,A6,A6,F1), V(73,B4,B4,C7), V(97,C6,C6,51), \
    V(CB,E8,E8,23), V(A1,DD,DD,7C), V(E8,74,74,9C), V(3E,1F,1F,21), \
    V(96,4B,4B,DD), V(61,BD,BD,DC), V(0D,8B,8B,86), V(0F,8A,8A,85), \
    V(E0,70,70,90), V(7C,3E,3E,42), V(71,B5,B5,C4), V(CC,66,66,AA), \
    V(90,48,48,D8), V(06,03,03,05), V(F7,F6,F6,01), V(1C,0E,0E,12), \
    V(C2,61,61,A3), V(6A,35,35,5F), V(AE,57,57,F9), V(69,B9,B9,D0), \
    V(17,86,86,91), V(99,C1,C1,58), V(3A,1D,1D,27), V(27,9E,9E,B9), \
    V(D9,E1,E1,38), V(EB,F8,F8,13), V(2B,98,98,B3), V(22,11,11,33), \
    V(D2,69,69,BB), V(A9,D9,D9,70), V(07,8E,8E,89), V(33,94,94,A7), \
    V(2D,9B,9B,B6), V(3C,1E,1E,22), V(15,87,87,92), V(C9,E9,E9,20), \
    V(87,CE,CE,49), V(AA,55,55,FF), V(50,28,28,78), V(A5,DF,DF,7A), \
    V(03,8C,8C,8F), V(59,A1,A1,F8), V(09,89,89,80), V(1A,0D,0D,17), \
    V(65,BF,BF,DA), V(D7,E6,E6,31), V(84,42,42,C6), V(D0,68,68,B8), \
    V(82,41,41,C3), V(29,99,99,B0), V(5A,2D,2D,77), V(1E,0F,0F,11), \
    V(7B,B0,B0,CB), V(A8,54,54,FC), V(6D,BB,BB,D6), V(2C,16,16,3A)

#define V(a,b,c,d) 0x##a##b##c##d
static uint32 FT0[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##d##a##b##c
static uint32 FT1[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##c##d##a##b
static uint32 FT2[256] = { FT };
#undef V

#define V(a,b,c,d) 0x##b##c##d##a
static uint32 FT3[256] = { FT };
#undef V

#undef FT

/* reverse S-box */

static uint32 RSb[256] =
{
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38,
    0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
    0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D,
    0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2,
    0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA,
    0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A,
    0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
    0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA,
    0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85,
    0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
    0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20,
    0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31,
    0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
    0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0,
    0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26,
    0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

/* reverse table */

#define RT \
\
    V(51,F4,A7,50), V(7E,41,65,53), V(1A,17,A4,C3), V(3A,27,5E,96), \
    V(3B,AB,6B,CB), V(1F,9D,45,F1), V(AC,FA,58,AB), V(4B,E3,03,93), \
    V(20,30,FA,55), V(AD,76,6D,F6), V(88,CC,76,91), V(F5,02,4C,25), \
    V(4F,E5,D7,FC), V(C5,2A,CB,D7), V(26,35,44,80), V(B5,62,A3,8F), \
    V(DE,B1,5A,49), V(25,BA,1B,67), V(45,EA,0E,98), V(5D,FE,C0,E1), \
    V(C3,2F,75,02), V(81,4C,F0,12), V(8D,46,97,A3), V(6B,D3,F9,C6), \
    V(03,8F,5F,E7), V(15,92,9C,95), V(BF,6D,7A,EB), V(95,52,59,DA), \
    V(D4,BE,83,2D), V(58,74,21,D3), V(49,E0,69,29), V(8E,C9,C8,44), \
    V(75,C2,89,6A), V(F4,8E,79,78), V(99,58,3E,6B), V(27,B9,71,DD), \
    V(BE,E1,4F,B6), V(F0,88,AD,17), V(C9,20,AC,66), V(7D,CE,3A,B4), \
    V(63,DF,4A,18), V(E5,1A,31,82), V(97,51,33,60), V(62,53,7F,45), \
    V(B1,64,77,E0), V(BB,6B,AE,84), V(FE,81,A0,1C), V(F9,08,2B,94), \
    V(70,48,68,58), V(8F,45,FD,19), V(94,DE,6C,87), V(52,7B,F8,B7), \
    V(AB,73,D3,23), V(72,4B,02,E2), V(E3,1F,8F,57), V(66,55,AB,2A), \
    V(B2,EB,28,07), V(2F,B5,C2,03), V(86,C5,7B,9A), V(D3,37,08,A5), \
    V(30,28,87,F2), V(23,BF,A5,B2), V(02,03,6A,BA), V(ED,16,82,5C), \
    V(8A,CF,1C,2B), V(A7,79,B4,92), V(F3,07,F2,F0), V(4E,69,E2,A1), \
    V(65,DA,F4,CD), V(06,05,BE,D5), V(D1,34,62,1F), V(C4,A6,FE,8A), \
    V(34,2E,53,9D), V(A2,F3,55,A0), V(05,8A,E1,32), V(A4,F6,EB,75), \
    V(0B,83,EC,39), V(40,60,EF,AA), V(5E,71,9F,06), V(BD,6E,10,51), \
    V(3E,21,8A,F9), V(96,DD,06,3D), V(DD,3E,05,AE), V(4D,E6,BD,46), \
    V(91,54,8D,B5), V(71,C4,5D,05), V(04,06,D4,6F), V(60,50,15,FF), \
    V(19,98,FB,24), V(D6,BD,E9,97), V(89,40,43,CC), V(67,D9,9E,77), \
    V(B0,E8,42,BD), V(07,89,8B,88), V(E7,19,5B,38), V(79,C8,EE,DB), \
    V(A1,7C,0A,47), V(7C,42,0F,E9), V(F8,84,1E,C9), V(00,00,00,00), \
    V(09,80,86,83), V(32,2B,ED,48), V(1E,11,70,AC), V(6C,5A,72,4E), \
    V(FD,0E,FF,FB), V(0F,85,38,56), V(3D,AE,D5,1E), V(36,2D,39,27), \
    V(0A,0F,D9,64), V(68,5C,A6,21), V(9B,5B,54,D1), V(24,36,2E,3A), \
    V(0C,0A,67,B1), V(93,57,E7,0F), V(B4,EE,96,D2), V(1B,9B,91,9E), \
    V(80,C0,C5,4F), V(61,DC,20,A2), V(5A,77,4B,69), V(1C,12,1A,16), \
    V(E2,93,BA,0A), V(C0,A0,2A,E5), V(3C,22,E0,43), V(12,1B,17,1D), \
    V(0E,09,0D,0B), V(F2,8B,C7,AD), V(2D,B6,A8,B9), V(14,1E,A9,C8), \
    V(57,F1,19,85), V(AF,75,07,4C), V(EE,99,DD,BB), V(A3,7F,60,FD), \
    V(F7,01,26,9F), V(5C,72,F5,BC), V(44,66,3B,C5), V(5B,FB,7E,34), \
    V(8B,43,29,76), V(CB,23,C6,DC), V(B6,ED,FC,68), V(B8,E4,F1,63), \
    V(D7,31,DC,CA), V(42,63,85,10), V(13,97,22,40), V(84,C6,11,20), \
    V(85,4A,24,7D), V(D2,BB,3D,F8), V(AE,F9,32,11), V(C7,29,A1,6D), \
    V(1D,9E,2F,4B), V(DC,B2,30,F3), V(0D,86,52,EC), V(77,C1,E3,D0), \
    V(2B,B3,16,6C), V(A9,70,B9,99), V(11,94,48,FA), V(47,E9,64,22), \
    V(A8,FC,8C,C4), V(A0,F0,3F,1A), V(56,7D,2C,D8), V(22,33,90,EF), \
    V(87,49,4E,C7), V(D9,38,D1,C1), V(8C,CA,A2,FE), V(98,D4,0B,36), \
    V(A6,F5,81,CF), V(A5,7A,DE,28), V(DA,B7,8E,26), V(3F,AD,BF,A4), \
    V(2C,3A,9D,E4), V(50,78,92,0D), V(6A,5F,CC,9B), V(54,7E,46,62), \
    V(F6,8D,13,C2), V(90,D8,B8,E8), V(2E,39,F7,5E), V(82,C3,AF,F5), \
    V(9F,5D,80,BE), V(69,D0,93,7C), V(6F,D5,2D,A9), V(CF,25,12,B3), \
    V(C8,AC,99,3B), V(10,18,7D,A7), V(E8,9C,63,6E), V(DB,3B,BB,7B), \
    V(CD,26,78,09), V(6E,59,18,F4), V(EC,9A,B7,01), V(83,4F,9A,A8), \
    V(E6,95,6E,65), V(AA,FF,E6,7E), V(21,BC,CF,08), V(EF,15,E8,E6), \
    V(BA,E7,9B,D9), V(4A,6F,36,CE), V(EA,9F,09,D4), V(29,B0,7C,D6), \
    V(31,A4,B2,AF), V(2A,3F,23,31), V(C6,A5,94,30), V(35,A2,66,C0), \
    V(74,4E,BC,37), V(FC,82,CA,A6), V(E0,90,D0,B0), V(33,A7,D8,15), \
    V(F1,04,98,4A), V(41,EC,DA,F7), V(7F,CD,50,0E), V(17,91,F6,2F), \
    V(76,4D,D6,8D), V(43,EF,B0,4D), V(CC,AA,4D,54), V(E4,96,04,DF), \
    V(9E,D1,B5,E3), V(4C,6A,88,1B), V(C1,2C,1F,B8), V(46,65,51,7F), \
    V(9D,5E,EA,04), V(01,8C,35,5D), V(FA,87,74,73), V(FB,0B,41,2E), \
    V(B3,67,1D,5A), V(92,DB,D2,52), V(E9,10,56,33), V(6D,D6,47,13), \
    V(9A,D7,61,8C), V(37,A1,0C,7A), V(59,F8,14,8E), V(EB,13,3C,89), \
    V(CE,A9,27,EE), V(B7,61,C9,35), V(E1,1C,E5,ED), V(7A,47,B1,3C), \
    V(9C,D2,DF,59), V(55,F2,73,3F), V(18,14,CE,79), V(73,C7,37,BF), \
    V(53,F7,CD,EA), V(5F,FD,AA,5B), V(DF,3D,6F,14), V(78,44,DB,86), \
    V(CA,AF,F3,81), V(B9,68,C4,3E), V(38,24,34,2C), V(C2,A3,40,5F), \
    V(16,1D,C3,72), V(BC,E2,25,0C), V(28,3C,49,8B), V(FF,0D,95,41), \
    V(39,A8,01,71), V(08,0C,B3,DE), V(D8,B4,E4,9C), V(64,56,C1,90), \
    V(7B,CB,84,61), V(D5,32,B6,70), V(48,6C,5C,74), V(D0,B8,57,42)

#define V(a,b,c,d) 0x##a##b##c##d
static uint32 RT0[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##d##a##b##c
static uint32 RT1[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##c##d##a##b
static uint32 RT2[256] = { RT };
#undef V

#define V(a,b,c,d) 0x##b##c##d##a
static uint32 RT3[256] = { RT };
#undef V

#undef RT

/* round constants */

static uint32 RCON[10] =
{
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1B000000, 0x36000000
};

/* key schedule tables */

static int KT_init = 1;

static uint32 KT0[256];
static uint32 KT1[256];
static uint32 KT2[256];
static uint32 KT3[256];

/* platform-independant 32-bit integer manipulation macros */

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (uint32) (b)[(i)    ] << 24 )       \
        | ( (uint32) (b)[(i) + 1] << 16 )       \
        | ( (uint32) (b)[(i) + 2] <<  8 )       \
        | ( (uint32) (b)[(i) + 3]       );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8) ( (n)       );       \
}

/* AES key scheduling routine */

int aes_set_key( aes_context *ctx, uint8 *key, int nbits )
{
    int i;
    uint32 *RK, *SK;

    switch( nbits )
    {
        case 128: ctx->nr = 10; break;
        case 192: ctx->nr = 12; break;
        case 256: ctx->nr = 14; break;
        default : return( 1 );
    }

    RK = ctx->erk;

    for( i = 0; i < (nbits >> 5); i++ )
    {
        GET_UINT32( RK[i], key, i * 4 );
    }

    /* setup encryption round keys */

    switch( nbits )
    {
        case 128:

            for( i = 0; i < 10; i++, RK += 4 )
            {
                RK[4]  = RK[0] ^ RCON[i] ^
                            ( FSb[ (uint8) ( RK[3] >> 16 ) ] << 24 ) ^
                            ( FSb[ (uint8) ( RK[3] >>  8 ) ] << 16 ) ^
                            ( FSb[ (uint8) ( RK[3]       ) ] <<  8 ) ^
                            ( FSb[ (uint8) ( RK[3] >> 24 ) ]       );

                RK[5]  = RK[1] ^ RK[4];
                RK[6]  = RK[2] ^ RK[5];
                RK[7]  = RK[3] ^ RK[6];
            }
            break;

        case 192:

            for( i = 0; i < 8; i++, RK += 6 )
            {
                RK[6]  = RK[0] ^ RCON[i] ^
                            ( FSb[ (uint8) ( RK[5] >> 16 ) ] << 24 ) ^
                            ( FSb[ (uint8) ( RK[5] >>  8 ) ] << 16 ) ^
                            ( FSb[ (uint8) ( RK[5]       ) ] <<  8 ) ^
                            ( FSb[ (uint8) ( RK[5] >> 24 ) ]       );

                RK[7]  = RK[1] ^ RK[6];
                RK[8]  = RK[2] ^ RK[7];
                RK[9]  = RK[3] ^ RK[8];
                RK[10] = RK[4] ^ RK[9];
                RK[11] = RK[5] ^ RK[10];
            }
            break;

        case 256:

            for( i = 0; i < 7; i++, RK += 8 )
            {
                RK[8]  = RK[0] ^ RCON[i] ^
                            ( FSb[ (uint8) ( RK[7] >> 16 ) ] << 24 ) ^
                            ( FSb[ (uint8) ( RK[7] >>  8 ) ] << 16 ) ^
                            ( FSb[ (uint8) ( RK[7]       ) ] <<  8 ) ^
                            ( FSb[ (uint8) ( RK[7] >> 24 ) ]       );

                RK[9]  = RK[1] ^ RK[8];
                RK[10] = RK[2] ^ RK[9];
                RK[11] = RK[3] ^ RK[10];

                RK[12] = RK[4] ^
                            ( FSb[ (uint8) ( RK[11] >> 24 ) ] << 24 ) ^
                            ( FSb[ (uint8) ( RK[11] >> 16 ) ] << 16 ) ^
                            ( FSb[ (uint8) ( RK[11] >>  8 ) ] <<  8 ) ^
                            ( FSb[ (uint8) ( RK[11]       ) ]       );

                RK[13] = RK[5] ^ RK[12];
                RK[14] = RK[6] ^ RK[13];
                RK[15] = RK[7] ^ RK[14];
            }
            break;
    }

    /* setup decryption round keys */

    if( KT_init )
    {
        for( i = 0; i < 256; i++ )
        {
            KT0[i] = RT0[ FSb[i] ];
            KT1[i] = RT1[ FSb[i] ];
            KT2[i] = RT2[ FSb[i] ];
            KT3[i] = RT3[ FSb[i] ];
        }

        KT_init = 0;
    }

    SK = ctx->drk;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    for( i = 1; i < ctx->nr; i++ )
    {
        RK -= 8;

        *SK++ = KT0[ (uint8) ( *RK >> 24 ) ] ^
                KT1[ (uint8) ( *RK >> 16 ) ] ^
                KT2[ (uint8) ( *RK >>  8 ) ] ^
                KT3[ (uint8) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (uint8) ( *RK >> 24 ) ] ^
                KT1[ (uint8) ( *RK >> 16 ) ] ^
                KT2[ (uint8) ( *RK >>  8 ) ] ^
                KT3[ (uint8) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (uint8) ( *RK >> 24 ) ] ^
                KT1[ (uint8) ( *RK >> 16 ) ] ^
                KT2[ (uint8) ( *RK >>  8 ) ] ^
                KT3[ (uint8) ( *RK       ) ]; RK++;

        *SK++ = KT0[ (uint8) ( *RK >> 24 ) ] ^
                KT1[ (uint8) ( *RK >> 16 ) ] ^
                KT2[ (uint8) ( *RK >>  8 ) ] ^
                KT3[ (uint8) ( *RK       ) ]; RK++;
    }

    RK -= 8;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    return( 0 );
}

/* AES 128-bit block encryption routine */

void aes_encrypt(aes_context *ctx, uint8 input[16], uint8 output[16] )
{
    uint32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->erk;
    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ FT0[ (uint8) ( Y0 >> 24 ) ] ^  \
                 FT1[ (uint8) ( Y1 >> 16 ) ] ^  \
                 FT2[ (uint8) ( Y2 >>  8 ) ] ^  \
                 FT3[ (uint8) ( Y3       ) ];   \
                                                \
    X1 = RK[1] ^ FT0[ (uint8) ( Y1 >> 24 ) ] ^  \
                 FT1[ (uint8) ( Y2 >> 16 ) ] ^  \
                 FT2[ (uint8) ( Y3 >>  8 ) ] ^  \
                 FT3[ (uint8) ( Y0       ) ];   \
                                                \
    X2 = RK[2] ^ FT0[ (uint8) ( Y2 >> 24 ) ] ^  \
                 FT1[ (uint8) ( Y3 >> 16 ) ] ^  \
                 FT2[ (uint8) ( Y0 >>  8 ) ] ^  \
                 FT3[ (uint8) ( Y1       ) ];   \
                                                \
    X3 = RK[3] ^ FT0[ (uint8) ( Y3 >> 24 ) ] ^  \
                 FT1[ (uint8) ( Y0 >> 16 ) ] ^  \
                 FT2[ (uint8) ( Y1 >>  8 ) ] ^  \
                 FT3[ (uint8) ( Y2       ) ];   \
}

    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( FSb[ (uint8) ( Y0 >> 24 ) ] << 24 ) ^
                 ( FSb[ (uint8) ( Y1 >> 16 ) ] << 16 ) ^
                 ( FSb[ (uint8) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (uint8) ( Y3       ) ]       );

    X1 = RK[1] ^ ( FSb[ (uint8) ( Y1 >> 24 ) ] << 24 ) ^
                 ( FSb[ (uint8) ( Y2 >> 16 ) ] << 16 ) ^
                 ( FSb[ (uint8) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (uint8) ( Y0       ) ]       );

    X2 = RK[2] ^ ( FSb[ (uint8) ( Y2 >> 24 ) ] << 24 ) ^
                 ( FSb[ (uint8) ( Y3 >> 16 ) ] << 16 ) ^
                 ( FSb[ (uint8) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (uint8) ( Y1       ) ]       );

    X3 = RK[3] ^ ( FSb[ (uint8) ( Y3 >> 24 ) ] << 24 ) ^
                 ( FSb[ (uint8) ( Y0 >> 16 ) ] << 16 ) ^
                 ( FSb[ (uint8) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( FSb[ (uint8) ( Y2       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

/* AES 128-bit block decryption routine */

void aes_decrypt( aes_context *ctx, uint8 input[16], uint8 output[16] )
{
    uint32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->drk;

    GET_UINT32( X0, input,  0 ); X0 ^= RK[0];
    GET_UINT32( X1, input,  4 ); X1 ^= RK[1];
    GET_UINT32( X2, input,  8 ); X2 ^= RK[2];
    GET_UINT32( X3, input, 12 ); X3 ^= RK[3];

#define AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    RK += 4;                                    \
                                                \
    X0 = RK[0] ^ RT0[ (uint8) ( Y0 >> 24 ) ] ^  \
                 RT1[ (uint8) ( Y3 >> 16 ) ] ^  \
                 RT2[ (uint8) ( Y2 >>  8 ) ] ^  \
                 RT3[ (uint8) ( Y1       ) ];   \
                                                \
    X1 = RK[1] ^ RT0[ (uint8) ( Y1 >> 24 ) ] ^  \
                 RT1[ (uint8) ( Y0 >> 16 ) ] ^  \
                 RT2[ (uint8) ( Y3 >>  8 ) ] ^  \
                 RT3[ (uint8) ( Y2       ) ];   \
                                                \
    X2 = RK[2] ^ RT0[ (uint8) ( Y2 >> 24 ) ] ^  \
                 RT1[ (uint8) ( Y1 >> 16 ) ] ^  \
                 RT2[ (uint8) ( Y0 >>  8 ) ] ^  \
                 RT3[ (uint8) ( Y3       ) ];   \
                                                \
    X3 = RK[3] ^ RT0[ (uint8) ( Y3 >> 24 ) ] ^  \
                 RT1[ (uint8) ( Y2 >> 16 ) ] ^  \
                 RT2[ (uint8) ( Y1 >>  8 ) ] ^  \
                 RT3[ (uint8) ( Y0       ) ];   \
}

    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 1 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 2 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 3 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 4 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 5 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 6 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 7 */
    AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );       /* round 8 */
    AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );       /* round 9 */

    if( ctx->nr > 10 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 10 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 11 */
    }

    if( ctx->nr > 12 )
    {
        AES_RROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );   /* round 12 */
        AES_RROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );   /* round 13 */
    }

    /* last round */

    RK += 4;

    X0 = RK[0] ^ ( RSb[ (uint8) ( Y0 >> 24 ) ] << 24 ) ^
                 ( RSb[ (uint8) ( Y3 >> 16 ) ] << 16 ) ^
                 ( RSb[ (uint8) ( Y2 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (uint8) ( Y1       ) ]       );

    X1 = RK[1] ^ ( RSb[ (uint8) ( Y1 >> 24 ) ] << 24 ) ^
                 ( RSb[ (uint8) ( Y0 >> 16 ) ] << 16 ) ^
                 ( RSb[ (uint8) ( Y3 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (uint8) ( Y2       ) ]       );

    X2 = RK[2] ^ ( RSb[ (uint8) ( Y2 >> 24 ) ] << 24 ) ^
                 ( RSb[ (uint8) ( Y1 >> 16 ) ] << 16 ) ^
                 ( RSb[ (uint8) ( Y0 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (uint8) ( Y3       ) ]       );

    X3 = RK[3] ^ ( RSb[ (uint8) ( Y3 >> 24 ) ] << 24 ) ^
                 ( RSb[ (uint8) ( Y2 >> 16 ) ] << 16 ) ^
                 ( RSb[ (uint8) ( Y1 >>  8 ) ] <<  8 ) ^
                 ( RSb[ (uint8) ( Y0       ) ]       );

    PUT_UINT32( X0, output,  0 );
    PUT_UINT32( X1, output,  4 );
    PUT_UINT32( X2, output,  8 );
    PUT_UINT32( X3, output, 12 );
}

/* =======  The related function of AES-128-CMAC  ======= */
VOID leftshift_onebit(
    IN  PUCHAR  input, 
    OUT PUCHAR  output)
{
    INT i;
    UCHAR overflow = 0;

    for (i=15; i>=0; i--)
    {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80) ? 1 : 0;
    }   
}

VOID do_padding(
    IN  PUCHAR  lastb,
    OUT PUCHAR  pad,
    IN  INT     len)
{
    INT j;

    for (j=0; j<16; j++)
    {
        if (j < len)
            pad[j] = lastb[j];
        else if (j == len)
            pad[j] = 0x80;
        else
            pad[j] = 0x00;
    }


}

/*
 *  The Subkey Generation Algorithm 
 */
VOID generate_subkey(
    IN  PUCHAR key,
    OUT PUCHAR K1, 
    OUT PUCHAR K2)
{
    aes_context aesctx;
    UCHAR aes_128_key[16];  
    UCHAR const_Zero[16];   
    UCHAR tmp[16];
    UCHAR const_Rb[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};                          

    // initial the key material 
    PlatformZeroMemory(const_Zero, 16);
    PlatformZeroMemory(aes_128_key, 16);

    // AES-128 with key is applied to an all-zero input block
    aes_set_key(&aesctx, key, 128);
    aes_encrypt(&aesctx, const_Zero, aes_128_key);

    // derive K1(128-bit first subkey) and K2(128-bit second subkey), refer to rfc-4493 ch 2.3  
    if ((aes_128_key[0] & 0x80) == 0)
    {
        leftshift_onebit(aes_128_key, K1);
    }
    else
    {
        leftshift_onebit(aes_128_key, tmp);
        xor_128(tmp, const_Rb, K1);
    }

    if ((K1[0] & 0x80) == 0)
    {
        leftshift_onebit(K1, K2);
    }
    else
    {
        leftshift_onebit(K1, tmp);
        xor_128(tmp, const_Rb, K2);
    }   

#if 0
    DBGPRINT(RT_DEBUG_TRACE, ("-----------------------\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("Subkey Generation\n"));
#endif          
}

/*
 *  AES-CMAC Algorithm. (refer to rfc-4493 and SP800-38B)
 *  
 *  Input : key         (128-bit key)
 *          input       (message to be authenticated)
 *          len         (length of the message in octets)
 *          
 *  output: mac         (message authentication code)
 */
VOID AES_128_CMAC(
    IN  PUCHAR  key,
    IN  PUCHAR  input,
    IN  INT     len,
    OUT PUCHAR  mac)
{
    UCHAR   X[16], Y[16], M_last[16], padded[16];
    UCHAR   K1[16], K2[16];
    aes_context aesctx;
    INT     n, i, flag;

    generate_subkey(key, K1, K2);
    
    n = (len+15) / 16;      // n is number of rounds

    if (n == 0)
    {
        n = 1;
        flag = 0;
    }
    else
    {           
        if ((len%16) == 0)
            flag = 1;           // indicate that last block is a complete block     
        else
            flag = 0;           // indicate that last block is not a complete block
    }

    if (flag)
    {
        xor_128(&input[16*(n-1)], K1, M_last);
    }
    else
    {
        do_padding(&input[16*(n-1)], padded, len%16);
        xor_128(padded, K2, M_last);
    }

    PlatformZeroMemory(X, 16);
    for (i=0; i<n-1; i++)
    {
        xor_128(X, &input[16*i], Y);
        aes_set_key(&aesctx, key, 128);
        aes_encrypt(&aesctx, Y, X);
    }

    xor_128(X, M_last, Y);
    aes_set_key(&aesctx, key, 128);
    aes_encrypt(&aesctx, Y, X);

    for (i=0; i<16; i++)
    {
        mac[i] = X[i];
    }

}
/* =======  The related function of AES-128-CMAC  ======= */


