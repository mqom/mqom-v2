#ifndef __PRG_H__
#define __PRG_H__

/* MQOM2 parameters */
#include "mqom2_parameters.h"
/* Our PRG is based on a block cipher defined in the following header */
#include "enc.h"

int PRG(const uint8_t salt[MQOM2_PARAM_SALT_SIZE], uint32_t e, const uint8_t seed[MQOM2_PARAM_SEED_SIZE], uint32_t nbytes, uint8_t *out_data);

#endif /* __PRG_H__ */
