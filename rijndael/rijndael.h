#ifndef __RIJNDAEL_H__
#define __RIJNDAEL_H__

#include "rijndael_platform.h"

#if defined(RIJNDAEL_CONSTANT_TIME_REF) && defined(RIJNDAEL_TABLE)
#error "RIJNDAEL_CONSTANT_TIME_REF, RIJNDAEL_TABLE and RIJNDAEL_AES_NI are exclusive!"
#endif
#if defined(RIJNDAEL_CONSTANT_TIME_REF) && defined(RIJNDAEL_AES_NI)
#error "RIJNDAEL_CONSTANT_TIME_REF, RIJNDAEL_TABLE and RIJNDAEL_AES_NI are exclusive!"
#endif
#if defined(RIJNDAEL_TABLE) && defined(RIJNDAEL_AES_NI)
#error "RIJNDAEL_CONSTANT_TIME_REF, RIJNDAEL_TABLE and RIJNDAEL_AES_NI are exclusive!"
#endif

/* === Constant time ref case */
#if defined(RIJNDAEL_CONSTANT_TIME_REF)
#include "rijndael_ref.h"
#define rijndael_ctx rijndael_ref_ctx
#define aes128_setkey_enc aes128_ref_setkey_enc
#define aes256_setkey_enc aes256_ref_setkey_enc
#define rijndael256_setkey_enc rijndael256_ref_setkey_enc
#define aes128_enc aes128_ref_enc
#define aes256_enc aes256_ref_enc
#define rijndael256_enc rijndael256_ref_enc
#define aes128_enc_x2 aes128_ref_enc_x2
#define aes128_enc_x4 aes128_ref_enc_x4
#define aes256_enc_x2 aes256_ref_enc_x2
#define aes256_enc_x4 aes256_ref_enc_x4
#define rijndael256_enc_x2 rijndael256_ref_enc_x2
#define rijndael256_enc_x4 rijndael256_ref_enc_x4
#endif

/* === Non-constant time table case */
#if defined(RIJNDAEL_TABLE)
#include "rijndael_table.h"
#define rijndael_ctx rijndael_table_ctx
#define aes128_setkey_enc aes128_table_setkey_enc
#define aes256_setkey_enc aes256_table_setkey_enc
#define rijndael256_setkey_enc rijndael256_table_setkey_enc
#define aes128_enc aes128_table_enc
#define aes256_enc aes256_table_enc
#define rijndael256_enc rijndael256_table_enc
#define aes128_enc_x2 aes128_table_enc_x2
#define aes128_enc_x4 aes128_table_enc_x4
#define aes256_enc_x2 aes256_table_enc_x2
#define aes256_enc_x4 aes256_table_enc_x4
#define rijndael256_enc_x2 rijndael256_table_enc_x2
#define rijndael256_enc_x4 rijndael256_table_enc_x4
#endif

/* === Constant time ref case */
#if defined(RIJNDAEL_AES_NI)
#include "rijndael_aes_ni.h"
#define rijndael_ctx rijndael_aes_ni_ctx
#define aes128_setkey_enc aes128_aes_ni_setkey_enc
#define aes256_setkey_enc aes256_aes_ni_setkey_enc
#define rijndael256_setkey_enc rijndael256_aes_ni_setkey_enc
#define aes128_enc aes128_aes_ni_enc
#define aes256_enc aes256_aes_ni_enc
#define rijndael256_enc rijndael256_aes_ni_enc
#define aes128_enc_x2 aes128_aes_ni_enc_x2
#define aes128_enc_x4 aes128_aes_ni_enc_x4
#define aes256_enc_x2 aes256_aes_ni_enc_x2
#define aes256_enc_x4 aes256_aes_ni_enc_x4
#define rijndael256_enc_x2 rijndael256_aes_ni_enc_x2
#define rijndael256_enc_x4 rijndael256_aes_ni_enc_x4
#endif

/* ==== Public API ==== */

int aes128_setkey_enc(rijndael_ctx *ctx, const uint8_t key[16]);
int aes256_setkey_enc(rijndael_ctx *ctx, const uint8_t key[32]);
int rijndael256_setkey_enc(rijndael_ctx *ctx, const uint8_t key[32]);
int aes128_enc(const rijndael_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int aes256_enc(const rijndael_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int rijndael256_enc(const rijndael_ctx *ctx, const uint8_t data_in[32], uint8_t data_out[32]);
/* x2 and x4 encryption APIs */
int aes128_enc_x2(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes128_enc_x4(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2, const rijndael_ctx *ctx3, const rijndael_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int aes256_enc_x2(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes256_enc_x4(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2, const rijndael_ctx *ctx3, const rijndael_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int rijndael256_enc_x2(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2,
                        const uint8_t plainText1[32], const uint8_t plainText2[32],
                        uint8_t cipherText1[32], uint8_t cipherText2[32]);
int rijndael256_enc_x4(const rijndael_ctx *ctx1, const rijndael_ctx *ctx2, const rijndael_ctx *ctx3, const rijndael_ctx *ctx4,
                const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
                uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32]);

#endif /* __RIJNDAEL_H__ */
