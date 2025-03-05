#include "piop.h"
#if MQOM2_PARAM_WITH_STATISTICAL_BATCHING == 1
#include "xof.h"
#endif
#include "piop_cache.h"
#include "benchmark.h"

static inline void compute_t1(const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_base_elt t1[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], uint32_t i, piop_cache *cache)
{	
	if(is_entry_active(cache, i)){
		get_entry(cache, i, t1);
	}
	else{
		field_base_mat_mult((field_base_elt*)A[i], x, t1, MQOM2_PARAM_MQ_N, TRI_INF);
		field_base_vect_add(t1, b[i], t1, MQOM2_PARAM_MQ_N);
		set_entry(cache, i, t1);
	}

	return;
}

int ComputePz(const field_ext_elt x0_e[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_ext_elt z0[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)], field_ext_elt z1[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)], piop_cache *cache) {
	int ret = -1;
    uint32_t i;

    field_ext_elt t0[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_base_elt t1[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt z_0i, z_1i;

	for(i = 0; i < MQOM2_PARAM_MQ_M; i++) {
        /* Compute P_t(X) = t_0 + t_1 X = A_i P_x(X) + b_i X */
        __BENCHMARK_START__(BS_PIOP_MAT_MUL_EXT);
        field_base_ext_mat_mult((field_base_elt*)A[i], x0_e, t0, MQOM2_PARAM_MQ_N, TRI_INF);
        __BENCHMARK_STOP__(BS_PIOP_MAT_MUL_EXT);
        __BENCHMARK_START__(BS_PIOP_COMPUTE_T1);
	compute_t1(A, x, b, t1, i, cache);
        __BENCHMARK_STOP__(BS_PIOP_COMPUTE_T1);

        /* Compute P_{z,i}(X) = z_{0,i} + z_{1,i} X = P_t(X)^T P_x(X) - y_i X^2 */
        __BENCHMARK_START__(BS_PIOP_COMPUTE_PZI);
        z_0i = field_ext_vect_mult(t0, x0_e, MQOM2_PARAM_MQ_N);
        z_1i = field_ext_base_vect_mult(t0, x, MQOM2_PARAM_MQ_N) ^ field_base_ext_vect_mult(t1, x0_e, MQOM2_PARAM_MQ_N);
        field_ext_vect_pack(z_0i, z0, i);
        field_ext_vect_pack(z_1i, z1, i);
        __BENCHMARK_STOP__(BS_PIOP_COMPUTE_PZI);
    }

	ret = 0;
	return ret;
}

int ComputePzEval(field_ext_elt r, const field_ext_elt v_x[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)], field_ext_elt v_z[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)]) {
	int ret = -1;
    uint32_t i;

    field_ext_elt v_t[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt tmp[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt v_zi;

    field_ext_elt r2 = field_ext_mult(r, r);
    field_ext_elt y_r2[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)];
    field_ext_base_constant_vect_mult(r2, y, y_r2, MQOM2_PARAM_MQ_M);

	for(i = 0; i < MQOM2_PARAM_MQ_M; i++) {
        /* Compute v_t = P_t(r) = A_i P_x(r) + b_i r */
        field_base_ext_mat_mult((field_base_elt*)A[i], v_x, tmp, MQOM2_PARAM_MQ_N, TRI_INF);
        field_ext_base_constant_vect_mult(r, b[i], v_t, MQOM2_PARAM_MQ_N);
        field_ext_vect_add(v_t, tmp, v_t, MQOM2_PARAM_MQ_N);

        /* Compute v_{z,i} = P_{z,i}(r) = v_t^T v_r - y_i r^2 */
        v_zi = field_ext_vect_mult(v_t, v_x, MQOM2_PARAM_MQ_N);
        field_ext_vect_pack(v_zi, v_z, i);
    }
    field_ext_vect_add(v_z, y_r2, v_z, MQOM2_PARAM_MQ_M);

	ret = 0;
	return ret;	
}

void phi(const field_ext_elt in[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)], field_ext_elt out[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)])
{
    uint32_t i;
    field_ext_elt out_i;
    for(i = 0; i<MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU; i++) {
        out_i = field_ext_vect_mult(&in[i*FIELD_EXT_PACKING(MQOM2_PARAM_MU)], ext_basis, MQOM2_PARAM_MU);
		field_ext_vect_pack(out_i, out, i); 
    }
}

