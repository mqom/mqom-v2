#ifdef SUPERCOP
#include "crypto_sign.h"
#else
#include "api.h"
#endif

#include <stdlib.h>
#include "common.h"
#include "fields.h"
#include "xof.h"
#include "expand_mq.h"
#include "blc.h"
#include "piop.h"

extern int randombytes(unsigned char* x, unsigned long long xlen);

/* NOTE: we use multi-dimensional array types to ease usage of indices.
 * While we can use pure local variables, these become too large to fit the stack
 * and heap allocation is needed. */
typedef field_base_elt (*MatrixSetMQ)[MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
typedef field_base_elt (*VectorSetMQ)[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];

int SampleChallenge(uint8_t hash[MQOM2_PARAM_DIGEST_SIZE], uint16_t i_star[MQOM2_PARAM_TAU], uint8_t nonce[4])
{
    int ret = -1;
    int e;
	xof_context xof_ctx;
    uint32_t nonce_int = 0;

    uint16_t val = 1;
    uint8_t tmp[MQOM2_PARAM_TAU*2+2];
    while(val != 0) {
        nonce[0] = nonce_int & 0xff;
        nonce[1] = (nonce_int>> 8) & 0xff;
        nonce[2] = (nonce_int>>16) & 0xff;
        nonce[3] = (nonce_int>>24) & 0xff;
        ret = xof_init(&xof_ctx); ERR(ret, err);
	    ret = xof_update(&xof_ctx, (const uint8_t*) "\x05", 1); ERR(ret, err);
        ret = xof_update(&xof_ctx, hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
        ret = xof_update(&xof_ctx, nonce, 4); ERR(ret, err);
        ret = xof_squeeze(&xof_ctx, tmp, MQOM2_PARAM_TAU*2+2); ERR(ret, err);
        for(e = 0; e<MQOM2_PARAM_TAU; e++) {
            i_star[e] = (tmp[2*e] + 256*tmp[2*e+1]) & ((1<<MQOM2_PARAM_NB_EVALS_LOG)-1);
        }
        val = (tmp[2*MQOM2_PARAM_TAU] + tmp[2*MQOM2_PARAM_TAU+1]*256) & ((1<<MQOM2_PARAM_W)-1);
        nonce_int++;
    }

    ret = 0;
err:
	return ret;
}

int Sign(const uint8_t sk[MQOM2_SK_SIZE], const uint8_t *msg, unsigned long long mlen, const uint8_t salt[MQOM2_PARAM_SALT_SIZE], const uint8_t mseed[MQOM2_PARAM_SEED_SIZE], uint8_t sig[MQOM2_SIG_SIZE]) {
    int ret = -1;
    int e;
	uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE];
	uint8_t msg_hash[MQOM2_PARAM_DIGEST_SIZE], hash[MQOM2_PARAM_DIGEST_SIZE];
	field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
	field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)];
	xof_context xof_ctx;
	field_base_elt *_A = NULL;
	field_base_elt *_b = NULL;
	_A = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * MQOM2_PARAM_MQ_N * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	_b = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	MatrixSetMQ A = (MatrixSetMQ)_A;
	VectorSetMQ b = (VectorSetMQ)_b;

	/* Parse the secret key */
	memcpy(mseed_eq, &sk[0], 2 * MQOM2_PARAM_SEED_SIZE);
    const uint8_t* pk = &sk[0];
	field_base_parse(&sk[2 * MQOM2_PARAM_SEED_SIZE], MQOM2_PARAM_MQ_M, y);
	field_base_parse(&sk[(2 * MQOM2_PARAM_SEED_SIZE) + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_M)], MQOM2_PARAM_MQ_N, x);

	/* Expand the equations */
	ret = ExpandEquations(mseed_eq, A, b); ERR(ret, err);

    /* Hash message */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x02", 1); ERR(ret, err);
    ret = xof_update(&xof_ctx, msg, mlen); ERR(ret, err);
    ret = xof_squeeze(&xof_ctx, msg_hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    /* Prepare the signature */
    unsigned int pos = 0;
    memcpy(&sig[pos], salt, MQOM2_PARAM_SALT_SIZE); pos += MQOM2_PARAM_SALT_SIZE;
    uint8_t *com1 = &sig[pos]; pos += MQOM2_PARAM_DIGEST_SIZE;
    uint8_t *com2 = &sig[pos]; pos += MQOM2_PARAM_DIGEST_SIZE;
    uint8_t *seriliared_alpha1 = &sig[pos]; pos += MQOM2_PARAM_TAU*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU);
    uint8_t *opening = &sig[pos];
    uint8_t *nonce = &sig[MQOM2_SIG_SIZE-4];

    /* Commit Lines */
    blc_key_t key;
    field_ext_elt x0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt u0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    field_ext_elt u1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    BLC_Commit(mseed, salt, x, com1, &key, x0, u0, u1);
    
    /* Compute P_alpha */
    field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    ComputePAlpha(com1, x0, u0, u1, x, A, b, alpha0, alpha1);

    /* Hash P_alpha and compute Fiat-Shamir hash */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x03", 1); ERR(ret, err);
    uint8_t alpha[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)];
    for(e=0; e<MQOM2_PARAM_TAU; e++) {
        field_ext_serialize(alpha0[e], MQOM2_PARAM_ETA, alpha);
	    ret = xof_update(&xof_ctx, alpha, BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)); ERR(ret, err);
    }
    for(e=0; e<MQOM2_PARAM_TAU; e++) {
        uint8_t* buffer = &seriliared_alpha1[e*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)];
        field_ext_serialize(alpha1[e], MQOM2_PARAM_ETA, buffer);
	    ret = xof_update(&xof_ctx, buffer, BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)); ERR(ret, err);
    }
	ret = xof_squeeze(&xof_ctx, com2, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x04", 1); ERR(ret, err);
    ret = xof_update(&xof_ctx, pk, MQOM2_PK_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, com1, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, com2, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, msg_hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
	ret = xof_squeeze(&xof_ctx, hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    /* Sample Challenge */
    uint16_t i_star[MQOM2_PARAM_TAU];
    ret = SampleChallenge(hash, i_star, nonce);

    /* Open Line Evaluation */
    BLC_Open(&key, i_star, opening);

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

int crypto_sign_signature(uint8_t *sig,
                          unsigned long long *siglen,
                          const uint8_t *m,
                          unsigned long long mlen,
                          const uint8_t *sk)
{
    int ret = -1;

    // Sample salt
    uint8_t salt[MQOM2_PARAM_SALT_SIZE];
    ret = randombytes(salt, MQOM2_PARAM_SALT_SIZE); ERR(ret, err);

    // Sample seed
    uint8_t mseed[MQOM2_PARAM_SEED_SIZE];
    ret = randombytes(mseed, MQOM2_PARAM_SEED_SIZE); ERR(ret, err);

    // Build the signature
    ret = Sign(sk, m, mlen, salt, mseed, sig); ERR(ret, err);
    if(siglen != NULL) {
        *siglen = (unsigned long long) MQOM2_SIG_SIZE;
    }

    ret = 0;
err:
    return ret;
}

int crypto_sign(
        unsigned char *sm, unsigned long long *smlen,
        const unsigned char *m, unsigned long long mlen,
        const unsigned char *sk)
{
    int ret = -1;

    uint8_t* message = sm;
    memmove(message, m, mlen);
    ret = crypto_sign_signature(sm+mlen, smlen, message, mlen, sk); ERR(ret, err);
    if(smlen != NULL) {
        *smlen += mlen;
    }

    ret = 0;
err:
    return ret;
}

int Verify(const uint8_t pk[MQOM2_PK_SIZE], const uint8_t *msg, unsigned long long mlen, const uint8_t sig[MQOM2_SIG_SIZE]) {
    int ret = -1;
    int e;
	uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE];
	uint8_t msg_hash[MQOM2_PARAM_DIGEST_SIZE], hash[MQOM2_PARAM_DIGEST_SIZE], com2_[MQOM2_PARAM_DIGEST_SIZE];
	field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)];
	xof_context xof_ctx;
	field_base_elt *_A = NULL;
	field_base_elt *_b = NULL;
	_A = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * MQOM2_PARAM_MQ_N * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	_b = (field_base_elt*)malloc(MQOM2_PARAM_MQ_M * FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N) * sizeof(field_base_elt));
	MatrixSetMQ A = (MatrixSetMQ)_A;
	VectorSetMQ b = (VectorSetMQ)_b;

	/* Parse the public key */
	memcpy(mseed_eq, &pk[0], 2 * MQOM2_PARAM_SEED_SIZE);
	field_base_parse(&pk[2 * MQOM2_PARAM_SEED_SIZE], MQOM2_PARAM_MQ_M, y);

	/* Expand the equations */
	ret = ExpandEquations(mseed_eq, A, b); ERR(ret, err);

    /* Parse the signature */
    unsigned int pos = 0;
    const uint8_t *salt = &sig[pos]; pos += MQOM2_PARAM_SALT_SIZE;
    const uint8_t *com1 = &sig[pos]; pos += MQOM2_PARAM_DIGEST_SIZE;
    const uint8_t *com2 = &sig[pos]; pos += MQOM2_PARAM_DIGEST_SIZE;
    const uint8_t *seriliared_alpha1 = &sig[pos]; pos += MQOM2_PARAM_TAU*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU);
    const uint8_t *opening = &sig[pos];
    const uint8_t *nonce = &sig[MQOM2_SIG_SIZE-4];

    /* Hash message */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x02", 1); ERR(ret, err);
    ret = xof_update(&xof_ctx, msg, mlen); ERR(ret, err);
    ret = xof_squeeze(&xof_ctx, msg_hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    /* Compute Fiat-Shamir hash */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x04", 1); ERR(ret, err);
    ret = xof_update(&xof_ctx, pk, MQOM2_PK_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, com1, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, com2, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, msg_hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
	ret = xof_squeeze(&xof_ctx, hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    /* Sample Challenge */
    uint16_t i_star[MQOM2_PARAM_TAU];
    uint8_t tmp[MQOM2_PARAM_TAU*2+2];
    ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x05", 1); ERR(ret, err);
    ret = xof_update(&xof_ctx, hash, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_update(&xof_ctx, nonce, 4); ERR(ret, err);
    ret = xof_squeeze(&xof_ctx, tmp, MQOM2_PARAM_TAU*2+2); ERR(ret, err);
    for(e = 0; e<MQOM2_PARAM_TAU; e++) {
        i_star[e] = (tmp[2*e] + 256*tmp[2*e+1]) & ((1<<MQOM2_PARAM_NB_EVALS_LOG)-1);
    }
    uint16_t val = (tmp[2*MQOM2_PARAM_TAU] + tmp[2*MQOM2_PARAM_TAU+1]*256) & ((1<<MQOM2_PARAM_W)-1);
    if(val != 0) {
        ret = -1;
        goto err;
    }

    /* Get Opened Evaluations */
    field_ext_elt x_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt u_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    ret = BLC_Eval(salt, com1, opening, i_star, x_eval, u_eval); ERR(ret, err);
    
    /* Recompute P_alpha */
    field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    for(e=0; e<MQOM2_PARAM_TAU; e++) {
         field_ext_parse(&seriliared_alpha1[e*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)], MQOM2_PARAM_ETA, alpha1[e]);
    }
    ret = RecomputePAlpha(com1, alpha1, i_star, x_eval, u_eval, A, b, y, alpha0); ERR(ret, err);

    /* Hash P_alpha */
	ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x03", 1); ERR(ret, err);
    uint8_t alpha[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)];
    for(e=0; e<MQOM2_PARAM_TAU; e++) {
        field_ext_serialize(alpha0[e], MQOM2_PARAM_ETA, alpha);
	    ret = xof_update(&xof_ctx, alpha, BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)); ERR(ret, err);
    }
    ret = xof_update(&xof_ctx, seriliared_alpha1, MQOM2_PARAM_TAU*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)); ERR(ret, err);
	ret = xof_squeeze(&xof_ctx, com2_, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    if(memcmp(com2, com2_, MQOM2_PARAM_DIGEST_SIZE)) {
        ret = -1;
        goto err;
    }

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

int crypto_sign_verify(const uint8_t *sig,
                       unsigned long long siglen,
                       const uint8_t *m,
                       unsigned long long mlen,
                       const uint8_t *pk)
{
    if(siglen != (unsigned long long) MQOM2_SIG_SIZE)
        return -1;

    return Verify(pk, m, mlen, sig);
}

int crypto_sign_open(
        unsigned char *m, unsigned long long *mlen,
        const unsigned char *sm, unsigned long long smlen,
        const unsigned char *pk) {

    if(smlen < (unsigned long long) MQOM2_SIG_SIZE)
        return -1;

    const uint8_t* message = sm;
    *mlen = smlen-(unsigned long long) MQOM2_SIG_SIZE;

    int ret = crypto_sign_verify(sm+(*mlen), MQOM2_SIG_SIZE, message, *mlen, pk);
    if (ret) {
        return ret;
    }

    memmove(m, message, *mlen);
    return 0;
}
