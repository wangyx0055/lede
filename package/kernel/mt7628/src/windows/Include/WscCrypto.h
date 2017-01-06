#ifndef _WSCCRYPTO_H_
#define _WSCCRYPTO_H_

#include <stdlib.h>

#define IS_LITTLE_ENDIAN        1234 /* byte 0 is least significant (i386) */
#define IS_BIG_ENDIAN           4321 /* byte 0 is most significant (mc68k) */
#ifdef BIG_ENDIAN
#define PLATFORM_BYTE_ORDER     IS_BIG_ENDIAN
#else
#define PLATFORM_BYTE_ORDER     IS_LITTLE_ENDIAN
#endif


typedef unsigned char       uint_8t;
typedef unsigned short      uint_16t;
typedef unsigned int        uint_32t;
//typedef unsigned long       uint_32t;
//typedef unsigned long long  uint_64t;
typedef ULONGLONG uint_64t;

#define void_ret  void
#define int_ret   int

#undef  EXIT_SUCCESS
#undef  EXIT_FAILURE
#define EXIT_SUCCESS    0x00
#define EXIT_FAILURE    0x01        

/* define the hash functions that you need  */
#define SHA_2   /* for dynamic hash length  */
#define SHA_256


/* Note that the following function prototypes are the same */
/* for both the bit and byte oriented implementations.  But */
/* the length fields are in bytes or bits as is appropriate */
/* for the version used.  Bit sequences are arrays of bytes */
/* in which bit sequence indexes increase from the most to  */
/* the least significant end of each byte                   */
#define SHA256_DIGEST_SIZE  32
#define SHA256_BLOCK_SIZE   64

/* type to hold the SHA256 context */
typedef struct
{   
    uint_32t count[2];
    uint_32t hash[8];
    uint_32t wbuf[16];
} sha256_ctx;


void_ret sha256_compile(sha256_ctx ctx[1]);
void_ret sha256_begin(sha256_ctx ctx[1]);
void_ret sha256_hash(const unsigned char data[], unsigned long len, sha256_ctx ctx[1]);
void_ret sha256_end(unsigned char hval[], sha256_ctx ctx[1]);
void_ret sha256(unsigned char hval[], const unsigned char data[], unsigned long len);
 

#ifndef SHA_64BIT

typedef struct
{   
    union
    { 
        sha256_ctx  ctx256[1];
    } uu[1];
    uint_32t    sha2_len;
} sha2_ctx;

#else

/* type to hold the SHA384 (and SHA512) context */
typedef struct
{   
    uint_64t count[2];
    uint_64t hash[8];
    uint_64t wbuf[16];
} sha512_ctx;

//typedef sha512_ctx  sha384_ctx;

typedef struct
{ 
    union
    { 
        sha256_ctx  ctx256[1];
        sha512_ctx  ctx512[1];
    } uu[1];
    uint_32t    sha2_len;
} sha2_ctx;

int_ret  sha2_begin(unsigned long size, sha2_ctx ctx[1]);
void_ret sha2_hash(const unsigned char data[], unsigned long len, sha2_ctx ctx[1]);
void_ret sha2_end(unsigned char hval[], sha2_ctx ctx[1]);
int_ret  sha2(unsigned char hval[], unsigned long size, const unsigned char data[], unsigned long len);

#endif /* SHA_64BIT */
#endif  /* _SHA2_H */


#ifndef _HMAC_H
#define _HMAC_H

#define HASH_INPUT_SIZE     SHA256_BLOCK_SIZE
#define HASH_OUTPUT_SIZE    SHA256_DIGEST_SIZE
#define sha_ctx             sha256_ctx
#define sha_begin           sha256_begin
#define sha_hash            sha256_hash
#define sha_end             sha256_end


#define HMAC_OK                0
#define HMAC_BAD_MODE         -1
#define HMAC_IN_DATA  0xffffffff

typedef struct
{   
    unsigned char   key[HASH_INPUT_SIZE];
    sha_ctx         ctx[1];
    unsigned long   klen;
} hmac_ctx;

void hmac_sha_begin(hmac_ctx cx[1]);

int  hmac_sha_key(const unsigned char key[], unsigned long key_len, hmac_ctx cx[1]);

void hmac_sha_data(const unsigned char data[], unsigned long data_len, hmac_ctx cx[1]);

void hmac_sha_end(unsigned char mac[], unsigned long mac_len, hmac_ctx cx[1]);

void hmac_sha(const unsigned char key[], unsigned long key_len,
          const unsigned char data[], unsigned long data_len,
          unsigned char mac[], unsigned long mac_len);


#endif /* _HMAC_H */


#ifndef _EVP_ENC_H
#define _EVP_ENC_H

//#undef SWAP32
//#define SWAP32(x) \
//    ((unsigned long)( \
//    (((unsigned long)(x) & (unsigned long) 0x000000ffUL) << 24) | \
//    (((unsigned long)(x) & (unsigned long) 0x0000ff00UL) <<  8) | \
//    (((unsigned long)(x) & (unsigned long) 0x00ff0000UL) >>  8) | \
//    (((unsigned long)(x) & (unsigned long) 0xff000000UL) >> 24) ))

#define GETU32(p) cpu2be32(*((u32 *)(p)))
#define PUTU32(ct, st) { *((u32 *)(ct)) = cpu2be32((st)); }


#define AES_ENCRYPT 1
#define AES_DECRYPT 0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct aes_key_st AES_KEY;

typedef struct _EVP_CIPHER_CTX_ {
    unsigned long flag;
    unsigned long type;  
    unsigned long encrypt; //1: Encrypt 0: Decrypt, 
    unsigned char key[16];
    unsigned char iv[8 + 16];
    unsigned long bufferlen;
    unsigned char buffer[AES_BLOCK_SIZE];
    AES_KEY        aesKey;
} EVP_CIPHER_CTX, *PEVP_CIPHER_CTX;

