#include "expand_mq.h"

int ExpandEquations(const uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE], field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)])
{
	int ret = -1;
	uint32_t i, j, nb_eq;
	uint32_t nc[MQOM2_PARAM_MQ_N] = { 0 };
	uint32_t nb[MQOM2_PARAM_MQ_N] = { 0 };
	uint8_t prg_salt[MQOM2_PARAM_SALT_SIZE] = { 0 };
	xof_context xof_ctx;
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

	/* Initialize the PRG cache when used */
	prg_cache = init_prg_cache(nb_eq);

	/* Generate the equations */
	for(i = 0; i < MQOM2_PARAM_MQ_M; i++){
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

	ret = 0;
err:
	if(stream != NULL){
		free(stream);
	}
	destroy_prg_cache(prg_cache);
	return ret;
}
