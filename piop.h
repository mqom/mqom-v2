#ifndef __PIOP_H__
#define __PIOP_H__

#include "mqom2_parameters.h"
#include "fields.h"

int ComputePAlpha(const uint8_t com[MQOM2_PARAM_DIGEST_SIZE], const field_ext_elt x0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_ext_elt u0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_ext_elt u1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_base_elt x[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)]);

int RecomputePAlpha(const uint8_t com[MQOM2_PARAM_DIGEST_SIZE], const field_ext_elt alpha1[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const uint16_t i_star[MQOM2_PARAM_TAU], const field_ext_elt x_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_MQ_N)], const field_ext_elt u_eval[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)], const field_base_elt A[MQOM2_PARAM_MQ_M][MQOM2_PARAM_MQ_N][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt b[MQOM2_PARAM_MQ_M][FIELD_BASE_PACKING(MQOM2_PARAM_MQ_N)], const field_base_elt y[FIELD_BASE_PACKING(MQOM2_PARAM_MQ_M)], field_ext_elt alpha0[MQOM2_PARAM_TAU][FIELD_EXT_PACKING(MQOM2_PARAM_ETA)]);

#endif /* __PIOP_H__ */
