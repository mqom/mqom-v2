#ifndef __FIELDS_GFNI_H__
#define __FIELDS_GFNI_H__

/* Check if GFNI is supported */
#if defined(__GFNI__) && defined(__AVX512BW__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__)

#include "fields_common.h"
#include "fields_ref.h"
/* Needed for memcpy */
#include <string.h>
/* Needed for GFNI assembly intrinsics */
#include <immintrin.h>

/* This helper tries to efficiently copy len bytes in the ymm register */
static inline __m512i load_incomplete_m512(const uint8_t *a, uint32_t len)
{
        const __m512i zero = _mm512_setzero_epi32();

	if(len % 4){
		/* In this case, we switch to the slower _mm512_mask_loadu_epi8 */
		__mmask64 mask = ((__mmask64)1 << len) - 1;
		return _mm512_mask_loadu_epi8(zero, mask, (int const*)a);
	}
	else{
		/* Optimize with faster instruction */
		__mmask16 mask = (1 << (len / 4)) - 1;
        	return _mm512_mask_loadu_epi32(zero, mask, (int const*)a);
	}
}

/* This helper tries to efficiently copy len bytes from the ymm register */
static inline void store_incomplete_m512(__m512i in, uint8_t *a, uint32_t len)
{
	if(len % 4){
		/* In this case, we switch to the slower _mm512_mask_storeu_epi8 */
		__mmask64 mask = ((__mmask64)1 << len) - 1;
		_mm512_mask_storeu_epi8((void*)a, mask, in);
	}
	else{
		/* Optimize with faster instruction */
		__mmask16 mask = (1 << (len / 4)) - 1;
        	_mm512_mask_storeu_epi32((void*)a, mask, in);
	}
}

static inline uint8_t parity_gfni(__m512i accu) {
	uint32_t i;
	uint8_t res;
	__attribute__((aligned(64))) uint64_t local_c[8];

	/* We have to compute the parity bit, do it 64 bits per 64 bits */
	accu = _mm512_popcnt_epi64(accu);
	/* Store the result */
	_mm512_storeu_epi64((__m512i*)local_c, accu);
	/* Finish the parity bit computation on the 64-bit parts */
	res = 0;
	for(i = 0; i < 8; i++){
		res ^= (local_c[i] & 1);
	}
	return res;
}

static inline uint8_t sum_uint8_gfni(__m512i accu) {
	uint32_t i;
	uint8_t res;
	__attribute__((aligned(64))) uint8_t local_c[64];

	/* Store the result */
	_mm512_storeu_epi64((__m512i*)local_c, accu);
	/* Finish the xor computation byte pet byte  */
	res = 0;
	for(i = 0; i < 64; i++){
		res ^= local_c[i];
	}

	return res;
}

static inline uint16_t sum_uint16_gfni(__m512i accu) {
	uint32_t i;
	uint16_t res;
	__attribute__((aligned(64))) uint16_t local_c[32];

	/* Store the result */
	_mm512_storeu_epi64((__m512i*)local_c, accu);
	/* Finish the xor computation byte pet byte  */
	res = 0;
	for(i = 0; i < 32; i++){
		res ^= local_c[i];
	}

	return res;
}

/* === GF(2) === */
/* NOTE: for atomic multiplication, using vectorization is suboptimal */
static inline uint8_t gf2_mult_gfni(uint8_t a, uint8_t b)
{
        return gf2_mult_ref(a, b);
}

/*
 * Vector multiplied by a constant in GF(2).
 */
static inline void gf2_constant_vect_mult_gfni(uint8_t b, const uint8_t *a, uint8_t *c, uint32_t len)
{
        gf2_constant_vect_mult_ref(b, a, c, len);

        return;
}

/* XXX: TODO: This is not optimal, since we hardly fill our zmm register to its full capacity */
static inline uint8_t gf2_vect_mult_gfni(const uint8_t *a, const uint8_t *b, uint32_t len_bits)
{
	uint32_t i;
	__m512i accu, _a, _b;
	uint32_t len = (len_bits / 8);

	/* Set the accumulator to 0 */
	accu = _mm512_setzero_epi32();

	for(i = 0; i < len; i += 64){
                if((len-i) < 64){
                        _a = load_incomplete_m512(&a[i], len-i);
                        _b = load_incomplete_m512(&b[i], len-i);
                }
                else{
                        /* Obvious 512-bit */
                        _a = _mm512_loadu_epi64((__m512i*)&a[i]);
                        _b = _mm512_loadu_epi64((__m512i*)&b[i]);
                }
                /* Vectorized AND of inputs and then XOR with the accumulator */
                accu ^= (_a & _b);
	}

	return parity_gfni(accu);
}

