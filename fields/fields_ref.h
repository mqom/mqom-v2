#ifndef __FIELDS_REF_H__
#define __FIELDS_REF_H__

#include "fields_common.h"
#include <string.h>

/* === GF(2) === */
/* GF(2) elements addition is xor, multiplication is and */
static inline uint8_t gf2_mult_ref(uint8_t a, uint8_t b)
{
	return a & b;
}

/*
 * Vector multiplied by a constant in GF(2).
 */
static inline void gf2_constant_vect_mult_ref(uint8_t b, const uint8_t *a, uint8_t *c, uint32_t len)
{
	uint32_t i;

	/* Constant-time selection */
	for(i = 0; i < (len / 8); i++){
		c[i] = -(b & 1) & a[i];
	}

	return;
}

/*
 * Vector to vector multiplication in GF(2).
 * Takes two vectors of length 'len', and returns a bit (element in GF(2))
 */
static inline uint8_t gf2_vect_mult_ref(const uint8_t *a, const uint8_t *b, uint32_t len)
{
	uint32_t i;
	uint8_t acc, res;

	acc = 0;
	for(i = 0; i < (len / 8); i++){
		acc ^= (a[i] & b[i]);
	}
	/* Compute the parity of acc as the output result */
	res = 0;
	for(i = 0; i < 8; i++){
		res ^= (acc >> i) & 1;
	}

	return res;
}

/* Matrix and vector multiplication over GF(2) 
 * C = A * X, where X is a vector
 * Matrix is supposed to be square n x n, and vector n x 1
 * The output is a vector n x 1
 * */
static inline void gf2_mat_mult_ref(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
	GF2_MAT_MULT(A, X, Y, n, mtype, gf2_vect_mult_ref);
}

/* GF(2) matrix transposition */
static inline void gf2_mat_transpose_ref(const uint8_t *A, uint8_t *B, uint32_t n, matrix_type mtype)
{
	uint32_t i, j;
	uint32_t msize = (n / 8);

	/* Zeroize the matrix */
	for(i = 0; i < (n * msize); i++){
		B[i] = 0;
	}
	switch(mtype){
		case REG:{
			for(i = 0; i < n; i++){
				for(j = 0; j < n; j++){					
					B[(n * j / 8) + (i / 8)] |= ((A[(n * i / 8) + (j / 8)] >> (j % 8)) & 1) << (i % 8);
				} 
			}
			break;
		}
		case TRI_SUP:{
			for(i = 0; i < n; i++){
				for(j = i; j < n; j++){					
					B[(n * j / 8) + (i / 8)] |= ((A[(n * i / 8) + (j / 8)] >> (j % 8)) & 1) << (i % 8);
				} 
			}
			break;
		}
		case TRI_INF:{
			for(i = 0; i < n; i++){
				for(j = 0; j <= i; j++){					
					B[(n * j / 8) + (i / 8)] |= ((A[(n * i / 8) + (j / 8)] >> (j % 8)) & 1) << (i % 8);
				} 
			}
			break;
		}
		default:{
			break;
		}
	}
}

/* === GF(256) === */
#define GF256_MODULUS 0x1B /* The GF(2^8) modulus (Rijndael polynomial) */
static inline uint8_t gf256_mult_ref(uint8_t x, uint8_t y)
{
	uint8_t res;

	res = (-(y >> 7) & x);
	res = (-(y >> 6 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y >> 5 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y >> 4 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y >> 3 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y >> 2 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y >> 1 & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);
	res = (-(y      & 1) & x) ^ (-(res >> 7) & GF256_MODULUS) ^ (res << 1);

	return res;
}

/*
 * Vector multiplied by a constant in GF(256).
 */
static inline void gf256_constant_vect_mult_ref(uint8_t b, const uint8_t *a, uint8_t *c, uint32_t len)
{
	uint32_t i;

	for(i = 0; i < len; i++){
		c[i] = gf256_mult_ref(a[i], b);
	}

	return;
}

/*
 * Vector to vector multiplication in GF(256).
 * Takes two vectors of length 'len', and returns a byte (element in GF(256))
 */
static inline uint8_t gf256_vect_mult_ref(const uint8_t *a, const uint8_t *b, uint32_t len)
{
	uint32_t i;
	uint8_t res;

	res = 0;
	for(i = 0; i < len; i++){
		res ^= gf256_mult_ref(a[i], b[i]);
	}
	
	return res;
}

/* Matrix and vector multiplication over GF(256) 
 * C += A * X, where X is a vector
 * Matrix is supposed to be square n x n, and vector n x 1
 * The output is a vector n x 1
 * */
static inline void gf256_mat_mult_ref(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
	GF256_MAT_MULT(A, X, Y, n, mtype, gf256_vect_mult_ref);
}

/*
 * "Hybrid" multiplication of a constant in GF(2) and a vector in GF(256)
 */
static inline void gf2_gf256_constant_vect_mult_ref(uint8_t a_gf2, const uint8_t *b_gf256, uint8_t *c_gf256, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256[i] = b_gf256[i] & -(a_gf2 & 1);
	}

	return;
}

