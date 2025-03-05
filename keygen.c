#include "keygen.h"

/* NOTE: we use multi-dimensional array types to ease usage of indices.
 * While we can use pure local variables, these become too large to fit the stack
 * and heap allocation is needed. */
typedef field_base_elt (*MatrixSetMQ)[MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
typedef field_base_elt (*VectorSetMQ)[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];

int KeyGen(const uint8_t seed_key[2 * MQOM2_PARAM_SEED_SIZE], uint8_t sk[MQOM2_SK_SIZE], uint8_t pk[MQOM2_PK_SIZE])
{
	int ret = -1;
	field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
	field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)];
	/* Byte array representing x */
	uint8_t _x[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)];
	uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE];
	uint32_t i;
	xof_context xof_ctx;
	field_base_elt *_A = NULL;
	field_base_elt *_b = NULL;
	_A = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * MQOM2_PARAM_MQ_N * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	_b = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	MatrixSetMQ A = (MatrixSetMQ)_A;
	VectorSetMQ b = (VectorSetMQ)_b;

	/* Sanity check for field operations */
	if((FIELD_BASE_LOG2_CARD == 1) && ((8 * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)) != MQOM2_PARAM_MQ_N)){
		ret = -1;
		goto err;
	}

	/* Generate x and mseed_eq */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x00", 1); ERR(ret, err);
	ret = xof_update(&xof_ctx, seed_key, 2 * MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
	/* XXX: NOTE: since we are sure that we are byte aligned and there are no endianness issues
 	 * for base fields, we could directly XOF squeeze in x, but for readability we prefer to
 	 * use field_base_parse (i.e. the cost of a memcpy) */
	ret = xof_squeeze(&xof_ctx, _x, sizeof(_x)); ERR(ret, err);
	field_base_parse(_x, MQOM2_PARAM_MQ_N, x);
	ret = xof_squeeze(&xof_ctx, mseed_eq, sizeof(mseed_eq)); ERR(ret, err);

	/* Expand the equations */
	ret = ExpandEquations(mseed_eq, A, b); ERR(ret, err);
	/* Perform the MQ equations computation in y */
	for(i = 0; i < MQOM2_PARAM_MQ_M; i++){
		field_base_elt y_i;
		field_base_elt vect_tmp[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
		field_base_mat_mult((field_base_elt*)A[i], x, vect_tmp, MQOM2_PARAM_MQ_N, TRI_INF);
		y_i = field_base_vect_mult(x, vect_tmp, MQOM2_PARAM_MQ_N) ^ field_base_vect_mult(x, b[i], MQOM2_PARAM_MQ_N);
		field_base_vect_pack(y_i, y, i); 
	}
	/* Serialize the public key */
	memcpy(&pk[0], mseed_eq, 2 * MQOM2_PARAM_SEED_SIZE);
	field_base_serialize(y, MQOM2_PARAM_MQ_M, &pk[2 * MQOM2_PARAM_SEED_SIZE]);
	/* Serialize the secret key */
	memcpy(&sk[0], mseed_eq, 2 * MQOM2_PARAM_SEED_SIZE);
	memcpy(&sk[2 * MQOM2_PARAM_SEED_SIZE], &pk[2 * MQOM2_PARAM_SEED_SIZE], BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_M));
	field_base_serialize(x, MQOM2_PARAM_MQ_N, &sk[(2 * MQOM2_PARAM_SEED_SIZE) + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_M)]);

	ret = 0;
err:
	if(_A){
		free(_A);
	}
	if(_b){
		free(_b);
	}
	return ret;	
}

extern int randombytes(unsigned char* x, unsigned long long xlen);
int crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
	/* Sample the seed key */
	uint8_t seed_key[2 * MQOM2_PARAM_SEED_SIZE];
	randombytes(seed_key, 2 * MQOM2_PARAM_SEED_SIZE);

	/* Run deterministic key generation */
    return KeyGen(seed_key, sk, pk);
}
