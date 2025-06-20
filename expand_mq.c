#include "expand_mq.h"

int ExpandEquations(const uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE], field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)])
{
	int ret = -1;
	uint32_t i, j, nb_eq;
	uint32_t nc[MQOM2_PARAM_MQ_N] = { 0 };
	uint32_t nb[MQOM2_PARAM_MQ_N] = { 0 };
	uint8_t prg_salt[MQOM2_PARAM_SALT_SIZE] = { 0 };
	uint8_t *stream = NULL;

	prg_key_sched_cache *prg_cache = NULL;

	/* Compute the number of PRG bytes */
	nb_eq = 0;
	for(j = 0; j < MQOM2_PARAM_MQ_N; j++){
		nc[j] = (j + 1) * FIELD_BASE_LOG2_CARD;
		nb[j] = (nc[j] % 8) ? (nc[j] / 8) + 1 : (nc[j] / 8); 
		nb_eq += nb[j];
	}
	nb_eq += nb[MQOM2_PARAM_MQ_N - 1];
	stream = (uint8_t*)malloc(nb_eq * sizeof(uint8_t));
	if(stream == NULL){
		ret = -1;
		goto err;
	}

	/* Initialize the PRG cache when used */
	prg_cache = init_prg_cache(nb_eq);

	/* Generate the equations */
#if defined(USE_XOF_X4)
	/* When using X4 XOF, we */
	if((MQOM2_PARAM_MQ_M < 4) || (MQOM2_PARAM_MQ_M % 4)){
		/* In all our instances, m should be a multiple of 4:
 		 * no leftover should remain for XOF X4. If this is 
 		 * not the case, return an error. */
		ret = -1;
		goto err;
	}
	for(i = 3; i < MQOM2_PARAM_MQ_M; i+=4){
		xof_context_x4 xof_ctx;
		uint32_t k, z;
		uint8_t i_16[4][2];
		uint8_t seed_eq[4][MQOM2_PARAM_SEED_SIZE];
		const uint8_t *constant_1[4] = { (const uint8_t*) "\x01", (const uint8_t*) "\x01", (const uint8_t*) "\x01", (const uint8_t*) "\x01" };
		const uint8_t *mseed_eq_ptr[4] = { mseed_eq, mseed_eq, mseed_eq, mseed_eq };
		const uint8_t *i_16_ptr[4] = { i_16[0], i_16[1], i_16[2], i_16[3] };
		uint8_t *seed_eq_ptr[4] = { seed_eq[0], seed_eq[1], seed_eq[2], seed_eq[3] };
		for(z = 0; z < 4; z++){
			i_16[z][0] = ((i-z) & 0xff);
			i_16[z][1] = (((i-z) >> 8) & 0xff);
		}
		ret = xof_init_x4(&xof_ctx); ERR(ret, err);
		ret = xof_update_x4(&xof_ctx, constant_1, 1); ERR(ret, err);
		ret = xof_update_x4(&xof_ctx, mseed_eq_ptr, 2 * MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
		ret = xof_update_x4(&xof_ctx, i_16_ptr, 2); ERR(ret, err);
		ret = xof_squeeze_x4(&xof_ctx, seed_eq_ptr, MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
		for(z = 0; z < 4; z++){
			ret = PRG(prg_salt, 0, seed_eq[z], nb_eq, stream, prg_cache); ERR(ret, err);
			k = 0;
			for(j = 0; j < MQOM2_PARAM_MQ_N; j++){
				/* Fill the jth row of Ai */
				memset(A[i-z][j], 0, FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
				/* NOTE: the number of elements to parse is (nc[j] / FIELD_BASE_LOG2_CARD),
 				 * the number of bits divided by the size in bits of base field elements */
				field_base_parse(&stream[k], (nc[j] / FIELD_BASE_LOG2_CARD), A[i-z][j]);
				k += nb[j];
			}
			/* Fill bi */
			field_base_parse(&stream[k], MQOM2_PARAM_MQ_N, b[i-z]);
		}
	}
#else
	for(i = 0; i < MQOM2_PARAM_MQ_M; i++){
		xof_context xof_ctx;
		uint32_t k;
		uint8_t i_16[2];
		uint8_t seed_eq[MQOM2_PARAM_SEED_SIZE];
		i_16[0] = (i & 0xff);
		i_16[1] = ((i >> 8) & 0xff);
		ret = xof_init(&xof_ctx); ERR(ret, err);
		ret = xof_update(&xof_ctx, (const uint8_t*) "\x01", 1); ERR(ret, err);
		ret = xof_update(&xof_ctx, mseed_eq, 2 * MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
		ret = xof_update(&xof_ctx, i_16, sizeof(i_16)); ERR(ret, err);
		ret = xof_squeeze(&xof_ctx, seed_eq, MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
		ret = PRG(prg_salt, 0, seed_eq, nb_eq, stream, prg_cache); ERR(ret, err);
		k = 0;
		for(j = 0; j < MQOM2_PARAM_MQ_N; j++){
			/* Fill the jth row of Ai */
			memset(A[i][j], 0, FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
			/* NOTE: the number of elements to parse is (nc[j] / FIELD_BASE_LOG2_CARD),
 			 * the number of bits divided by the size in bits of base field elements */
			field_base_parse(&stream[k], (nc[j] / FIELD_BASE_LOG2_CARD), A[i][j]);
			k += nb[j];
		}
		/* Fill bi */
		field_base_parse(&stream[k], MQOM2_PARAM_MQ_N, b[i]);
	}
#endif

	ret = 0;
err:
	if(stream != NULL){
		free(stream);
	}
	destroy_prg_cache(prg_cache);
	return ret;
}
