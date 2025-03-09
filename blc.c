#include "blc.h"
#include "ggm_tree.h"
#include "benchmark.h"

/* SeedCommit variants
 * NOTE: we factorize the key schedule, the tweaked salt is inside the encryption context
 */
static inline void SeedCommit(enc_ctx *ctx1, enc_ctx *ctx2, const uint8_t seed[MQOM2_PARAM_SEED_SIZE], uint8_t seed_com[2 * MQOM2_PARAM_SEED_SIZE])
{
    uint8_t linortho_seed[MQOM2_PARAM_SEED_SIZE];
    LinOrtho(seed, linortho_seed);
    enc_encrypt_x2(ctx1, ctx2, seed, seed, &seed_com[0], &seed_com[MQOM2_PARAM_SEED_SIZE]);
    /* Xor with LinOrtho seed */
    xor_blocks(&seed_com[0], linortho_seed, &seed_com[0]);
    xor_blocks(&seed_com[MQOM2_PARAM_SEED_SIZE], linortho_seed, &seed_com[MQOM2_PARAM_SEED_SIZE]);

    return;
}

static inline void SeedCommit_x2(enc_ctx *ctx1, enc_ctx *ctx2, const uint8_t seed1[MQOM2_PARAM_SEED_SIZE], const uint8_t seed2[MQOM2_PARAM_SEED_SIZE], uint8_t seed_com1[2 * MQOM2_PARAM_SEED_SIZE], uint8_t seed_com2[2 * MQOM2_PARAM_SEED_SIZE])
{
    uint8_t linortho_seed1[MQOM2_PARAM_SEED_SIZE];
    uint8_t linortho_seed2[MQOM2_PARAM_SEED_SIZE];
    LinOrtho(seed1, linortho_seed1);
    LinOrtho(seed2, linortho_seed2);
    enc_encrypt_x4(ctx1, ctx2, ctx1, ctx2,
                    seed1, seed1, seed2, seed2,
                    &seed_com1[0], &seed_com1[MQOM2_PARAM_SEED_SIZE],
                    &seed_com2[0], &seed_com2[MQOM2_PARAM_SEED_SIZE]);
    /* Xor with LinOrtho seed */
    xor_blocks(&seed_com1[0], linortho_seed1, &seed_com1[0]);
    xor_blocks(&seed_com1[MQOM2_PARAM_SEED_SIZE], linortho_seed1, &seed_com1[MQOM2_PARAM_SEED_SIZE]);
    xor_blocks(&seed_com2[0], linortho_seed2, &seed_com2[0]);
    xor_blocks(&seed_com2[MQOM2_PARAM_SEED_SIZE], linortho_seed2, &seed_com2[MQOM2_PARAM_SEED_SIZE]);

    return;
}

#define PRG_BLC_SIZE (BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N) + BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_ETA) - MQOM2_PARAM_SEED_SIZE)

