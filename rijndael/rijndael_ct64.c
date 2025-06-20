#include "rijndael_platform.h"

#if defined(RIJNDAEL_BITSLICE)
#include "rijndael_ct64_enc.h"


/****** API functions *******/
int aes128_ct64_setkey_enc(rijndael_ct64_ctx *ctx, const uint8_t key[16])
{
	return br_aes_ct64_keysched(ctx, key, AES128);
}

int aes256_ct64_setkey_enc(rijndael_ct64_ctx *ctx, const uint8_t key[32])
{
	return br_aes_ct64_keysched(ctx, key, AES256);
}

int rijndael256_ct64_setkey_enc(rijndael_ct64_ctx *ctx, const uint8_t key[32])
{
	return br_aes_ct64_keysched(ctx, key, RIJNDAEL_256_256);
}


// === AES-128 enc
int aes128_ct64_enc(const rijndael_ct64_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16])
{
	if((ctx == NULL) || (ctx->rtype != AES128)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx, NULL, NULL, NULL,
        data_in, NULL, NULL, NULL,
        data_out, NULL, NULL, NULL);
}

int aes128_ct64_enc_x2(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16])
{
	if((ctx1 == NULL) || (ctx1->rtype != AES128)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES128)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx1, ctx2, NULL, NULL,
        plainText1, plainText2, NULL, NULL,
        cipherText1, cipherText2, NULL, NULL);
}

int aes128_ct64_enc_x4(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const rijndael_ct64_ctx *ctx3, const rijndael_ct64_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16])
{
	if((ctx1 == NULL) || (ctx1->rtype != AES128)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES128)){
		return -1;
	}
	if((ctx3 == NULL) || (ctx3->rtype != AES128)){
		return -1;
	}
	if((ctx4 == NULL) || (ctx4->rtype != AES128)){
		return -1;
	}

	return core_ct64_bitslice_encrypt(ctx1, ctx2, ctx3, ctx4,
        plainText1, plainText2, plainText3, plainText4,
        cipherText1, cipherText2, cipherText3, cipherText4);
}

// === AES-256 enc
int aes256_ct64_enc(const rijndael_ct64_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16])
{
	if((ctx == NULL) || (ctx->rtype != AES256)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx, NULL, NULL, NULL,
        data_in, NULL, NULL, NULL,
        data_out, NULL, NULL, NULL);
}

int aes256_ct64_enc_x2(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16])
{
	if((ctx1 == NULL) || (ctx1->rtype != AES256)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES256)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx1, ctx2, NULL, NULL,
        plainText1, plainText2, NULL, NULL,
        cipherText1, cipherText2, NULL, NULL);
}

int aes256_ct64_enc_x4(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const rijndael_ct64_ctx *ctx3, const rijndael_ct64_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16])
{
	if((ctx1 == NULL) || (ctx1->rtype != AES256)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES256)){
		return -1;
	}
	if((ctx3 == NULL) || (ctx3->rtype != AES256)){
		return -1;
	}
	if((ctx4 == NULL) || (ctx4->rtype != AES256)){
		return -1;
	}

	return core_ct64_bitslice_encrypt(ctx1, ctx2, ctx3, ctx4,
        plainText1, plainText2, plainText3, plainText4,
        cipherText1, cipherText2, cipherText3, cipherText4);
}


// === Rijndael-256 enc
int rijndael256_ct64_enc(const rijndael_ct64_ctx *ctx, const uint8_t data_in[32], uint8_t data_out[32])
{
	if((ctx == NULL) || (ctx->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx, NULL, NULL, NULL,
        data_in, NULL, NULL, NULL,
        data_out, NULL, NULL, NULL);
}

int rijndael256_ct64_enc_x2(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const uint8_t plainText1[32], const uint8_t plainText2[32], uint8_t cipherText1[32], uint8_t cipherText2[32])
{
	if((ctx1 == NULL) || (ctx1->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	return core_ct64_bitslice_encrypt(ctx1, ctx2, NULL, NULL,
        plainText1, plainText2, NULL, NULL,
        cipherText1, cipherText2, NULL, NULL);
}

int rijndael256_ct64_enc_x4(const rijndael_ct64_ctx *ctx1, const rijndael_ct64_ctx *ctx2, const rijndael_ct64_ctx *ctx3, const rijndael_ct64_ctx *ctx4,
                const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
                uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32])
{
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	if((ctx2 == NULL) || (ctx2->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	if((ctx3 == NULL) || (ctx3->rtype != RIJNDAEL_256_256)){
		return -1;
	}
	if((ctx4 == NULL) || (ctx4->rtype != RIJNDAEL_256_256)){
		return -1;
	}

	ret = core_ct64_bitslice_encrypt(ctx1, ctx2, NULL, NULL,
        plainText1, plainText2, NULL, NULL,
        cipherText1, cipherText2, NULL, NULL);
	if(ret){
		ret = -1;
		goto err;
	}
	ret = core_ct64_bitslice_encrypt(ctx3, ctx4, NULL, NULL,
        plainText3, plainText4, NULL, NULL,
        cipherText3, cipherText4, NULL, NULL);
	if(ret){
		ret = -1;
		goto err;
	}

	ret = 0;
err:
	return ret;
}

#endif /* RIJNDAEL_BITSLICE */

