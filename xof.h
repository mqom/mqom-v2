#ifndef __XOF_H__
#define __XOF_H__

#include <stdint.h>
/* Include the underlying Keccak header for hash and XOF */
#include "sha3/KeccakHash.h"

/* For common helpers */
#include "common.h"

/* Depending on the parameter, the instances are different:
 *   - For 128 bits security, we use SHAKE-128 for XOF 
 *   - For 192 bits security, we use SHAKE-256 for XOF
 *   - For 256 bits security, we use SHAKE-256 for XOF
 */
#include "mqom2_parameters.h"
/* === 128 bits security === */
#if MQOM2_PARAM_SECURITY == 128
#define _XOF_Init Keccak_HashInitialize_SHAKE128
#define _XOF_Init_x4 Keccak_HashInitializetimes4_SHAKE128
/* === 192 bits security === */
#elif MQOM2_PARAM_SECURITY == 192
#define _XOF_Init Keccak_HashInitialize_SHAKE256
#define _XOF_Init_x4 Keccak_HashInitializetimes4_SHAKE256
/* === 256 bits security === */
#elif MQOM2_PARAM_SECURITY == 256
#define _XOF_Init Keccak_HashInitialize_SHAKE256
#define _XOF_Init_x4 Keccak_HashInitializetimes4_SHAKE256
#else
#error "No XOF implementation for this security level"
#endif
/* Common defines for XOF */
#define _XOF_Update Keccak_HashUpdate
#define _XOF_Final Keccak_HashFinal
#define _XOF_Squeeze Keccak_HashSqueeze

/* Hash and XOF contexts are simply Keccak instances, with XOF finalization state
 * for XOF
 */
typedef struct {
	uint8_t xof_finalized;
	Keccak_HashInstance ctx;
} xof_context; 

/* Exported API for XOF, simple and x4 */
int xof_init(xof_context *ctx);
int xof_update(xof_context *ctx, const uint8_t* data, size_t byte_len);
int xof_squeeze(xof_context *ctx, uint8_t *out, uint32_t byte_len);

#endif /* __HASH_XOF_H__ */
