#include "prg.h"

int PRG(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], uint32_t e, const uint8_t seed[MQOM2_PARAM_SEED_SIZE], uint32_t nbytes, uint8_t *out_data)
{	
	int ret = -1;
	uint32_t i, filled_blocks;
	enc_ctx ctx1, ctx2, ctx3, ctx4;
	uint8_t tweaked_salt[MQOM2_PARAM_SALT_SIZE];
	uint8_t linortho_seed[MQOM2_PARAM_SEED_SIZE];

	/* Compute Psi(seed) once and for all */
	LinOrtho(seed, linortho_seed);

	/* Depending on the number of blocks, exploit the 2x or 4x variants */
	filled_blocks = 0;
	for(i = 0; i < (nbytes / (4 * MQOM2_PARAM_SEED_SIZE)); i++){
		TweakSalt(salt, tweaked_salt, 3, e, filled_blocks);
		ret = enc_key_sched(&ctx1, tweaked_salt); ERR(ret, err);
		TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 1);
		ret = enc_key_sched(&ctx2, tweaked_salt); ERR(ret, err);
		TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 2);
		ret = enc_key_sched(&ctx3, tweaked_salt); ERR(ret, err);
		TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 3);
		ret = enc_key_sched(&ctx4, tweaked_salt); ERR(ret, err);
		ret = enc_encrypt_x4(&ctx1, &ctx2, &ctx3, &ctx4, seed, seed, seed, seed,
			&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)],
			&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)], &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 3)]); ERR(ret, err);
                /* Xor with LinOrtho seed */
                xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
		xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]);
		xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)]);
		xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 3)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 3)]);
		filled_blocks += 4;
	}
	switch((nbytes - (4 * MQOM2_PARAM_SEED_SIZE * i)) / MQOM2_PARAM_SEED_SIZE){
		case 0:{
			/* No remaining block */
			break;
		}
		case 1:{
			/* One remaining block: 1x */
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks);
			ret = enc_key_sched(&ctx1, tweaked_salt); ERR(ret, err);
			ret = enc_encrypt(&ctx1, seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]); ERR(ret, err);
	                /* Xor with LinOrtho seed */
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
			filled_blocks += 1;
			break;
		}
		case 2:{
			/* Two remaining block: 2x */
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks);
			ret = enc_key_sched(&ctx1, tweaked_salt); ERR(ret, err);
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 1);
			ret = enc_key_sched(&ctx2, tweaked_salt); ERR(ret, err);
			ret = enc_encrypt_x2(&ctx1, &ctx2, seed, seed,
				&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]); ERR(ret, err);
	                /* Xor with LinOrtho seed */
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]);
			filled_blocks += 2;
			break;
		}
		case 3:{
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks);
			ret = enc_key_sched(&ctx1, tweaked_salt); ERR(ret, err);
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 1);
			ret = enc_key_sched(&ctx2, tweaked_salt); ERR(ret, err);
			TweakSalt(salt, tweaked_salt, 3, e, filled_blocks + 2);
			ret = enc_key_sched(&ctx3, tweaked_salt); ERR(ret, err);
			/* Three remaining block: 2x and then 1x */
			ret = enc_encrypt_x2(&ctx1, &ctx2, seed, seed,
				&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]); ERR(ret, err);
			ret = enc_encrypt(&ctx3, seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)]); ERR(ret, err);
	                /* Xor with LinOrtho seed */
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]);
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 2)]);
			filled_blocks += 3;
			break;
		}
		default:{
			ret = -1;
			goto err;
		}
	}
	/* Deal with the possible leftover incomplete block */
	if(nbytes % MQOM2_PARAM_SEED_SIZE){
		uint8_t leftover[MQOM2_PARAM_SEED_SIZE];
		TweakSalt(salt, tweaked_salt, 3, e, filled_blocks);
		ret = enc_key_sched(&ctx1, tweaked_salt); ERR(ret, err);
		ret = enc_encrypt(&ctx1, seed, leftover); ERR(ret, err);
                /* Xor with LinOrtho seed */
		xor_blocks(leftover, linortho_seed, leftover);
		memcpy(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], leftover, nbytes % MQOM2_PARAM_SEED_SIZE);
	}
	
	ret = 0;
err:
	return ret;
}