/* Matrix and vector multiplication over GF(2) 
 * C = A * X, where X is a vector
 * Matrix is supposed to be square n x n, and vector n x 1
 * The output is a vector n x 1
 * */
/* XXX: TODO: this can be optimized by packing rows in zmm when n <= 256 */
static inline void gf2_mat_mult_gfni(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
        /* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
         * than using the trianglar shape ... */
	(void)mtype;
	GF2_MAT_MULT(A, X, Y, n, REG, gf2_vect_mult_gfni);
}

/* GF(2) matrix transposition */
static inline void gf2_mat_transpose_gfni(const uint8_t *A, uint8_t *B, uint32_t n, matrix_type mtype)
{
	gf2_mat_transpose_ref(A, B, n, mtype);
}

/* === GF(256) === */
/* NOTE: for atomic multiplication, using vectorization is suboptimal */
static inline uint8_t gf256_mult_gfni(uint8_t x, uint8_t y)
{ 
        return gf256_mult_ref(x, y);
}

static inline __m512i gf256_mult_vectorized_gfni(__m512i _a, __m512i _b)
{
	return _mm512_gf2p8mul_epi8(_a, _b);
}

/*              
 * Vector multiplied by a constant in GF(256).
 */                     
static inline void gf256_constant_vect_mult_gfni(uint8_t b, const uint8_t *a, uint8_t *c, uint32_t len)
{
        uint32_t i;
        __m512i _a, _b;

	/* Create a vector _b with the duplicated constant everywhere */
	_b = _mm512_set1_epi8(b);

        for(i = 0; i < len; i += 64){
                if((len-i) < 64){
                        _a = load_incomplete_m512(&a[i], len-i);
	                /* Vectorized multiplication in GF(256) */
        	        store_incomplete_m512(gf256_mult_vectorized_gfni(_a, _b), &c[i], len-i);
                }
                else{
                        /* Obvious 512-bit */
                        _a = _mm512_loadu_epi64((__m512i*)&a[i]);
	                /* Vectorized multiplication in GF(256) */
        	        _mm512_storeu_epi64(&c[i], gf256_mult_vectorized_gfni(_a, _b));
                }
        }

	return;
}

/*
 * Vector to vector multiplication in GF(256).
 * Takes two vectors of length 'len', and returns a byte (element in GF(256))
 */
static inline uint8_t gf256_vect_mult_gfni(const uint8_t *a, const uint8_t *b, uint32_t len)
{
        uint32_t i;
        __m512i accu, _a, _b;

        /* Set the accumulator to 0 */
        accu = _mm512_setzero_epi32();

        for(i = 0; i < len; i += 64){
                if((len-i) < 64){
                        _a = load_incomplete_m512(&a[i], len-i);
                        _b = load_incomplete_m512(&b[i], len-i);
                }
                else{
                        /* Obvious 512-bit */
                        _a = _mm512_loadu_epi64((__m512i*)&a[i]);
                        _b = _mm512_loadu_epi64((__m512i*)&b[i]);
                }
                /* Vectorized multiplication in GF(256) */
                accu ^= gf256_mult_vectorized_gfni(_a, _b);
        }

	return sum_uint8_gfni(accu);
}

/* Matrix and vector multiplication over GF(256) 
 * C += A * X, where X is a vector
 * Matrix is supposed to be square n x n, and vector n x 1
 * The output is a vector n x 1
 * */
static inline void gf256_mat_mult_gfni(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
	/* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
	 * than using the trianglar shape ... */
	(void)mtype;
	GF256_MAT_MULT(A, X, Y, n, REG, gf256_vect_mult_gfni);
}


/*
 * "Hybrid" multiplication of a constant in GF(2) and a vector in GF(256)
 */
static inline void gf2_gf256_constant_vect_mult_gfni(uint8_t a_gf2, const uint8_t *b_gf256, uint8_t *c_gf256, uint32_t n)
{
        gf2_gf256_constant_vect_mult_ref(a_gf2, b_gf256, c_gf256, n);

        return;
}

/*
 * "Hybrid" multiplication of a constant in GF(256) and a vector in GF(2)
 */