int ComputePAlpha(const uint8_t com[MQOM2_PARAM_DIGEST_SIZE], const field_ext_elt x0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_ext_elt u0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_ext_elt u1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)])
{
	int ret = -1;
    uint32_t e;

    /* Initialize the PIOP cache for t1 */
    piop_cache *t1_cache = init_piop_cache(MQOM2_PARAM_MQ_M);

    __BENCHMARK_START__(BS_PIOP_EXPAND_BATCHING_MAT);
#if MQOM2_PARAM_WITH_STATISTICAL_BATCHING == 1
    uint32_t i;
	xof_context xof_ctx;
    field_ext_elt Gamma[MQOM2_PARAM_ETA][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
    uint8_t stream[MQOM2_PARAM_ETA*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
    ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x08", 1); ERR(ret, err);
	ret = xof_update(&xof_ctx, com, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_squeeze(&xof_ctx, stream, MQOM2_PARAM_ETA*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU));
    for(i=0; i<MQOM2_PARAM_ETA; i++)
        field_ext_parse(&stream[i*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)], MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU, Gamma[i]);

    field_ext_elt compressed_z[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
#else
    (void) com;
#endif
    __BENCHMARK_STOP__(BS_PIOP_EXPAND_BATCHING_MAT);

    field_ext_elt z0[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)], z1[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)];
	for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        ret = ComputePz(x0[e], x, A, b, z0, z1, t1_cache); ERR(ret, err);
        __BENCHMARK_START__(BS_PIOP_BATCH);
#if MQOM2_PARAM_WITH_STATISTICAL_BATCHING == 1
        phi(z0, compressed_z);
        for(i=0; i<MQOM2_PARAM_ETA; i++) {
            field_ext_vect_pack(
                field_ext_vect_mult(Gamma[i], compressed_z, MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU),
                alpha0[e], i
            );
        }
        phi(z1, compressed_z);
        for(i=0; i<MQOM2_PARAM_ETA; i++) {
            field_ext_vect_pack(
                field_ext_vect_mult(Gamma[i], compressed_z, MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU),
                alpha1[e], i
            );
        }
#else
        phi(z0, alpha0[e]);
        phi(z1, alpha1[e]);
#endif
        __BENCHMARK_STOP__(BS_PIOP_BATCH);
        __BENCHMARK_START__(BS_PIOP_ADD_MASKS);
        field_ext_vect_add(alpha0[e], u0[e], alpha0[e], MQOM2_PARAM_ETA);
        field_ext_vect_add(alpha1[e], u1[e], alpha1[e], MQOM2_PARAM_ETA);
        __BENCHMARK_STOP__(BS_PIOP_ADD_MASKS);
    }

	ret = 0;
err:
	destroy_piop_cache(t1_cache);
	return ret;
}

int RecomputePAlpha(const uint8_t com[MQOM2_PARAM_DIGEST_SIZE], const field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const uint16_t i_star[MQOM2_PARAM_TAU], const field_ext_elt x_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_ext_elt u_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)], field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)])
{
	int ret = -1;
    uint32_t e;

#if MQOM2_PARAM_WITH_STATISTICAL_BATCHING == 1
    uint32_t i;
	xof_context xof_ctx;
    field_ext_elt Gamma[MQOM2_PARAM_ETA][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
    uint8_t stream[MQOM2_PARAM_ETA*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
    ret = xof_init(&xof_ctx); ERR(ret, err);
	ret = xof_update(&xof_ctx, (const uint8_t*) "\x08", 1); ERR(ret, err);
	ret = xof_update(&xof_ctx, com, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    ret = xof_squeeze(&xof_ctx, stream, MQOM2_PARAM_ETA*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU));
    for(i=0; i<MQOM2_PARAM_ETA; i++)
        field_ext_parse(&stream[i*BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)], MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU, Gamma[i]);

    field_ext_elt compressed_v_z[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU)];
#else
    (void) com;
#endif

    field_ext_elt v_z[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_M)];
    field_ext_elt v_alpha[FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
	for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        field_ext_elt r = get_evaluation_point(i_star[e]);
        ret = ComputePzEval(r, x_eval[e], A, b, y, v_z); ERR(ret, err);
#if MQOM2_PARAM_WITH_STATISTICAL_BATCHING == 1
        phi(v_z, compressed_v_z);
        for(i=0; i<MQOM2_PARAM_ETA; i++) {
            field_ext_vect_pack(
                field_ext_vect_mult(Gamma[i], compressed_v_z, MQOM2_PARAM_MQ_M/MQOM2_PARAM_MU),
                v_alpha, i
            );
        }
#else
        phi(v_z, v_alpha);
#endif
        field_ext_vect_add(v_alpha, u_eval[e], v_alpha, MQOM2_PARAM_ETA);
        field_ext_constant_vect_mult(r, alpha1[e], alpha0[e], MQOM2_PARAM_ETA);
        field_ext_vect_add(v_alpha, alpha0[e], alpha0[e], MQOM2_PARAM_ETA);
    }

	ret = 0;
err:
	return ret;	
}

