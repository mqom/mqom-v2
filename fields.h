#ifndef __FIELDS_H__
#define __FIELDS_H__

#include "common.h"

#ifndef _concat3
#define _concat3(a, b, c) a ## b ## c
#endif
#ifndef concat3
#define concat3(a, b, c) _concat3(a, b, c)
#endif
#ifndef _concat5
#define _concat5(a, b, c, d, e) a ## b ## c ## d ## e
#endif
#ifndef concat5
#define concat5(a, b, c, d, e) _concat5(a, b, c, d, e)
#endif

/* Small helpers to compute the byte size of num elements in the base
 * and extension fields */
#define BYTE_SIZE_FIELD_BASE(num) (((num) * FIELD_BASE_LOG2_CARD) / 8)
#define BYTE_SIZE_FIELD_EXT(num)  (((num) * FIELD_EXT_LOG2_CARD) / 8)

/* ==== Packing primitives ====== */
static inline void field_gf2_vect_pack(uint8_t elt, uint8_t *packed_elt, uint32_t index)
{
	packed_elt[index / 8] &= ~(1 << (index % 8));
	packed_elt[index / 8] |= (elt << (index % 8));
	return;
}
static inline void field_gf256_vect_pack(uint8_t elt, uint8_t *packed_elt, uint32_t index)
{
	packed_elt[index] = elt;
	return;
}
static inline void field_gf256to2_vect_pack(uint16_t elt, uint16_t *packed_elt, uint32_t index)
{
	packed_elt[index] = elt;
	return;
}


/* ==== String parsing primitives ====== */
static inline void field_gf2_parse(const uint8_t *in_string, uint32_t num_bits, uint8_t *elements)
{
	uint32_t num_bytes = num_bits / 8;
	uint8_t leftover = (uint8_t)(num_bits % 8);
	
	/* Memcpy as much elements as we can */
	memcpy(elements, in_string, num_bytes);
	/* Deal with the possible last left over bits */
	if(leftover){
		uint8_t mask = ((1 << leftover) - 1);
		elements[num_bytes] = in_string[num_bytes] & mask;
	}
	return;
}
static inline void field_gf256_parse(const uint8_t *in_string, uint32_t num, uint8_t *elements)
{
	/* Since we map bytes, this is simply a memcpy */
	memcpy(elements, in_string, num);
	return;
}
static inline void field_gf256to2_parse(const uint8_t *in_string, uint32_t num, uint16_t *elements)
{
	if(!arch_is_big_endian()){
		/* For little endian platforms, this is a memcpy */
		memcpy(elements, in_string, (2 * num));
	}
	else{
		/* For big endian platforms, we have to reverse the bytes */
		uint32_t i;
		for(i = 0; i < (2 * num); i += 2){
			elements[i / 2] = (in_string[i + 1] << 8) | in_string[i];
		}
	}
	return;
}


/* ==== Serialization primitives ====== */
static inline void field_gf2_serialize(const uint8_t *elements, uint32_t num, uint8_t *out_string)
{
	/* This is simply a memcpy 
         * NOTE: in the serialization cases, we only deal with number of bits multiple of bytes
   	 * */
	memcpy(out_string, elements, (num / 8));
	return;
}
static inline void field_gf256_serialize(const uint8_t *elements, uint32_t num, uint8_t *out_string)
{
	/* This is simply a memcpy */
	memcpy(out_string, elements, num);
	return;
}
static inline void field_gf256to2_serialize(const uint16_t *elements, uint32_t num, uint8_t *out_string)
{
	if(!arch_is_big_endian()){
		/* For little endian platforms, this is a memcpy */
		memcpy(out_string, elements, (2 * num));
	}
	else{
		/* For big endian platforms, we have to reverse the bytes */
		uint32_t i;
		for(i = 0; i < (2 * num); i += 2){
			out_string[i]   = (elements[i / 2] & 0xff);
			out_string[i+1] = (elements[i / 2] >> 8);
		}
	}
	return;
}

