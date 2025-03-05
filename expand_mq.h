#ifndef __EXPAND_MQ_H__
#define __EXPAND_MQ_H__

#include "xof.h"
#include "prg.h"
#include "fields.h"
#include <stdlib.h>

int ExpandEquations(const uint8_t mseed_eq[2 * MQOM2_PARAM_SEED_SIZE], field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)]);

#endif /* __EXPAND_MQ_H__ */
