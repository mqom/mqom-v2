#ifndef __RIJNDAEL_TABLE_H__
#define __RIJNDAEL_TABLE_H__

#include "rijndael_common.h"

/* The general Rijndael core context structure */
typedef struct
{
	rijndael_type rtype; /* Type of Rijndael */
        uint32_t Nr; /* Number of rounds */
        uint32_t Nk; /* Number of words in the key */
        uint32_t Nb; /* Number of words in the block*/
        uint32_t rk[480 / 4];  /* Round keys    */
} rijndael_table_ctx;


/* ==== Public API ==== */
int aes128_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[16]);
int aes256_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[32]);
int rijndael256_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[32]);
int aes128_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int aes256_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16]);
int rijndael256_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[32], uint8_t data_out[32]);
/* x2 and x4 encryption APIs */
int aes128_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes128_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int aes256_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]);
int aes256_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]);
int rijndael256_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2,
                        const uint8_t plainText1[32], const uint8_t plainText2[32],
                        uint8_t cipherText1[32], uint8_t cipherText2[32]);
int rijndael256_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
                uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32]);

#endif /* __RIJNDAEL_TABLE_H__ */