/* Adapt our types definition depending on the parameters */
/* ==== Base field definition ====== */
/* ==== Base field is GF(2) */
#if MQOM2_PARAM_BASE_FIELD == 1
typedef uint8_t field_base_elt; /* GF(2) */
#define FIELD_BASE_PREFIX gf2
#define FIELD_BASE_LOG2_CARD 1
#define FIELD_BASE_PACKING(num) ((num) / 8)
#define field_base_vect_pack field_gf2_vect_pack
#define field_base_parse field_gf2_parse
#define field_base_serialize field_gf2_serialize
/* ==== Base field is GF(256) */
#elif MQOM2_PARAM_BASE_FIELD == 8 
typedef uint8_t field_base_elt; /* GF(256) */
#define FIELD_BASE_PREFIX gf256
#define FIELD_BASE_LOG2_CARD 8
#define FIELD_BASE_PACKING(num) (num)
#define field_base_vect_pack field_gf256_vect_pack
#define field_base_parse field_gf256_parse
#define field_base_serialize field_gf256_serialize
#else
#error "Error: MQOM2_PARAM_BASE_FIELD is neither GF(2) nor GF(256)! Please choose one of those"
#endif

/* ==== Extension field definition ====== */
/* ==== Extension field is GF(256) */
#if MQOM2_PARAM_EXT_FIELD == 8
typedef uint8_t field_ext_elt; /* GF(256) */
#define FIELD_EXT_PREFIX gf256
#define FIELD_EXT_LOG2_CARD 8
#define FIELD_EXT_PACKING(num) (num)
#define field_ext_vect_pack field_gf256_vect_pack
#define field_ext_parse field_gf256_parse
#define field_ext_serialize field_gf256_serialize
/* ==== Base field is GF(256^2) */
#elif MQOM2_PARAM_EXT_FIELD == 16
typedef uint16_t field_ext_elt; /* GF(256^2) = GF(2^16) */
#define FIELD_EXT_PREFIX gf256to2
#define FIELD_EXT_LOG2_CARD 16
#define FIELD_EXT_PACKING(num) (num)
#define field_ext_vect_pack field_gf256to2_vect_pack
#define field_ext_parse field_gf256to2_parse
#define field_ext_serialize field_gf256to2_serialize
#else
#error "Error: MQOM2_PARAM_EXT_FIELD is neither GF(256) nor GF(256^2)! Please choose one of those"
#endif

