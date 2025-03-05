#ifndef __ENC_H__
#define __ENC_H__

/* MQOM2 parameters */
#include "mqom2_parameters.h"
/* Our block encryption is based on Rijndael variants */
#include "rijndael.h"
/* Common helpers */
#include "common.h"

/* An encryption context is simply a Rijndael context in all the cases */
#define enc_ctx rijndael_ctx

/* === 128 bits security === */
#if MQOM2_PARAM_SECURITY == 128

/* For 128 bits security, we transparently use AES-128 */
#define enc_key_sched aes128_setkey_enc
#define enc_encrypt aes128_enc
#define enc_encrypt_x2 aes128_enc_x2
#define enc_encrypt_x4 aes128_enc_x4

/* === 192 bits security === */
#elif MQOM2_PARAM_SECURITY == 192

/* Specifically for 192 bits, we need to adapt the underlying calls
 * as we use Rijndael-256-256 with expansion and truncation
 * */
static inline int enc_key_sched(enc_ctx *ctx, const uint8_t key[24])
{
	/* Key expansion key || O^64 */
	uint8_t exp_key[32] = { 0 };
	memcpy(exp_key, key, 24);

	/* Execute the Rijndael-256-256 key schedule */
	return rijndael256_setkey_enc(ctx, exp_key);
}

static inline int enc_encrypt(const enc_ctx *ctx, const uint8_t pt[24], uint8_t ct[24])
{
	int ret = -1;

	/* Plaintext expansion pt || 0^64 */
	uint8_t exp_pt[32] = { 0 }, exp_ct[32];
	memcpy(exp_pt, pt, 24);

	/* Encrypt */
	ret = rijndael256_enc(ctx, exp_pt, exp_ct); ERR(ret, err);

	/* Truncate the output */
	memcpy(ct, exp_ct, 24);

	ret = 0;
err:
	return ret;
}

static inline int enc_encrypt_x2(const enc_ctx *ctx1, const enc_ctx *ctx2, const uint8_t pt1[24], const uint8_t pt2[24], uint8_t ct1[24], uint8_t ct2[24])
{
	int ret = -1;

	/* Plaintext expansion pt || 0^64 */
	uint8_t exp_pt[2][32] = { 0 }, exp_ct[2][32];
	memcpy(exp_pt[0], pt1, 24);
	memcpy(exp_pt[1], pt2, 24);
	/* Encrypt */
	ret = rijndael256_enc_x2(ctx1, ctx2, exp_pt[0], exp_pt[1], exp_ct[0], exp_ct[1]); ERR(ret, err);

	/* Truncate the output */
	memcpy(ct1, exp_ct[0], 24);
	memcpy(ct2, exp_ct[1], 24);

	ret = 0;
err:
	return ret;
}

static inline int enc_encrypt_x4(const enc_ctx *ctx1, const enc_ctx *ctx2, const enc_ctx *ctx3, const enc_ctx *ctx4,
				const uint8_t pt1[24], const uint8_t pt2[24], const uint8_t pt3[24], const uint8_t pt4[24],
				uint8_t ct1[24], uint8_t ct2[24], uint8_t ct3[24], uint8_t ct4[24])
{
	int ret = -1;

	/* Plaintext expansion pt || 0^64 */
	uint8_t exp_pt[4][32] = { 0 }, exp_ct[4][32];
	memcpy(exp_pt[0], pt1, 24);
	memcpy(exp_pt[1], pt2, 24);
	memcpy(exp_pt[2], pt3, 24);
	memcpy(exp_pt[3], pt4, 24);

	/* Encrypt */
	ret = rijndael256_enc_x4(ctx1, ctx2, ctx3, ctx4, exp_pt[0], exp_pt[1], exp_pt[2], exp_pt[3], exp_ct[0], exp_ct[1], exp_ct[2], exp_ct[3]);
	ERR(ret, err);

	/* Truncate the output */
	memcpy(ct1, exp_ct[0], 24);
	memcpy(ct2, exp_ct[1], 24);
	memcpy(ct3, exp_ct[2], 24);
	memcpy(ct4, exp_ct[3], 24);

	ret = 0;
err:
	return ret;
}

/* === 256 bits security === */
#elif MQOM2_PARAM_SECURITY == 256

/* For 256 bits security, we tyransparently use Rijndael-256-256 */
#define enc_key_sched rijndael256_setkey_enc
#define enc_encrypt rijndael256_enc
#define enc_encrypt_x2 rijndael256_enc_x2
#define enc_encrypt_x4 rijndael256_enc_x4

#else

#error "Sorry, unsupported security parameters: must be one of 128, 192, 256"
#endif


#endif /* __ENC_H__ */
