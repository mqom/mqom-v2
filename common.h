#ifndef __COMMON_H__
#define __COMMON_H__

/* MQOM2 parameters */
#include "mqom2_parameters.h"
#include <string.h>
#include <stdint.h>

#undef ERR
#define ERR(r, e) if(r) { goto e; }

/* Helpers to compute some sizes of serialized objects */
#define MQOM2_PK_SIZE ((2 * MQOM2_PARAM_SEED_SIZE) + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_M))
#define MQOM2_SK_SIZE ((2 * MQOM2_PARAM_SEED_SIZE) + BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_M + MQOM2_PARAM_MQ_N))
#define MQOM2_PARAM_OPENING_SIZE (MQOM2_PARAM_TAU*(                           \
    BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_MQ_N)-MQOM2_PARAM_SEED_SIZE              \
      + MQOM2_PARAM_NB_EVALS_LOG*MQOM2_PARAM_SEED_SIZE                        \
      + MQOM2_PARAM_DIGEST_SIZE                                               \
))
#define MQOM2_SIG_SIZE (                                                      \
    4 + MQOM2_PARAM_TAU*BYTE_SIZE_FIELD_BASE(MQOM2_PARAM_ETA*MQOM2_PARAM_MU)  \
      + (MQOM2_PARAM_SALT_SIZE) + (2*(MQOM2_PARAM_DIGEST_SIZE))               \
      + MQOM2_PARAM_OPENING_SIZE                                              \
)

/* Endianness detection:
 * Return 1 if architecture is big endian, 0 otherwise. 
 */
static inline int arch_is_big_endian(void)
{
        const uint16_t val = 0x0102;
        const uint8_t *buf = (const uint8_t *)(&val);

        return (buf[0] == 0x01);
}


/* Salt tweaking primitive */
static inline void TweakSalt(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], uint8_t tweaked_salt[MQOM2_PARAM_SALT_SIZE], uint8_t sel, uint8_t e, uint16_t j)
{
        /* The tweak only consists in xoring the salt with the counter */
        memcpy(tweaked_salt, salt, MQOM2_PARAM_SALT_SIZE);
        tweaked_salt[0] ^= sel + 4*e;
        tweaked_salt[1] ^= j & 0xff;
        tweaked_salt[2] ^= (j>>8) & 0xff;
        return;
}

/* LinOrtho primitive */
static inline void LinOrtho(const uint8_t seed[MQOM2_PARAM_SEED_SIZE], uint8_t linortho_seed[MQOM2_PARAM_SEED_SIZE])
{
        unsigned int i;
        memcpy(&linortho_seed[0], &seed[MQOM2_PARAM_SEED_SIZE / 2], MQOM2_PARAM_SEED_SIZE / 2);
        memcpy(&linortho_seed[MQOM2_PARAM_SEED_SIZE / 2], &seed[0], MQOM2_PARAM_SEED_SIZE / 2);

        for(i = 0; i < MQOM2_PARAM_SEED_SIZE / 2; i++){
                linortho_seed[i] ^= seed[i];
        }

        return;
}

/* Helper to xor MQOM2_PARAM_SEED_SIZE elements 
 * NOTE: since we do not have information about alignment, we must
 * copy this to aligned buffers and do this on 64 bits, this will
 * hopefully trigger the compiler vectorization
 */
static inline void xor_blocks(const uint8_t seed_in[MQOM2_PARAM_SEED_SIZE], const uint8_t delta[MQOM2_PARAM_SEED_SIZE], uint8_t seed_out[MQOM2_PARAM_SEED_SIZE]){
        unsigned int i;
        uint64_t s[3][MQOM2_PARAM_SEED_SIZE / 8];

        memcpy(&s[0], seed_in, MQOM2_PARAM_SEED_SIZE);
        memcpy(&s[1], delta, MQOM2_PARAM_SEED_SIZE);

        for(i = 0; i < MQOM2_PARAM_SEED_SIZE / 8; i++){
                s[2][i] = s[0][i] ^ s[1][i];
        }

        memcpy(seed_out, &s[2], MQOM2_PARAM_SEED_SIZE);

        return;
}

#endif /* __COMMON_H__ */