/* Field extension basis */
#if MQOM2_PARAM_BASE_FIELD == 1
#if MQOM2_PARAM_EXT_FIELD == 8
static field_ext_elt ext_basis[8] = {1, 2, 4, 8, 16, 32, 64, 128};
#elif MQOM2_PARAM_EXT_FIELD == 16
static field_ext_elt ext_basis[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
#endif
#elif MQOM2_PARAM_BASE_FIELD == 8
#if MQOM2_PARAM_EXT_FIELD == 8
static field_ext_elt ext_basis[1] = {1};
#elif MQOM2_PARAM_EXT_FIELD == 16
static field_ext_elt ext_basis[2] = {1, 256};
#endif
#endif
/* Dummy function to avoid "unused variable" warning for ext_basis */
static inline void _field_ext_basis_dummy(void)
{
	(void)ext_basis;
}

/* Some helper to deal with false "hybrid" GF(256) and GF(256) situations */
/**/
#define gf256_gf256_constant_vect_mult_ref gf256_constant_vect_mult_ref
#define gf256_gf256_vect_mult_ref gf256_vect_mult_ref
#define gf256_gf256_mat_mult_ref gf256_mat_mult_ref
/**/
#define gf256_gf256_constant_vect_mult_avx2 gf256_constant_vect_mult_avx2
#define gf256_gf256_vect_mult_avx2 gf256_vect_mult_avx2
#define gf256_gf256_mat_mult_avx2 gf256_mat_mult_avx2
/**/
#define gf256_gf256_constant_vect_mult_gfni gf256_constant_vect_mult_gfni
#define gf256_gf256_vect_mult_gfni gf256_vect_mult_gfni
#define gf256_gf256_mat_mult_gfni gf256_mat_mult_gfni

/* This file adapts the fields definition depending on the compilation */
#if !defined(FIELDS_REF) && !defined(FIELDS_AVX2) && !defined(FIELDS_GFNI)
    #if defined(__GFNI__) && defined(__AVX512BW__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__)
        #define FIELDS_GFNI
    #else
        #ifdef __AVX2__
            #define FIELDS_AVX2
        #else
            #define FIELDS_REF
        #endif
    #endif
#endif

#if defined(FIELDS_REF)
#include "fields_ref.h"
#define FIELD_IMPLEMENTATION_SUFFIX ref
#elif defined(FIELDS_AVX2)
#include "fields_avx2.h"
#define FIELD_IMPLEMENTATION_SUFFIX avx2
#elif defined(FIELDS_GFNI)
#include "fields_gfni.h"
#define FIELD_IMPLEMENTATION_SUFFIX gfni
#else
  #error "Error: no low-level field implementation detected ..."
#endif

/* Now construct our base operations by constructing our low-level
 * functions depending on the prefix (fields types) and suffix (implementation) */
/* Base field operations */
#define _field_base_mult concat3(FIELD_BASE_PREFIX, _mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_base_constant_vect_mult concat3(FIELD_BASE_PREFIX, _constant_vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_base_vect_mult concat3(FIELD_BASE_PREFIX, _vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_base_mat_mult concat3(FIELD_BASE_PREFIX, _mat_mult_, FIELD_IMPLEMENTATION_SUFFIX)
/* Extension field operations */
#define _field_ext_mult concat3(FIELD_EXT_PREFIX, _mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_constant_vect_mult concat3(FIELD_EXT_PREFIX, _constant_vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_vect_mult concat3(FIELD_EXT_PREFIX, _vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_mat_mult concat3(FIELD_EXT_PREFIX, _mat_mult_, FIELD_IMPLEMENTATION_SUFFIX)
/* Hybrid multiplications */
#define _field_base_ext_constant_vect_mult concat5(FIELD_BASE_PREFIX, _, FIELD_EXT_PREFIX, _constant_vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_base_constant_vect_mult concat5(FIELD_EXT_PREFIX, _, FIELD_BASE_PREFIX, _constant_vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_base_ext_vect_mult concat5(FIELD_BASE_PREFIX, _, FIELD_EXT_PREFIX, _vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_base_vect_mult concat5(FIELD_EXT_PREFIX, _, FIELD_BASE_PREFIX, _vect_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_base_ext_mat_mult concat5(FIELD_BASE_PREFIX, _, FIELD_EXT_PREFIX, _mat_mult_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_base_mat_mult concat5(FIELD_EXT_PREFIX, _, FIELD_BASE_PREFIX, _mat_mult_, FIELD_IMPLEMENTATION_SUFFIX)
/* Matrix transposition */
#define _field_base_mat_transpose concat3(FIELD_BASE_PREFIX, _mat_transpose_, FIELD_IMPLEMENTATION_SUFFIX)
#define _field_ext_mat_transpose concat3(FIELD_EXT_PREFIX, _mat_transpose_, FIELD_IMPLEMENTATION_SUFFIX)

/* Public API */
static inline field_base_elt field_base_mult(field_base_elt a, field_base_elt b)
{
	return _field_base_mult(a, b);
}

static inline void field_base_constant_vect_mult(field_base_elt a, const field_base_elt *b, field_base_elt *c, uint32_t len)
{
	_field_base_constant_vect_mult(a, b, c, len);
	return;
}

static inline field_base_elt field_base_vect_mult(const field_base_elt *a, const field_base_elt *b, uint32_t len)
{
	return _field_base_vect_mult(a, b, len);
}

static inline void field_base_mat_mult(const field_base_elt *A, const field_base_elt *X, field_base_elt *Y, uint32_t n, matrix_type mtype)
{
	_field_base_mat_mult(A, X, Y, n, mtype);
	return;
}

static inline field_ext_elt field_ext_mult(field_ext_elt a, field_ext_elt b)
{
	return _field_ext_mult(a, b);
}

static inline void field_ext_constant_vect_mult(field_ext_elt a, const field_ext_elt *b, field_ext_elt *c, uint32_t len)
{
	_field_ext_constant_vect_mult(a, b, c, len);
	return;
}

static inline field_ext_elt field_ext_vect_mult(const field_ext_elt *a, const field_ext_elt *b, uint32_t len)
{
        return _field_ext_vect_mult(a, b, len);
}

static inline void field_ext_mat_mult(const field_ext_elt *A, const field_ext_elt *X, field_ext_elt *Y, uint32_t n, matrix_type mtype)
{
	_field_ext_mat_mult(A, X, Y, n, mtype);
	return;
}

static inline void field_base_ext_constant_vect_mult(field_base_elt a, const field_ext_elt *b, field_ext_elt *c, uint32_t len)
{
	_field_base_ext_constant_vect_mult(a, b, c, len);
	return;
}

static inline void field_ext_base_constant_vect_mult(field_ext_elt a, const field_base_elt *b, field_ext_elt *c, uint32_t len)
{
	_field_ext_base_constant_vect_mult(a, b, c, len);
	return;
}


static inline field_ext_elt field_base_ext_vect_mult(const field_base_elt *a, const field_ext_elt *b, uint32_t len)
{
	return _field_base_ext_vect_mult(a, b, len);
}

static inline void field_base_ext_mat_mult(const field_base_elt *A, const field_ext_elt *X, field_ext_elt *Y, uint32_t n, matrix_type mtype)
{
	_field_base_ext_mat_mult(A, X, Y, n, mtype);
	return;
}

static inline field_ext_elt field_ext_base_vect_mult(const field_ext_elt *a, const field_base_elt *b, uint32_t len)
{
	return _field_ext_base_vect_mult(a, b, len);
}

static inline void field_ext_base_mat_mult(const field_ext_elt *A, const field_base_elt *X, field_ext_elt *Y, uint32_t n, matrix_type mtype)
{
	_field_ext_base_mat_mult(A, X, Y, n, mtype);
	return;
}

static inline void field_base_mat_transpose(const field_base_elt *A, field_base_elt *B, uint32_t n, matrix_type mtype)
{
	_field_base_mat_transpose(A, B, n, mtype);
	return;
}

static inline void field_ext_mat_transpose(const field_ext_elt *A, field_ext_elt *B, uint32_t n, matrix_type mtype)
{
	_field_ext_mat_transpose(A, B, n, mtype);
	return;
}

/*
 * NOTE: these are rather "naive" xors between bytes, but the compiler optimization passes should
 * hopefully properly vectorize this.
 */
static inline void field_base_vect_add(const field_base_elt *a, const field_base_elt *b, field_base_elt *c, uint32_t len)
{
	uint32_t i;

	for(i = 0; i < FIELD_BASE_PACKING(len); i++){
		c[i] = a[i] ^ b[i];
	}
	return;
}

static inline void field_ext_vect_add(const field_ext_elt *a, const field_ext_elt *b, field_ext_elt *c, uint32_t len)
{
	uint32_t i;

	for(i = 0; i < FIELD_EXT_PACKING(len); i++){
		c[i] = a[i] ^ b[i];
	}
	return;
}

static inline field_ext_elt get_evaluation_point(uint16_t i) {
	return (field_ext_elt) i;
}

#endif /* __FIELDS_H__ */