static inline void gf256_gf2_constant_vect_mult_gfni(uint8_t a_gf256, const uint8_t *b_gf2, uint8_t *c_gf256, uint32_t n)
{
        uint32_t i;
        const __m512i zero = _mm512_setzero_epi32();
        const __m128i zero_128 = _mm_setzero_si128();

	/* Load and broadcast the constant */
	__m512i _a = _mm512_set1_epi8(a_gf256);

        for(i = 0; i < n; i += 64){
                /* We use a mask load depending on the mask value in b_gf2 */
                uint32_t len = (n - i) < 64 ? (n - i) : 64;
                __m128i mask_128 = _mm_mask_loadu_epi8(zero_128, ((__mmask16)1 << (len / 8)) - 1, &b_gf2[(i / 8)]);
                /* Transfer to our 64 bits mask */
                __mmask64 mask = (__mmask64)_mm_movepi64_pi64(mask_128);
		__m512i _c = _mm512_mask_mov_epi8(zero, mask, _a);
		store_incomplete_m512(_c, &c_gf256[i], len);
        }

        return;
}

/*      
 * "Hybrid" scalar multiplication of a vector in GF(2) and a vector in GF(256)
 */
static inline uint8_t gf2_gf256_vect_mult_gfni(const uint8_t *a_gf2, const uint8_t *b_gf256, uint32_t n)
{
        uint32_t i;
        const __m512i zero = _mm512_setzero_epi32();
        const __m128i zero_128 = _mm_setzero_si128();

        /* Set the accumulator to 0 */
        __m512i accu = _mm512_setzero_epi32();

        for(i = 0; i < n; i += 64){
                /* We use a mask load depending on the mask value in a_gf2 */
                uint32_t len = (n - i) < 64 ? (n - i) : 64;
                __m128i mask_128 = _mm_mask_loadu_epi8(zero_128, ((__mmask16)1 << (len / 8)) - 1, &a_gf2[(i / 8)]);
                /* Transfer to our 64 bits mask */
                __mmask64 mask = (__mmask64)_mm_movepi64_pi64(mask_128);
                accu ^= _mm512_mask_loadu_epi8(zero, mask, (int const*)&b_gf256[i]);
        }

        return sum_uint8_gfni(accu);
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256) and a vector in GF(2)
 */