void AES_encrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key);
int AES_set_encrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key);
int EVP_aes_128_cbc(void);
int EVP_EncryptInit(EVP_CIPHER_CTX *ctx, int type, unsigned char *key, unsigned char *iv);
int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen, unsigned char *inbuf, int inlen);
int EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen);
int EVP_DecryptInit(EVP_CIPHER_CTX *ctx, int type, unsigned char *key, unsigned char *iv);
int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen, unsigned char *inbuf, int inlen);
int EVP_DecryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *outbuf, int *outlen);
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
             const unsigned long length, const AES_KEY *key,
             unsigned char *ivec, const int enc);

void WscEncryptData(
    PMP_ADAPTER pAd,
    unsigned char *plainText,   int ptx_len,
    unsigned char *key, unsigned char *iv,
    unsigned char *cipherText,  int *ctx_len);
    
void WscDecryptData(
    PMP_ADAPTER pAd,
    unsigned char *cipherText,  int ctx_len, 
    unsigned char *key, unsigned char *iv,
    unsigned char *plainText,   int *ptx_len);           

int WscDeriveKey(
    unsigned char *kdk, unsigned int kdk_len, 
    unsigned char *prsnlString, unsigned int str_len,
    unsigned char *key, unsigned int keyBits );

#endif  /* _EVP_ENC_H */

#ifndef _DH_KEY_H
#define _DH_KEY_H

#define MODP_1536_BITS      1536                        // 1563 bits
#define MODP_1536_LEN       ((MODP_1536_BITS + 7) / 8)  // 192 bytes
#define DH_KEY_LEN          MODP_1536_LEN

void GenerateDHPublicKey(unsigned char *ran_buf, int ran_len, 
                         unsigned char *dhkey, int *dhkey_len);
                          
void GenerateDHSecreteKey(unsigned char *ran_buf, int ran_len,
                          unsigned char *peer_dhkey, int peer_dhkey_len,
                          unsigned char *secrete_dhkey, int *secrete_dhkey_len);
                           

//====== Natural numbers routines ====

/* Type definitions. 
 */
/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;
typedef unsigned long int NN_DIGIT;         // a four byte word
typedef unsigned short int NN_HALF_DIGIT;   // a two byte word

/* Constants.

   Note: MAX_NN_DIGITS is long enough to hold any RSA modulus, plus
   one more digit as required by R_GeneratePEMKeys (for n and phiN,
   whose lengths must be even). All natural numbers have at most
   MAX_NN_DIGITS digits, except for double-length intermediate values
   in NN_Mult (t), NN_ModMult (t), NN_ModInv (w), and NN_Div (c).
 */
/* Length of digit in bits */
#define NN_DIGIT_BITS 32
#define NN_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_NN_DIGITS \
  ((DH_KEY_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_NN_DIGIT        0xffffffff  
#define MAX_NN_HALF_DIGIT   0xffff

/* Macros.
 */ 
#define LOW_HALF(x) ((x) & MAX_NN_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((NN_DIGIT)(x)) << NN_HALF_DIGIT_BITS)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 2)) & 3)
#define R_memset(Destination, Fill, Length)     memset(Destination, Fill, Length)


/* CONVERSIONS
   NN_Decode (a, digits, b, len)   Decodes character string b into a.
   NN_Encode (a, len, b, digits)   Encodes a into character string b.

   ASSIGNMENTS
   NN_Assign (a, b, digits)        Assigns a = b.
   NN_AssignZero (a, b, digits)    Assigns a = 0.
     
   ARITHMETIC OPERATIONS
   NN_Sub (a, b, c, digits)        Computes a = b - c.
   NN_Mult (a, b, c, digits)       Computes a = b * c.
   NN_LShift (a, b, c, digits)     Computes a = b * 2^c.
   NN_RShift (a, b, c, digits)     Computes a = b / 2^c.
   NN_Div (a, b, c, cDigits, d, dDigits)  Computes a = c div d and b = c mod d.

   NUMBER THEORY
   NN_Mod (a, b, bDigits, c, cDigits)  Computes a = b mod c.
   NN_ModMult (a, b, c, d, digits) Computes a = b * c mod d.
   NN_ModExp (a, b, c, cDigits, d, dDigits)  Computes a = b^c mod d.
 
   OTHER OPERATIONS
   NN_Cmp (a, b, digits)           Returns sign of a - b.
   NN_Digits (a, digits)           Returns significant length of a in digits.
 */
 
void NN_Decode(NN_DIGIT *, unsigned int, unsigned char *, unsigned int);
void NN_Encode(unsigned char *, unsigned int, NN_DIGIT *, unsigned int);

void NN_Assign(NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_AssignZero(NN_DIGIT *, unsigned int);


NN_DIGIT NN_Sub(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_Mult(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_Div(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int);
NN_DIGIT NN_LShift(NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int);
NN_DIGIT NN_RShift(NN_DIGIT *, NN_DIGIT *, unsigned int, unsigned int);

void NN_Mod(NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int);
void NN_ModMult(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int);
void NN_ModExp(NN_DIGIT *, NN_DIGIT *, NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int);

int NN_Cmp(NN_DIGIT *, NN_DIGIT *, unsigned int);
unsigned int NN_Digits(NN_DIGIT *, unsigned int);

#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;}

// === Digit arithmetic routines ===
void NN_DigitMult(NN_DIGIT [2], NN_DIGIT, NN_DIGIT);
void NN_DigitDiv(NN_DIGIT *, NN_DIGIT [2], NN_DIGIT);


#endif  // _DH_KEY_H