/*
 * "Hybrid" multiplication of a constant in GF(256) and a vector in GF(2)
 */
static inline void gf256_gf2_constant_vect_mult_ref(uint8_t a_gf256, const uint8_t *b_gf2, uint8_t *c_gf256, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256[i] = a_gf256 & -((b_gf2[i / 8] >> (i % 8)) & 1);
	}

	return;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(2) and a vector in GF(256)
 */
static inline uint8_t gf2_gf256_vect_mult_ref(const uint8_t *a_gf2, const uint8_t *b_gf256, uint32_t n)
{
	uint32_t i;
	uint8_t res;

	res = 0;
	for(i = 0; i < n; i++){
		/* Conditionally add the element from GF(256) in a constant time fashion */
		res ^= b_gf256[i] & -((a_gf2[i / 8] >> (i % 8)) & 1);
	}

	return res;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256) and a vector in GF(2)
 */
static inline uint8_t gf256_gf2_vect_mult_ref(const uint8_t *a_gf256, const uint8_t *b_gf2, uint32_t n)
{
	return gf2_gf256_vect_mult_ref(b_gf2, a_gf256, n);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(2) and a vector in GF(256), resulting
 *  in a vector in GF(256)
 */
static inline void gf2_gf256_mat_mult_ref(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
        GF2_GF256_MAT_MULT(A, X, Y, n, mtype, gf2_gf256_vect_mult_ref);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256) and a vector in GF(2), resulting
 *  in a vector in GF(256)
 */
static inline void gf256_gf2_mat_mult_ref(const uint8_t *A, const uint8_t *X, uint8_t *Y, uint32_t n, matrix_type mtype)
{
        GF256_GF2_MAT_MULT(A, X, Y, n, mtype, gf256_gf2_vect_mult_ref);
}

/* GF(256) matrix transposition */
static inline void gf256_mat_transpose_ref(const uint8_t *A, uint8_t *B, uint32_t n, matrix_type mtype)
{
	GF256_MAT_TRANSPOSE(A, B, n, mtype);
}

/* === GF(256^2) === */
/*
 * For GF(2^16) elements, we use a GF(256^2) representation as two elements (a1, a0), with
 * a0 and a1 in GF(256) and a1 * X + a0 representing the element as a polynomial. The reduction
 * polynomial with coefficients in GF(256) is X^2 + X + 32, as it is irreductible in this field.
 *
 */
static inline uint16_t gf256to2_mult_ref(uint16_t a, uint16_t b)
{
	uint8_t c0, c1, a0, a1, b0, b1, a1b1, a0b0;

	a0 = a & 0xff;
	a1 = a >> 8;
	b0 = b & 0xff;
	b1 = b >> 8;
	/**/
	a1b1 = gf256_mult_ref(a1, b1);
	a0b0 = gf256_mult_ref(a0, b0);
	/**/
	c0 = a0b0 ^ gf256_mult_ref(a1b1, 32);
	c1 = a0b0 ^ gf256_mult_ref(a0 ^ a1, b0 ^ b1);

	return (((uint16_t)c1 << 8) ^ c0);
}

/*
 * Vector multiplied by a constant in GF(256^2).
 */
static inline void gf256to2_constant_vect_mult_ref(uint16_t b, const uint16_t *a, uint16_t *c, uint32_t len)
{
	uint32_t i;

	for(i = 0; i < len; i++){
		c[i] = gf256to2_mult_ref(a[i], b);
	}

	return;
}

/*
 * GF(2^16) scalar multiplication
 */
static inline uint16_t gf256to2_vect_mult_ref(const uint16_t *a, const uint16_t *b, uint32_t len)
{
	uint32_t i;
	uint16_t res;

	res = 0;
	for(i = 0; i < len; i++){
		res ^= gf256to2_mult_ref(a[i], b[i]);
	}

	return res;
}

/*
 * GF(2^16) matrix multiplication
 */
static inline void gf256to2_mat_mult_ref(const uint16_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
	GF256to2_MAT_MULT(A, X, Y, n, mtype, gf256to2_vect_mult_ref);
}

/*
 * "Hybrid" constant multiplication of a constant in GF(2) and a vector in GF(256^2)
 */
static inline void gf2_gf256to2_constant_vect_mult_ref(uint8_t a_gf2, const uint16_t *b_gf256to2, uint16_t *c_gf256to2, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256to2[i] = b_gf256to2[i] & -(uint16_t)(a_gf2 & 1);
	}

	return;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256^2) and a vector in GF(2)
 */
