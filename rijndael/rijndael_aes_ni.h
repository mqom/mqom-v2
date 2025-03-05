#ifndef __RIJNDAEL_AES_NI_H__
#define __RIJNDAEL_AES_NI_H__

/* First of all check for the support of AES-NI */
#ifndef __AES__
#error "Error: asking for RIJNDAEL_AES_NI while AES-NI support is not detected ... Please check your platform and/or compilation options!"
#endif /* __AES__ */

#include "rijndael_common.h"

/* Header for AES-NI intrinsics */
#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

/* The general Rijndael core context structure */
typedef struct
{
	rijndael_type rtype; /* Type of Rijndael */
	/* AES-NI context using aligned __m128i */
	/* NOTE: __m128i type should align on 16 bytes and avoid unalignment issues with xmm memory accesses */
	__m128i rk[30]; /* Round keys */
} rijndael_aes_ni_ctx;


/* ==== Public API ==== */
int aes128_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t key[16]);
int aes256_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t key[32]);
int rijndael256_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t key[32]);
int aes128_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int aes256_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int rijndael256_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t data_in[32], uint8_t data_out[32]);
/* x2 and x4 encryption APIs */
int aes128_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes128_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int aes256_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes256_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int rijndael256_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2,
                        const uint8_t plainText1[32], const uint8_t plainText2[32],
                        uint8_t cipherText1[32], uint8_t cipherText2[32]);
int rijndael256_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
                const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
                uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32]);
#endif /* __RIJNDAEL_AES_NI_H__ */
