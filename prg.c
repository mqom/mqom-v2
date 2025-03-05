#include "prg.h"

static inline int prg_key_sched(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], uint32_t e, uint32_t i, enc_ctx *ctx, prg_key_sched_cache *cache)
{
	int ret = -1;
	uint8_t tweaked_salt[MQOM2_PARAM_SALT_SIZE];

	if(is_entry_active(cache, i)){
		/* The cache line is active, get the value and return */
		get_entry(cache, i, ctx);
	}
	else{
		/* The cache line is inactive: perform the computation and fill it */
		/* Tweak the salt and perform the key schedule */
		TweakSalt(salt, tweaked_salt, 3, e, i);
		ret = enc_key_sched(ctx, tweaked_salt); ERR(ret, err);
		set_entry(cache, i, ctx);
	}

	ret = 0;
err:
	return ret;
}

int PRG(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], uint32_t e, const uint8_t seed[MQOM2_PARAM_SEED_SIZE], uint32_t nbytes, uint8_t *out_data, prg_key_sched_cache *cache)
{	
	int ret = -1;
	uint32_t i, filled_blocks;
	enc_ctx ctx1, ctx2, ctx3, ctx4;
	uint8_t linortho_seed[MQOM2_PARAM_SEED_SIZE];

	/* Compute Psi(seed) once and for all */
	LinOrtho(seed, linortho_seed);

	/* Depending on the number of blocks, exploit the 2x or 4x variants */
	filled_blocks = 0;
	for(i = 0; i < (nbytes / (4 * MQOM2_PARAM_SEED_SIZE)); i++){
		/* Key schedule */
		ret = prg_key_sched(salt, e, filled_blocks    , &ctx1, cache); ERR(ret, err);
		ret = prg_key_sched(salt, e, filled_blocks + 1, &ctx2, cache); ERR(ret, err);
		ret = prg_key_sched(salt, e, filled_blocks + 2, &ctx3, cache); ERR(ret, err);
		ret = prg_key_sched(salt, e, filled_blocks + 3, &ctx4, cache); ERR(ret, err);
		/* Encryption */
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
			ret = prg_key_sched(salt, e, filled_blocks    , &ctx1, cache); ERR(ret, err);
			ret = enc_encrypt(&ctx1, seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]); ERR(ret, err);
	                /* Xor with LinOrtho seed */
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
			filled_blocks += 1;
			break;
		}
		case 2:{
			/* Two remaining block: 2x */
			ret = prg_key_sched(salt, e, filled_blocks    , &ctx1, cache); ERR(ret, err);
			ret = prg_key_sched(salt, e, filled_blocks + 1, &ctx2, cache); ERR(ret, err);
			ret = enc_encrypt_x2(&ctx1, &ctx2, seed, seed,
				&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]); ERR(ret, err);
	                /* Xor with LinOrtho seed */
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks]);
			xor_blocks(&out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)], linortho_seed, &out_data[MQOM2_PARAM_SEED_SIZE * (filled_blocks + 1)]);
			filled_blocks += 2;
			break;
		}
		case 3:{
			ret = prg_key_sched(salt, e, filled_blocks    , &ctx1, cache); ERR(ret, err);
			ret = prg_key_sched(salt, e, filled_blocks + 1, &ctx2, cache); ERR(ret, err);
			ret = prg_key_sched(salt, e, filled_blocks + 2, &ctx3, cache); ERR(ret, err);
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
		ret = prg_key_sched(salt, e, filled_blocks    , &ctx1, cache); ERR(ret, err);
		ret = enc_encrypt(&ctx1, seed, leftover); ERR(ret, err);
                /* Xor with LinOrtho seed */
		xor_blocks(leftover, linortho_seed, leftover);
		memcpy(&out_data[MQOM2_PARAM_SEED_SIZE * filled_blocks], leftover, nbytes % MQOM2_PARAM_SEED_SIZE);
	}
	
	ret = 0;
err:
	return ret;
}