static inline uint8_t gf256_gf2_vect_mult_gfni(const uint8_t *a_gf256, const uint8_t *b_gf2, uint32_t n)
{
        return gf2_gf256_vect_mult_gfni(b_gf2, a_gf256, n);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(2) and a vector in GF(256), resulting
 *  in a vector in GF(256)
 */
static inline void gf2_gf256_mat_mult_gfni(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
	/* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
	 * than using the trianglar shape ... */
	(void)mtype;
        GF2_GF256_MAT_MULT(A, X, Y, n, REG, gf2_gf256_vect_mult_gfni);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256) and a vector in GF(2), resulting
 *  in a vector in GF(256)
 */
static inline void gf256_gf2_mat_mult_gfni(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
        /* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
         * than using the trianglar shape ... */
        (void)mtype;
        GF256_GF2_MAT_MULT(A, X, Y, n, REG, gf256_gf2_vect_mult_gfni);
}

/* GF(256) matrix transposition */
static inline void gf256_mat_transpose_gfni(const uint8_t *A, uint8_t *B, uint32_t n, matrix_type mtype)
{
	gf256_mat_transpose_ref(A, B, n, mtype);
}

/* === GF(256^2) === */
/* NOTE: for atomic multiplication, using vectorization is suboptimal */
static inline uint16_t gf256to2_mult_gfni(uint16_t x, uint16_t y)
{
        return gf256to2_mult_ref(x, y);
}

/* Vectorize multiplication of _a and _b in GF(256^2): the elements in the field are made of
 * 16 bits each in the lanes of the zmm */
static inline __m512i gf256to2_mult_vectorized_gfni(__m512i _a, __m512i _b)
{
	const __m512i shuff_msk_crossed = _mm512_set_epi8(62, 63, 60, 61, 58, 59, 56, 57, 54, 55, 52, 53,
				 		          50, 51, 48, 49, 46, 47, 44, 45, 42, 43, 40, 41,
						          38, 39, 36, 37, 34, 35, 32, 33, 30, 31, 28, 29,
						          26, 27, 24, 25, 22, 23, 20, 21, 18, 19, 16, 17,
						          14, 15, 12, 13, 10, 11, 8 ,  9,  6,  7,  4,  5,
						          2,  3,  0,  1);
	const __m512i shuff_msk1 = _mm512_set_epi8(62, 62, 60, 60, 58, 58, 56, 56, 54, 54, 52, 52,
                                                   50, 50, 48, 48, 46, 46, 44, 44, 42, 42, 40, 40,
                                                   38, 38, 36, 36, 34, 34, 32, 32, 30, 30, 28, 28,
                                                   26, 26, 24, 24, 22, 22, 20, 20, 18, 18, 16, 16,
                                                   14, 14, 12, 12, 10, 10, 8 ,  8,  6,  6,  4,  4,
                                                   2,  2,  0,  0);
	const __m512i shuff_msk2 = _mm512_set_epi8(63, 63, 61, 61, 59, 59, 57, 57, 55, 55, 53, 53,
                                                   51, 51, 49, 49, 47, 47, 45, 45, 43, 43, 41, 41,
                                                   39, 39, 37, 37, 35, 35, 33, 33, 31, 31, 29, 29,
                                                   27, 27, 25, 25, 23, 23, 21, 21, 19, 19, 17, 17,
                                                   15, 15, 13, 13, 11, 11, 9 ,  9,  7,  7,  5,  5,
                                                   3,  3,  1,  1);
	const __m512i const32 = _mm512_set_epi64(0x0020002000200020, 0x0020002000200020, 0x0020002000200020, 0x0020002000200020,
						 0x0020002000200020, 0x0020002000200020, 0x0020002000200020, 0x0020002000200020);
	const __m512i zero = _mm512_setzero_epi32();
	
	__m512i ab = gf256_mult_vectorized_gfni(_a, _b);
	__m512i a0b0 = _mm512_shuffle_epi8(ab, shuff_msk1);
	__m512i a1b1 = _mm512_shuffle_epi8(ab, shuff_msk2);
	__m512i a1b1_32 = gf256_mult_vectorized_gfni(a1b1, const32);
	/* */
	__m512i a0_xor_a1 = _a ^ _mm512_shuffle_epi8(_a, shuff_msk_crossed);
	__m512i b0_xor_b1 = _mm512_mask_blend_epi8((__mmask64)0xaaaaaaaaaaaaaaaa, zero, _b ^ _mm512_shuffle_epi8(_b, shuff_msk_crossed));
	__m512i mult_ab_xor = gf256_mult_vectorized_gfni(a0_xor_a1, b0_xor_b1);

	/* Compute the result */
	__m512i res = a0b0 ^ a1b1_32 ^ mult_ab_xor;

	return res;
}

/*                      
 * Vector multiplied by a constant in GF(256^2).
 */
static inline void gf256to2_constant_vect_mult_gfni(uint16_t b, const uint16_t *a, uint16_t *c, uint32_t len)
{       
         uint32_t i;
        __m512i _a, _b;

	/* Create a vector _b with the duplicated constant everywhere */
	_b = _mm512_set1_epi16(b);

        for(i = 0; i < (2 * len); i += 64){
                if(((2 * len)-i) < 64){
                        _a = load_incomplete_m512((const uint8_t*)&a[i / 2], (2*len)-i);
	                /* Vectorized multiplication in GF(256^2) */
        	        store_incomplete_m512(gf256to2_mult_vectorized_gfni(_a, _b), (uint8_t*)&c[i / 2], (2*len)-i);
                }
                else{
                        /* Obvious 512-bit */
                        _a = _mm512_loadu_epi64((__m512i*)&a[i / 2]);
	                /* Vectorized multiplication in GF(256^2) */
        	        _mm512_storeu_epi64(&c[i / 2], gf256to2_mult_vectorized_gfni(_a, _b));
                }
        }

        return; 
}

/* Perform a multiplication in GF(256^2) of elements in vectors a an b */
static inline uint16_t gf256to2_vect_mult_gfni(const uint16_t *a, const uint16_t *b, uint32_t len)
{
	uint32_t i;
	__m512i accu, _a, _b;

	/* Set the accumulator to 0 */
	accu = _mm512_setzero_epi32();

	for(i = 0; i < (2 * len); i += 64){
                if(((2 * len)-i) < 64){
                        _a = load_incomplete_m512((const uint8_t*)&a[i / 2], ((2 * len) - i));
                        _b = load_incomplete_m512((const uint8_t*)&b[i / 2], ((2 * len) - i));
                }
                else{
                        /* Obvious 512-bit */
                        _a = _mm512_loadu_epi64((__m512i*)&a[i / 2]);
                        _b = _mm512_loadu_epi64((__m512i*)&b[i / 2]);
                }
                accu ^= gf256to2_mult_vectorized_gfni(_a, _b);
	}

	return sum_uint16_gfni(accu);
}

/*
 * "Hybrid" constant multiplication of a constant in GF(2) and a vector in GF(256^2)
 */
static inline void gf2_gf256to2_constant_vect_mult_gfni(uint8_t a_gf2, const uint16_t *b_gf256to2, uint16_t *c_gf256to2, uint32_t n)
{
        gf2_gf256to2_constant_vect_mult_ref(a_gf2, b_gf256to2, c_gf256to2, n);

        return;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256^2) and a vector in GF(2)
 */   
static inline void gf256to2_gf2_constant_vect_mult_gfni(uint16_t a_gf256to2, const uint8_t *b_gf2, uint16_t *c_gf256to2, uint32_t n)
{
        uint32_t i;
        const __m512i zero = _mm512_setzero_epi32();
        const __m128i zero_128 = _mm_setzero_si128();

	/* Load and broadcast the constant */
	__m512i _a = _mm512_set1_epi16(a_gf256to2);

        for(i = 0; i < n; i += 32){
                /* We use a mask load depending on the mask value in b_gf2 */
                uint32_t len = (n - i) < 32 ? (n - i) : 32;
                __m128i mask_128 = _mm_mask_loadu_epi8(zero_128, ((__mmask16)1 << (len / 8)) - 1, &b_gf2[(i / 8)]);
                /* Transfer to our 64 bits mask */
                __mmask64 mask64 = (__mmask64)_mm_movepi64_pi64(mask_128);
		__mmask32 mask = (__mmask32)mask64;
		__m512i _c = _mm512_mask_mov_epi16(zero, mask, _a);
		store_incomplete_m512(_c, (uint8_t*)&c_gf256to2[i], 2 * len);
        }

        return;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256^2) and a vector in GF(256)
 */
static inline void gf256_gf256to2_constant_vect_mult_gfni(uint8_t a_gf256, const uint16_t *b_gf256to2, uint16_t *c_gf256to2, uint32_t n)
{
        gf256_gf256to2_constant_vect_mult_ref(a_gf256, b_gf256to2, c_gf256to2, n);

        return;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256^2) and a vector in GF(256)
 */
static inline void gf256to2_gf256_constant_vect_mult_gfni(uint16_t a_gf256to2, const uint8_t *b_gf256, uint16_t *c_gf256to2, uint32_t len)
{
        uint32_t i;
        __m512i _a, _b;                    
        const __m512i shuff_msk = _mm512_set_epi8(31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 
                                                  23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 
                                                  15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 
                                                  7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,  2,  1, 1, 0, 0);

        /* Create a vector _b with the duplicated constant everywhere */
        _a = _mm512_set1_epi16(a_gf256to2);
 
        for(i = 0; i < len; i += 32){
                uint32_t to_load = (len - i) < 32 ? (len - i) : 32;
                _b = load_incomplete_m512((const uint8_t*)&b_gf256[i], to_load);
                /* Duplicate the values in _b */
                _b = _mm512_permutex2var_epi8(_b, shuff_msk, _b);
                /* Vectorized multiplication in GF(256^2) */
                store_incomplete_m512(gf256_mult_vectorized_gfni(_a, _b), (uint8_t*)&c_gf256to2[i], 2 * to_load);
        }
        return;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(2) and a vector in GF(256^2)
 */
static inline uint16_t gf2_gf256to2_vect_mult_gfni(const uint8_t *a_gf2, const uint16_t *b_gf256to2, uint32_t n)
{
        uint32_t i;
        const __m512i zero = _mm512_setzero_epi32();
        const __m128i zero_128 = _mm_setzero_si128();

        /* Set the accumulator to 0 */
        __m512i accu = _mm512_setzero_epi32();

        for(i = 0; i < n; i += 32){
                /* We use a mask load depending on the mask value in a_gf2 */
                uint32_t len = (n - i) < 32 ? (n - i) : 32;
		/* Load 32 bits max */
                __m128i mask_128 = _mm_mask_loadu_epi8(zero_128, ((__mmask16)1 << (len / 8)) - 1, &a_gf2[(i / 8)]);
                /* Transfer to our 32 bits mask */
                __mmask64 mask64 = (__mmask64)_mm_movepi64_pi64(mask_128);
		__mmask32 mask = (__mmask32)mask64;
                accu ^= _mm512_mask_loadu_epi16(zero, mask, (int const*)&b_gf256to2[i]);
        }

	return sum_uint16_gfni(accu);
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256^2) and a vector in GF(256)
 */
static inline uint16_t gf256to2_gf2_vect_mult_gfni(const uint16_t *a_gf256to2, const uint8_t *b_gf2, uint32_t n)
{
        return gf2_gf256to2_vect_mult_gfni(b_gf2, a_gf256to2, n);
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256) and a vector in GF(256^2)
 */
static inline uint16_t gf256_gf256to2_vect_mult_gfni(const uint8_t *a_gf256, const uint16_t *b_gf256to2, uint32_t len)
{
	/* Note: the multiplication of an element in GF(256) and an element in GF(256^2)
         * simply consists in two multiplications in GF(256) (this is multiplying a constant by a degree 1 polynomial)
         * */
	uint32_t i;
	__m512i accu, _a, _b;

	/* Set the accumulator to 0 */
	accu = _mm512_setzero_epi32();

	for(i = 0; i < (2 * len); i += 64){
		const __m512i shuff_msk = _mm512_set_epi8(31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24,
							  23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16,
							  15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 
							  7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2,  2,  1, 1, 0, 0);
                if(((2 * len)-i) < 64){
                        _a = load_incomplete_m512((const uint8_t*)&a_gf256[i / 2], ((2 * len)-i) / 2);
                        _b = load_incomplete_m512((const uint8_t*)&b_gf256to2[i / 2], (2 * len)-i);
                }
                else{
                        /* Obvious 512-bit */
                        _a = load_incomplete_m512((const uint8_t*)&a_gf256[i / 2], 32);
                        _b = _mm512_loadu_epi64((__m512i*)&b_gf256to2[i / 2]);
                }
                /* Duplicate the values in _a */
		_a = _mm512_permutex2var_epi8(_a, shuff_msk, _a);
		/* Multiply in GF(256) */		
                accu ^= gf256_mult_vectorized_gfni(_a, _b);
	}

	return sum_uint16_gfni(accu);
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256^2) and a vector in GF(256)
 */
static inline uint16_t gf256to2_gf256_vect_mult_gfni(const uint16_t *a_gf256to2, const uint8_t *b_gf256, uint32_t n)
{
        return gf256_gf256to2_vect_mult_gfni(b_gf256, a_gf256to2, n);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(2) and a vector in GF(256^2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf2_gf256to2_mat_mult_gfni(const uint8_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        /* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
         * than using the trianglar shape ... */
        (void)mtype;
        GF2_GF256to2_MAT_MULT(A, X, Y, n, REG, gf2_gf256to2_vect_mult_gfni);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256^2) and a vector in GF(2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256to2_gf2_mat_mult_gfni(const uint16_t *A, const uint8_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        /* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
         * than using the trianglar shape ... */
        (void)mtype;
        GF256to2_GF2_MAT_MULT(A, X, Y, n, REG, gf256to2_gf2_vect_mult_gfni);
}

/*
 * GF(2^16) matrix multiplication
 */
static inline void gf256to2_mat_mult_gfni(const uint16_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
	/* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
	 * than using the trianglar shape ... */
	(void)mtype;
        GF256to2_MAT_MULT(A, X, Y, n, REG, gf256to2_vect_mult_gfni);
}


/*
 * "Hybrid" matrix multiplication of a matrix in GF(256) and a vector in GF(256^2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256_gf256to2_mat_mult_gfni(const uint8_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
	/* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
	 * than using the trianglar shape ... */
	(void)mtype;
        GF256to2_MAT_MULT(A, X, Y, n, REG, gf256_gf256to2_vect_mult_gfni);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256^2) and a vector in GF(256), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256to2_gf256_mat_mult_gfni(const uint16_t *A, const uint8_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        /* XXX: NOTE: because of alignment and loading latencies, treating the matrix as "regular" seems always better
         * than using the trianglar shape ... */
        (void)mtype;
        GF256to2_MAT_MULT(A, X, Y, n, REG, gf256to2_gf256_vect_mult_gfni);
}

/* GF(256^2) matrix transposition */
static inline void gf256to2_mat_transpose_gfni(const uint16_t *A, uint16_t *B, uint32_t n, matrix_type mtype)
{
	gf256to2_mat_transpose_ref(A, B, n, mtype);
}


#endif /* defined(__GFNI__) && defined(__AVX512BW__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__) */

#endif /* __FIELDS_GFNI_H__ */