static inline void gf256to2_gf2_constant_vect_mult_ref(uint16_t a_gf256to2, const uint8_t *b_gf2, uint16_t *c_gf256to2, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256to2[i] = a_gf256to2 & -(uint16_t)((b_gf2[i / 8] >> (i % 8)) & 1);
	}

	return;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(2) and a vector in GF(256^2)
 */
static inline uint16_t gf2_gf256to2_vect_mult_ref(const uint8_t *a_gf2, const uint16_t *b_gf256to2, uint32_t n)
{
	uint32_t i;
	uint16_t res;

	res = 0;
	for(i = 0; i < n; i++){
		res ^= b_gf256to2[i] & -(uint16_t)((a_gf2[i / 8] >> (i % 8)) & 1);
	}

	return res;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256) and a vector in GF(256^2)
 */
static inline void gf256_gf256to2_constant_vect_mult_ref(uint8_t a_gf256, const uint16_t *b_gf256to2, uint16_t *c_gf256to2, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256to2[i] = (gf256_mult_ref(a_gf256, b_gf256to2[i] >> 8) << 8) | gf256_mult_ref(a_gf256, b_gf256to2[i] & 0xff);
	}

	return;
}

/*
 * "Hybrid" constant multiplication of a constant in GF(256^2) and a vector in GF(256)
 */
static inline void gf256to2_gf256_constant_vect_mult_ref(uint16_t a_gf256to2, const uint8_t *b_gf256, uint16_t *c_gf256to2, uint32_t n)
{
	uint32_t i;

	for(i = 0; i < n; i++){
		c_gf256to2[i] = (gf256_mult_ref(b_gf256[i], a_gf256to2 >> 8) << 8) | gf256_mult_ref(b_gf256[i], a_gf256to2 & 0xff);
	}

	return;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256^2) and a vector in GF(256)
 */
static inline uint16_t gf256to2_gf2_vect_mult_ref(const uint16_t *a_gf256to2, const uint8_t *b_gf2, uint32_t n)
{
	return gf2_gf256to2_vect_mult_ref(b_gf2, a_gf256to2, n);
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256) and a vector in GF(256^2)
 */
static inline uint16_t gf256_gf256to2_vect_mult_ref(const uint8_t *a_gf256, const uint16_t *b_gf256to2, uint32_t n)
{
	uint32_t i;
	uint16_t res;

	res = 0;
	for(i = 0; i < n; i++){
		/* Note: the multiplication of an element in GF(256) and an element in GF(256^2)
 		 * simply consists in two multiplications in GF(256) (this is multiplying a constant by a degree 1 polynomial)
 		 * */
		res ^= (gf256_mult_ref(a_gf256[i], b_gf256to2[i] >> 8) << 8) | gf256_mult_ref(a_gf256[i], b_gf256to2[i] & 0xff);
	}

	return res;
}

/*
 * "Hybrid" scalar multiplication of a vector in GF(256^2) and a vector in GF(256)
 */
static inline uint16_t gf256to2_gf256_vect_mult_ref(const uint16_t *a_gf256to2, const uint8_t *b_gf256, uint32_t n)
{
	return gf256_gf256to2_vect_mult_ref(b_gf256, a_gf256to2, n);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(2) and a vector in GF(256^2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf2_gf256to2_mat_mult_ref(const uint8_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        GF2_GF256to2_MAT_MULT(A, X, Y, n, mtype, gf2_gf256to2_vect_mult_ref);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256^2) and a vector in GF(2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256to2_gf2_mat_mult_ref(const uint16_t *A, const uint8_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        GF256to2_GF2_MAT_MULT(A, X, Y, n, mtype, gf256to2_gf2_vect_mult_ref);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256) and a vector in GF(256^2), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256_gf256to2_mat_mult_ref(const uint8_t *A, const uint16_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        GF256to2_MAT_MULT(A, X, Y, n, mtype, gf256_gf256to2_vect_mult_ref);
}

/*
 * "Hybrid" matrix multiplication of a matrix in GF(256^2) and a vector in GF(256), resulting
 *  in a vector in GF(256^2)
 */
static inline void gf256to2_gf256_mat_mult_ref(const uint16_t *A, const uint8_t *X, uint16_t *Y, uint32_t n, matrix_type mtype)
{
        GF256to2_MAT_MULT(A, X, Y, n, mtype, gf256to2_gf256_vect_mult_ref);
}

/* GF(256^2) matrix transposition */
static inline void gf256to2_mat_transpose_ref(const uint16_t *A, uint16_t *B, uint32_t n, matrix_type mtype)
{
	GF256to2_MAT_TRANSPOSE(A, B, n, mtype);
}

#endif /* __FIELDS_REF_H__ */