int BLC_Commit(const uint8_t mseed[MQOM2_PARAM_SEED_SIZE], const uint8_t salt[MQOM2_PARAM_SALT_SIZE], const field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], uint8_t com1[MQOM2_PARAM_DIGEST_SIZE], blc_key_t* key, field_ext_elt x0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], field_ext_elt u0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], field_ext_elt u1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)])
{
    int ret = -1;
    enc_ctx ctx_seed_commit1, ctx_seed_commit2;
    xof_context xof_ctx;
    uint8_t tweaked_salt[MQOM2_PARAM_SALT_SIZE];
    uint8_t delta[MQOM2_PARAM_SEED_SIZE];
    uint8_t rseed[MQOM2_PARAM_TAU][MQOM2_PARAM_SEED_SIZE];
    uint32_t e, i, i_;
    /* The serialization of x */
    uint8_t _x[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)];
    /* Tree rseed PRG salt, set to 0 */
    uint8_t tree_prg_salt[MQOM2_PARAM_SALT_SIZE] = { 0 };
    /* PRG cache */
    prg_key_sched_cache *prg_cache = NULL;

    /* Compute the rseed table */
    ret = PRG(tree_prg_salt, 0, mseed, MQOM2_PARAM_TAU * MQOM2_PARAM_SEED_SIZE, (uint8_t*)rseed, NULL); ERR(ret, err);
    /* Compute delta */
    field_base_serialize(x, MQOM2_PARAM_MQ_N, _x); 
    memcpy(delta, _x, MQOM2_PARAM_SEED_SIZE);
    
    uint8_t hash_ls_com[MQOM2_PARAM_TAU][MQOM2_PARAM_DIGEST_SIZE];

    uint8_t lseed[MQOM2_PARAM_NB_EVALS][MQOM2_PARAM_SEED_SIZE];
    uint8_t exp[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)+BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_ETA)];
    field_base_elt bar_x_i[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt bar_u_i[FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    field_ext_elt tmp_n[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
    field_ext_elt tmp_eta[FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
    field_base_elt acc_x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
    for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        /* Initialize the PRG cache when used */
        prg_cache = init_prg_cache(PRG_BLC_SIZE);

        __BENCHMARK_START__(BS_BLC_EXPAND_TREE);
        ret = GGMTree_Expand(salt, rseed[e], delta, e, key->node[e], lseed); ERR(ret, err);
        __BENCHMARK_STOP__(BS_BLC_EXPAND_TREE);

        __BENCHMARK_START__(BS_BLC_KEYSCH_COMMIT);
        TweakSalt(salt, tweaked_salt, 0, e, 0);
        ret = enc_key_sched(&ctx_seed_commit1, tweaked_salt); ERR(ret, err);
        tweaked_salt[0] ^= 0x01;
        ret = enc_key_sched(&ctx_seed_commit2, tweaked_salt); ERR(ret, err);
        __BENCHMARK_STOP__(BS_BLC_KEYSCH_COMMIT);

        ret = xof_init(&xof_ctx); ERR(ret, err);
	    ret = xof_update(&xof_ctx, (const uint8_t*) "\x06", 1); ERR(ret, err);
        memset((uint8_t*) acc_x, 0, sizeof(acc_x));
        memset((uint8_t*) u0[e], 0, sizeof(u0[e]));
        memset((uint8_t*) u1[e], 0, sizeof(u1[e]));
        memset((uint8_t*) x0[e], 0, sizeof(x0[e]));
        for(i = 0; i < MQOM2_PARAM_NB_EVALS; i+=2) {
            __BENCHMARK_START__(BS_BLC_SEED_COMMIT);
            SeedCommit_x2(&ctx_seed_commit1, &ctx_seed_commit2, lseed[i], lseed[i+1], key->ls_com[e][i], key->ls_com[e][i+1]);
            __BENCHMARK_STOP__(BS_BLC_SEED_COMMIT);
            
            for(i_ = 0; i_ < 2; i_++) {
                memcpy(exp, lseed[i+i_], MQOM2_PARAM_SEED_SIZE);
                __BENCHMARK_START__(BS_BLC_PRG);
                ret = PRG(salt, e, lseed[i+i_], PRG_BLC_SIZE, exp + MQOM2_PARAM_SEED_SIZE, prg_cache); ERR(ret, err);
                __BENCHMARK_STOP__(BS_BLC_PRG);
                __BENCHMARK_START__(BS_BLC_XOF);
                ret = xof_update(&xof_ctx, key->ls_com[e][i+i_], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
                __BENCHMARK_STOP__(BS_BLC_XOF);
                
                __BENCHMARK_START__(BS_BLC_ARITH);
                field_ext_elt w = get_evaluation_point(i+i_);
                // Compute P_u
                field_ext_parse(exp + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N), MQOM2_PARAM_ETA, bar_u_i);
                field_ext_vect_add(u1[e], bar_u_i, u1[e], MQOM2_PARAM_ETA);
                field_ext_constant_vect_mult(w, bar_u_i, tmp_eta, MQOM2_PARAM_ETA);
                field_ext_vect_add(u0[e], tmp_eta, u0[e], MQOM2_PARAM_ETA);

                // Compute P_x
                field_base_parse(exp, MQOM2_PARAM_MQ_N, bar_x_i);
                field_base_vect_add(acc_x, bar_x_i, acc_x, MQOM2_PARAM_MQ_N);
                field_ext_base_constant_vect_mult(w, bar_x_i, tmp_n, MQOM2_PARAM_MQ_N);
                field_ext_vect_add(x0[e], tmp_n, x0[e], MQOM2_PARAM_MQ_N);
                __BENCHMARK_STOP__(BS_BLC_ARITH);
            }
        }
        /* Invalidate the PRG cache */
        destroy_prg_cache(prg_cache);
        prg_cache = NULL;

        __BENCHMARK_START__(BS_BLC_XOF);
        ret = xof_squeeze(&xof_ctx, hash_ls_com[e], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
        __BENCHMARK_STOP__(BS_BLC_XOF);

        field_base_elt delta_x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
        uint8_t serialized_delta_x[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)];
        field_base_vect_add(x, acc_x, delta_x, MQOM2_PARAM_MQ_N);
        field_base_serialize(delta_x, MQOM2_PARAM_MQ_N, serialized_delta_x);
        memcpy(key->partial_delta_x[e], serialized_delta_x+MQOM2_PARAM_SEED_SIZE, BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE);
    }

    __BENCHMARK_START__(BS_BLC_GLOBAL_XOF);
    ret = xof_init(&xof_ctx); ERR(ret, err);
    ret = xof_update(&xof_ctx, (const uint8_t*) "\x07", 1); ERR(ret, err);
    for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        ret = xof_update(&xof_ctx, hash_ls_com[e], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    }
    for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        ret = xof_update(&xof_ctx, key->partial_delta_x[e], BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
    }
    ret = xof_squeeze(&xof_ctx, com1, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    __BENCHMARK_STOP__(BS_BLC_GLOBAL_XOF);

    ret = 0;
err:
    destroy_prg_cache(prg_cache);
    return ret;
}

int BLC_Open(const blc_key_t* key, const uint16_t i_star[MQOM2_PARAM_TAU], uint8_t opening[MQOM2_PARAM_OPENING_SIZE])
{
    int ret = -1;
    int e;
    
    uint8_t* path = &opening[0];
    uint8_t* out_ls_com = &opening[MQOM2_PARAM_TAU*MQOM2_PARAM_SEED_SIZE*MQOM2_PARAM_NB_EVALS_LOG];
    uint8_t* partial_delta_x = &opening[MQOM2_PARAM_TAU*(MQOM2_PARAM_SEED_SIZE*MQOM2_PARAM_NB_EVALS_LOG+MQOM2_PARAM_DIGEST_SIZE)];

    for(e = 0; e < MQOM2_PARAM_TAU; e++){
        ret = GGMTree_Open(key->node[e], i_star[e], (uint8_t(*)[MQOM2_PARAM_SEED_SIZE]) &path[e*(MQOM2_PARAM_NB_EVALS_LOG*MQOM2_PARAM_SEED_SIZE)]); ERR(ret, err);

        memcpy(&out_ls_com[e*MQOM2_PARAM_DIGEST_SIZE], key->ls_com[e][i_star[e]], MQOM2_PARAM_DIGEST_SIZE);
        memcpy(&partial_delta_x[e*(BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE)], key->partial_delta_x[e], BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE);
    }

    ret = 0;
err:
    return ret;
}

int BLC_Eval(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], const uint8_t com1[MQOM2_PARAM_DIGEST_SIZE], const uint8_t opening[MQOM2_PARAM_OPENING_SIZE], const uint16_t i_star[MQOM2_PARAM_TAU], field_ext_elt x_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], field_ext_elt u_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)])
{
    int ret = -1;
    enc_ctx ctx_seed_commit1, ctx_seed_commit2;
    xof_context xof_ctx;
    uint32_t e, i, i_;
    uint8_t tweaked_salt[MQOM2_PARAM_SALT_SIZE];
    uint8_t lseed[MQOM2_PARAM_NB_EVALS][MQOM2_PARAM_SEED_SIZE];
    uint8_t com1_[MQOM2_PARAM_DIGEST_SIZE];
    uint8_t ls_com_e[MQOM2_PARAM_NB_EVALS][MQOM2_PARAM_DIGEST_SIZE];
    uint8_t exp[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)+BYTE_SIZE_FIELD_EXT(MQOM2_PARAM_ETA)];

    const uint8_t* path = &opening[0];
    const uint8_t* out_ls_com = &opening[MQOM2_PARAM_TAU*MQOM2_PARAM_SEED_SIZE*MQOM2_PARAM_NB_EVALS_LOG];
    const uint8_t* partial_delta_x = &opening[MQOM2_PARAM_TAU*(MQOM2_PARAM_SEED_SIZE*MQOM2_PARAM_NB_EVALS_LOG+MQOM2_PARAM_DIGEST_SIZE)];
    
    uint8_t hash_ls_com[MQOM2_PARAM_TAU][MQOM2_PARAM_DIGEST_SIZE];

    /* PRG cache */
    prg_key_sched_cache *prg_cache = NULL;

    for(e = 0; e < MQOM2_PARAM_TAU; e++){
        /* Initialize the PRG cache when used */
        prg_cache = init_prg_cache(PRG_BLC_SIZE);

        ret = GGMTree_PartiallyExpand(salt, (uint8_t(*)[MQOM2_PARAM_SEED_SIZE]) &path[e*(MQOM2_PARAM_NB_EVALS_LOG*MQOM2_PARAM_SEED_SIZE)], e, i_star[e], lseed); ERR(ret, err);

        TweakSalt(salt, tweaked_salt, 0, e, 0);
        ret = enc_key_sched(&ctx_seed_commit1, tweaked_salt); ERR(ret, err);
        tweaked_salt[0] ^= 0x01;
        ret = enc_key_sched(&ctx_seed_commit2, tweaked_salt); ERR(ret, err);

        ret = xof_init(&xof_ctx); ERR(ret, err);
	    ret = xof_update(&xof_ctx, (const uint8_t*) "\x06", 1); ERR(ret, err);

        field_base_elt bar_x_i[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)] = {0};
        field_ext_elt bar_u_i[FIELD_EXT_PACKING(MQOM2_PARAM_ETA)] = {0};
        field_ext_elt tmp_n[FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)];
        field_ext_elt tmp_eta[FIELD_EXT_PACKING(MQOM2_PARAM_ETA)];
        
        field_ext_elt r = get_evaluation_point(i_star[e]);
        memset((uint8_t*) u_eval[e], 0, sizeof(u_eval[e]));
        field_base_elt delta_x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)];
        uint8_t serialized_delta_x[BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)];
        memset(serialized_delta_x, 0, MQOM2_PARAM_SEED_SIZE);
        memcpy(serialized_delta_x+MQOM2_PARAM_SEED_SIZE, &partial_delta_x[e*(BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE)], BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE);
        field_base_parse(serialized_delta_x, MQOM2_PARAM_MQ_N, delta_x);
        field_ext_base_constant_vect_mult(r, delta_x, x_eval[e], MQOM2_PARAM_MQ_N);
        
        for(i = 0; i < MQOM2_PARAM_NB_EVALS; i+=2) {
            if((i != i_star[e]) && (i+1 != i_star[e])) {
                SeedCommit_x2(&ctx_seed_commit1, &ctx_seed_commit2, lseed[i], lseed[i+1], ls_com_e[i], ls_com_e[i+1]);
            } else if(i != i_star[e]) {
                SeedCommit(&ctx_seed_commit1, &ctx_seed_commit2, lseed[i], ls_com_e[i]);
            } else {
                SeedCommit(&ctx_seed_commit1, &ctx_seed_commit2, lseed[i+1], ls_com_e[i+1]);
            }
            
            for(i_ = 0; i_ < 2; i_++) {
                if(i+i_ !=i_star[e]) {
                    memcpy(exp, lseed[i+i_], MQOM2_PARAM_SEED_SIZE);
                    ret = PRG(salt, e, lseed[i+i_], PRG_BLC_SIZE, exp + MQOM2_PARAM_SEED_SIZE, prg_cache); ERR(ret, err);
                    ret = xof_update(&xof_ctx, ls_com_e[i+i_], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
                
                    field_ext_elt w = get_evaluation_point(i+i_);
                    // Compute v_u
                    field_ext_parse(exp + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N), MQOM2_PARAM_ETA, bar_u_i);
                    field_ext_constant_vect_mult(r^w, bar_u_i, tmp_eta, MQOM2_PARAM_ETA);
                    field_ext_vect_add(u_eval[e], tmp_eta, u_eval[e], MQOM2_PARAM_ETA);

                    // Compute P_x
                    field_base_parse(exp, MQOM2_PARAM_MQ_N, bar_x_i);
                    field_ext_base_constant_vect_mult(r^w, bar_x_i, tmp_n, MQOM2_PARAM_MQ_N);
                    field_ext_vect_add(x_eval[e], tmp_n, x_eval[e], MQOM2_PARAM_MQ_N);
                } else {
                    ret = xof_update(&xof_ctx, &out_ls_com[e*MQOM2_PARAM_DIGEST_SIZE], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
                }
            }
        }
        /* Invalidate the PRG cache */
        destroy_prg_cache(prg_cache);
        prg_cache = NULL;

        ret = xof_squeeze(&xof_ctx, hash_ls_com[e], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    }

    ret = xof_init(&xof_ctx); ERR(ret, err);
    ret = xof_update(&xof_ctx, (const uint8_t*) "\x07", 1); ERR(ret, err);
    for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        ret = xof_update(&xof_ctx, hash_ls_com[e], MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);
    }
    for(e = 0; e < MQOM2_PARAM_TAU; e++) {
        ret = xof_update(&xof_ctx, &partial_delta_x[e*(BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE)], BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE); ERR(ret, err);
    }
    ret = xof_squeeze(&xof_ctx, com1_, MQOM2_PARAM_DIGEST_SIZE); ERR(ret, err);

    if(memcmp(com1, com1_, MQOM2_PARAM_DIGEST_SIZE) != 0) {
        ret = -1;
        goto err;
    }
    
    ret = 0;
err:
    destroy_prg_cache(prg_cache);
    return ret;
}
