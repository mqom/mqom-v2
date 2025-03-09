#include "rijndael_platform.h"

#if defined(RIJNDAEL_AES_NI)

/* This file mainly contains an optimized implementation
 * of the AES-128 and AES-256 using AES-NI intrinsics. This
 * is the only file of the whole project that has an adherence
 * with the hardware as it expects to run on an Intel or AMD
 * platform supporting AES-NI.
 */

#include "rijndael_aes_ni.h"

/* ======== AES-128 ================ */
#define DO_ENC_BLOCK_128_128(m,k) \
	do{\
		m = _mm_xor_si128       (m, k[ 0]); \
		m = _mm_aesenc_si128	(m, k[ 1]); \
		m = _mm_aesenc_si128	(m, k[ 2]); \
		m = _mm_aesenc_si128	(m, k[ 3]); \
		m = _mm_aesenc_si128	(m, k[ 4]); \
		m = _mm_aesenc_si128	(m, k[ 5]); \
		m = _mm_aesenc_si128	(m, k[ 6]); \
		m = _mm_aesenc_si128	(m, k[ 7]); \
		m = _mm_aesenc_si128	(m, k[ 8]); \
		m = _mm_aesenc_si128	(m, k[ 9]); \
		m = _mm_aesenclast_si128(m, k[10]);\
	}while(0)

#define DO_ENC_BLOCK_128_128_x2(m1,m2,k1,k2) \
	do{\
		m1 = _mm_xor_si128       (m1, k1[ 0]); \
		m2 = _mm_xor_si128       (m2, k2[ 0]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 1]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 1]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 2]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 2]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 3]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 3]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 4]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 4]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 5]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 5]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 6]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 6]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 7]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 7]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 8]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 8]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 9]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 9]); \
		m1 = _mm_aesenclast_si128(m1, k1[10]);\
		m2 = _mm_aesenclast_si128(m2, k2[10]);\
	}while(0)

#define DO_ENC_BLOCK_128_128_x4(m1,m2,m3,m4,k1,k2,k3,k4) \
	do{\
		m1 = _mm_xor_si128       (m1, k1[ 0]); \
		m2 = _mm_xor_si128       (m2, k2[ 0]); \
		m3 = _mm_xor_si128       (m3, k3[ 0]); \
		m4 = _mm_xor_si128       (m4, k4[ 0]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 1]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 1]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 1]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 1]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 2]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 2]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 2]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 2]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 3]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 3]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 3]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 3]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 4]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 4]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 4]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 4]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 5]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 5]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 5]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 5]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 6]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 6]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 6]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 6]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 7]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 7]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 7]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 7]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 8]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 8]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 8]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 8]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 9]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 9]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 9]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 9]); \
		m1 = _mm_aesenclast_si128(m1, k1[10]);\
		m2 = _mm_aesenclast_si128(m2, k2[10]);\
		m3 = _mm_aesenclast_si128(m3, k3[10]);\
		m4 = _mm_aesenclast_si128(m4, k4[10]);\
	}while(0)

/* ======== AES-256 ================ */
#define DO_ENC_BLOCK_128_256(m,k) \
	do{\
		m = _mm_xor_si128       (m, k[ 0]); \
		m = _mm_aesenc_si128	(m, k[ 1]); \
		m = _mm_aesenc_si128	(m, k[ 2]); \
		m = _mm_aesenc_si128	(m, k[ 3]); \
		m = _mm_aesenc_si128	(m, k[ 4]); \
		m = _mm_aesenc_si128	(m, k[ 5]); \
		m = _mm_aesenc_si128	(m, k[ 6]); \
		m = _mm_aesenc_si128	(m, k[ 7]); \
		m = _mm_aesenc_si128	(m, k[ 8]); \
		m = _mm_aesenc_si128	(m, k[ 9]); \
		m = _mm_aesenc_si128	(m, k[10]); \
		m = _mm_aesenc_si128	(m, k[11]); \
		m = _mm_aesenc_si128	(m, k[12]); \
		m = _mm_aesenc_si128	(m, k[13]); \
		m = _mm_aesenclast_si128(m, k[14]);\
	}while(0)

#define DO_ENC_BLOCK_128_256_x2(m1,m2,k1,k2) \
	do{\
		m1 = _mm_xor_si128       (m1, k1[ 0]); \
		m2 = _mm_xor_si128       (m2, k2[ 0]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 1]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 1]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 2]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 2]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 3]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 3]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 4]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 4]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 5]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 5]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 6]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 6]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 7]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 7]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 8]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 8]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 9]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 9]); \
		m1 = _mm_aesenc_si128	(m1, k1[10]); \
		m2 = _mm_aesenc_si128	(m2, k2[10]); \
		m1 = _mm_aesenc_si128	(m1, k1[11]); \
		m2 = _mm_aesenc_si128	(m2, k2[11]); \
		m1 = _mm_aesenc_si128	(m1, k1[12]); \
		m2 = _mm_aesenc_si128	(m2, k2[12]); \
		m1 = _mm_aesenc_si128	(m1, k1[13]); \
		m2 = _mm_aesenc_si128	(m2, k2[13]); \
		m1 = _mm_aesenclast_si128(m1, k1[14]);\
		m2 = _mm_aesenclast_si128(m2, k2[14]);\
	}while(0)

#define DO_ENC_BLOCK_128_256_x4(m1,m2,m3,m4,k1,k2,k3,k4) \
	do{\
		m1 = _mm_xor_si128       (m1, k1[ 0]); \
		m2 = _mm_xor_si128       (m2, k2[ 0]); \
		m3 = _mm_xor_si128       (m3, k3[ 0]); \
		m4 = _mm_xor_si128       (m4, k4[ 0]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 1]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 1]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 1]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 1]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 2]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 2]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 2]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 2]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 3]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 3]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 3]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 3]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 4]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 4]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 4]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 4]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 5]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 5]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 5]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 5]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 6]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 6]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 6]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 6]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 7]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 7]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 7]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 7]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 8]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 8]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 8]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 8]); \
		m1 = _mm_aesenc_si128	(m1, k1[ 9]); \
		m2 = _mm_aesenc_si128	(m2, k2[ 9]); \
		m3 = _mm_aesenc_si128	(m3, k3[ 9]); \
		m4 = _mm_aesenc_si128	(m4, k4[ 9]); \
		m1 = _mm_aesenc_si128	(m1, k1[10]); \
		m2 = _mm_aesenc_si128	(m2, k2[10]); \
		m3 = _mm_aesenc_si128	(m3, k3[10]); \
		m4 = _mm_aesenc_si128	(m4, k4[10]); \
		m1 = _mm_aesenc_si128	(m1, k1[11]); \
		m2 = _mm_aesenc_si128	(m2, k2[11]); \
		m3 = _mm_aesenc_si128	(m3, k3[11]); \
		m4 = _mm_aesenc_si128	(m4, k4[11]); \
		m1 = _mm_aesenc_si128	(m1, k1[12]); \
		m2 = _mm_aesenc_si128	(m2, k2[12]); \
		m3 = _mm_aesenc_si128	(m3, k3[12]); \
		m4 = _mm_aesenc_si128	(m4, k4[12]); \
		m1 = _mm_aesenc_si128	(m1, k1[13]); \
		m2 = _mm_aesenc_si128	(m2, k2[13]); \
		m3 = _mm_aesenc_si128	(m3, k3[13]); \
		m4 = _mm_aesenc_si128	(m4, k4[13]); \
		m1 = _mm_aesenclast_si128(m1, k1[14]);\
		m2 = _mm_aesenclast_si128(m2, k2[14]);\
		m3 = _mm_aesenclast_si128(m3, k3[14]);\
		m4 = _mm_aesenclast_si128(m4, k4[14]);\
	}while(0)

/* ======== Rijndael-256 ================ */
/* 256 bits blocks: ml is left block, mr is right block */
#define RINJDAEL_COMPENSATE_SR(ml, mr, tmpl, tmpr, mblend, mshuff) \
	do{\
		/* This compensates for the AES shiftrows to */	\
		/* conform to Rijdael one */              	\
		tmpl = _mm_blendv_epi8(ml, mr, mblend); \
		tmpr = _mm_blendv_epi8(mr, ml, mblend); \
		tmpl = _mm_shuffle_epi8(tmpl, mshuff); \
		tmpr = _mm_shuffle_epi8(tmpr, mshuff); \
	}while(0)

#define ENC_256_BLOCK(ml, mr, kl, kr, mb, ms) \
	do{\
		__m128i tmpl, tmpr;  						\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR(ml, mr, tmpl, tmpr, mb, ms);		\
		ml = _mm_aesenc_si128	(tmpl, kl); 				\
		mr = _mm_aesenc_si128	(tmpr, kr); 				\
	}while(0)

#define ENC_256_BLOCK_LAST(ml, mr, kl, kr, mblend, mshuff) \
	do{\
		__m128i tmpl, tmpr;  						\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR(ml, mr, tmpl, tmpr, mblend, mshuff);	\
		ml = _mm_aesenclast_si128	(tmpl, kl); 			\
		mr = _mm_aesenclast_si128	(tmpr, kr); 			\
	}while(0)

#define DO_ENC_BLOCK_256_256(ml, mr, k) \
	do{\
		/* ShiftRows compensation constants */		\
		const __m128i mblend = _mm_set_epi32(0x80000000, 0x80800000, 0x80800000, 0x80808000); \
		const __m128i mshuff = _mm_set_epi32(0x03020d0c, 0x0f0e0908, 0x0b0a0504, 0x07060100); \
		/**/						\
		ml = _mm_xor_si128       (ml, k[ 0]); 		\
		mr = _mm_xor_si128       (mr, k[ 1]); 		\
		/**/						\
		ENC_256_BLOCK(ml, mr, k[2], k[3]  , mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[4], k[5]  , mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[6], k[7]  , mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[8], k[9]  , mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[10], k[11], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[12], k[13], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[14], k[15], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[16], k[17], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[18], k[19], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[20], k[21], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[22], k[23], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[24], k[25], mblend, mshuff); \
		ENC_256_BLOCK(ml, mr, k[26], k[27], mblend, mshuff); \
		/**/						\
		ENC_256_BLOCK_LAST(ml, mr, k[28], k[29], mblend, mshuff); \
	}while(0)

/* X2 variant */
#define RINJDAEL_COMPENSATE_SR_x2(ml1, mr1, ml2, mr2, tmpl1, tmpr1, tmpl2, tmpr2, mblend, mshuff) \
	do{\
		/* This compensates for the AES shiftrows to */	\
		/* conform to Rijdael one */              	\
		tmpl1 = _mm_blendv_epi8(ml1, mr1, mblend); \
		tmpl2 = _mm_blendv_epi8(ml2, mr2, mblend); \
		tmpr1 = _mm_blendv_epi8(mr1, ml1, mblend); \
		tmpr2 = _mm_blendv_epi8(mr2, ml2, mblend); \
		tmpl1 = _mm_shuffle_epi8(tmpl1, mshuff); \
		tmpl2 = _mm_shuffle_epi8(tmpl2, mshuff); \
		tmpr1 = _mm_shuffle_epi8(tmpr1, mshuff); \
		tmpr2 = _mm_shuffle_epi8(tmpr2, mshuff); \
	}while(0)

#define ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, kl1, kl2, kr1, kr2, mb, ms) \
	do{\
		__m128i tmpl1, tmpr1, tmpl2, tmpr2;				\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR_x2(ml1, mr1, ml2, mr2, tmpl1, tmpr1, tmpl2, tmpr2, mblend, mshuff); \
		ml1 = _mm_aesenc_si128	(tmpl1, kl1); 				\
		ml2 = _mm_aesenc_si128	(tmpl2, kl2); 				\
		mr1 = _mm_aesenc_si128	(tmpr1, kr1); 				\
		mr2 = _mm_aesenc_si128	(tmpr2, kr2); 				\
	}while(0)

#define ENC_256_BLOCK_LAST_x2(ml1, mr1, ml2, mr2, kl1, kl2, kr1, kr2, mb, ms) \
	do{\
		__m128i tmpl1, tmpr1, tmpl2, tmpr2;				\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR_x2(ml1, mr1, ml2, mr2, tmpl1, tmpr1, tmpl2, tmpr2, mblend, mshuff); \
		ml1 = _mm_aesenclast_si128	(tmpl1, kl1); 			\
		ml2 = _mm_aesenclast_si128	(tmpl2, kl2); 			\
		mr1 = _mm_aesenclast_si128	(tmpr1, kr1); 			\
		mr2 = _mm_aesenclast_si128	(tmpr2, kr2); 			\
	}while(0)

#define DO_ENC_BLOCK_256_256_x2(ml1, mr1, ml2, mr2, k1, k2) \
	do{\
		/* ShiftRows compensation constants */		\
		const __m128i mblend = _mm_set_epi32(0x80000000, 0x80800000, 0x80800000, 0x80808000); \
		const __m128i mshuff = _mm_set_epi32(0x03020d0c, 0x0f0e0908, 0x0b0a0504, 0x07060100); \
		/**/						\
		ml1 = _mm_xor_si128       (ml1, k1[ 0]); 		\
		ml2 = _mm_xor_si128       (ml2, k2[ 0]); 		\
		mr1 = _mm_xor_si128       (mr1, k1[ 1]); 		\
		mr2 = _mm_xor_si128       (mr2, k2[ 1]); 		\
		/**/						\
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[2], k2[2], k1[3], k2[3], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[4], k2[4], k1[5], k2[5], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[6], k2[6], k1[7], k2[7], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[8], k2[8], k1[9], k2[9], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[10], k2[10], k1[11], k2[11], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[12], k2[12], k1[13], k2[13], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[14], k2[14], k1[15], k2[15], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[16], k2[16], k1[17], k2[17], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[18], k2[18], k1[19], k2[19], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[20], k2[20], k1[21], k2[21], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[22], k2[22], k1[23], k2[23], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[24], k2[24], k1[25], k2[25], mblend, mshuff); \
		ENC_256_BLOCK_x2(ml1, mr1, ml2, mr2, k1[26], k2[26], k1[27], k2[27], mblend, mshuff); \
		/**/						\
		ENC_256_BLOCK_LAST_x2(ml1, mr1, ml2, mr2, k1[28], k2[28], k1[29], k2[29], mblend, mshuff); \
	}while(0)

/* X4 variant */
#define RINJDAEL_COMPENSATE_SR_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, tmpl1, tmpr1, tmpl2, tmpr2, tmpl3, tmpr3, tmpl4, tmpr4, mblend, mshuff) \
	do{\
		/* This compensates for the AES shiftrows to */	\
		/* conform to Rijdael one */              	\
		tmpl1 = _mm_blendv_epi8(ml1, mr1, mblend); \
		tmpl2 = _mm_blendv_epi8(ml2, mr2, mblend); \
		tmpl3 = _mm_blendv_epi8(ml3, mr3, mblend); \
		tmpl4 = _mm_blendv_epi8(ml4, mr4, mblend); \
		tmpr1 = _mm_blendv_epi8(mr1, ml1, mblend); \
		tmpr2 = _mm_blendv_epi8(mr2, ml2, mblend); \
		tmpr3 = _mm_blendv_epi8(mr3, ml3, mblend); \
		tmpr4 = _mm_blendv_epi8(mr4, ml4, mblend); \
		tmpl1 = _mm_shuffle_epi8(tmpl1, mshuff); \
		tmpl2 = _mm_shuffle_epi8(tmpl2, mshuff); \
		tmpl3 = _mm_shuffle_epi8(tmpl3, mshuff); \
		tmpl4 = _mm_shuffle_epi8(tmpl4, mshuff); \
		tmpr1 = _mm_shuffle_epi8(tmpr1, mshuff); \
		tmpr2 = _mm_shuffle_epi8(tmpr2, mshuff); \
		tmpr3 = _mm_shuffle_epi8(tmpr3, mshuff); \
		tmpr4 = _mm_shuffle_epi8(tmpr4, mshuff); \
	}while(0)

#define ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, kl1, kl2, kl3, kl4, kr1, kr2, kr3, kr4, mb, ms) \
	do{\
		__m128i tmpl1, tmpr1, tmpl2, tmpr2, tmpl3, tmpr3, tmpl4, tmpr4;	\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, tmpl1, tmpr1, tmpl2, tmpr2, tmpl3, tmpr3, tmpl4, tmpr4, mblend, mshuff); \
		ml1 = _mm_aesenc_si128	(tmpl1, kl1); 				\
		ml2 = _mm_aesenc_si128	(tmpl2, kl2); 				\
		mr1 = _mm_aesenc_si128	(tmpr1, kr1); 				\
		mr2 = _mm_aesenc_si128	(tmpr2, kr2); 				\
		ml3 = _mm_aesenc_si128	(tmpl3, kl3); 				\
		ml4 = _mm_aesenc_si128	(tmpl4, kl4); 				\
		mr3 = _mm_aesenc_si128	(tmpr3, kr3); 				\
		mr4 = _mm_aesenc_si128	(tmpr4, kr4); 				\
	}while(0)

#define ENC_256_BLOCK_LAST_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, kl1, kl2, kl3, kl4, kr1, kr2, kr3, kr4, mb, ms) \
	do{\
		__m128i tmpl1, tmpr1, tmpl2, tmpr2, tmpl3, tmpr3, tmpl4, tmpr4;	\
		/* Compensate for the shift rows */				\
		RINJDAEL_COMPENSATE_SR_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, tmpl1, tmpr1, tmpl2, tmpr2, tmpl3, tmpr3, tmpl4, tmpr4, mblend, mshuff); \
		ml1 = _mm_aesenclast_si128	(tmpl1, kl1); 				\
		ml2 = _mm_aesenclast_si128	(tmpl2, kl2); 				\
		mr1 = _mm_aesenclast_si128	(tmpr1, kr1); 				\
		mr2 = _mm_aesenclast_si128	(tmpr2, kr2); 				\
		ml3 = _mm_aesenclast_si128	(tmpl3, kl3); 				\
		ml4 = _mm_aesenclast_si128	(tmpl4, kl4); 				\
		mr3 = _mm_aesenclast_si128	(tmpr3, kr3); 				\
		mr4 = _mm_aesenclast_si128	(tmpr4, kr4); 				\
	}while(0)

#define DO_ENC_BLOCK_256_256_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1, k2, k3, k4) \
	do{\
		/* ShiftRows compensation constants */		\
		const __m128i mblend = _mm_set_epi32(0x80000000, 0x80800000, 0x80800000, 0x80808000); \
		const __m128i mshuff = _mm_set_epi32(0x03020d0c, 0x0f0e0908, 0x0b0a0504, 0x07060100); \
		/**/						\
		ml1 = _mm_xor_si128       (ml1, k1[ 0]); 		\
		ml2 = _mm_xor_si128       (ml2, k2[ 0]); 		\
		mr1 = _mm_xor_si128       (mr1, k1[ 1]); 		\
		mr2 = _mm_xor_si128       (mr2, k2[ 1]); 		\
		ml3 = _mm_xor_si128       (ml3, k3[ 0]); 		\
		ml4 = _mm_xor_si128       (ml4, k4[ 0]); 		\
		mr3 = _mm_xor_si128       (mr3, k3[ 1]); 		\
		mr4 = _mm_xor_si128       (mr4, k4[ 1]); 		\
		/**/						\
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[2], k2[2], k3[2], k4[2], k1[3], k2[3], k3[3], k4[3], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[4], k2[4], k3[4], k4[4], k1[5], k2[5], k3[5], k4[5], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[6], k2[6], k3[6], k4[6], k1[7], k2[7], k3[7], k4[7], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[8], k2[8], k3[8], k4[8], k1[9], k2[9], k3[9], k4[9], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[10], k2[10], k3[10], k4[10], k1[11], k2[11], k3[11], k4[11], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[12], k2[12], k3[12], k4[12], k1[13], k2[13], k3[13], k4[13], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[14], k2[14], k3[14], k4[14], k1[15], k2[15], k3[15], k4[15], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[16], k2[16], k3[16], k4[16], k1[17], k2[17], k3[17], k4[17], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[18], k2[18], k3[18], k4[18], k1[19], k2[19], k3[19], k4[19], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[20], k2[20], k3[20], k4[20], k1[21], k2[21], k3[21], k4[21], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[22], k2[22], k3[22], k4[22], k1[23], k2[23], k3[23], k4[23], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[24], k2[24], k3[24], k4[24], k1[25], k2[25], k3[25], k4[25], mblend, mshuff); \
		ENC_256_BLOCK_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[26], k2[26], k3[26], k4[26], k1[27], k2[27], k3[27], k4[27], mblend, mshuff); \
		/**/						\
		ENC_256_BLOCK_LAST_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, k1[28], k2[28], k3[28], k4[28], k1[29], k2[29], k3[29], k4[29], mblend, mshuff); \
	}while(0)

#define AES_128_key_exp(k, rcon) aes_128_key_expansion(k, _mm_aeskeygenassist_si128(k, rcon))

static inline __m128i aes_128_key_expansion(__m128i key, __m128i keygened){
	keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	return _mm_xor_si128(key, keygened);
}

static inline void KEY_256_ASSIST_1(__m128i* temp1, __m128i * temp2)
{
	__m128i temp4;
	*temp2 = _mm_shuffle_epi32(*temp2, 0xff);
	temp4 = _mm_slli_si128 (*temp1, 0x4);
	*temp1 = _mm_xor_si128 (*temp1, temp4);
	temp4 = _mm_slli_si128 (temp4, 0x4);
	*temp1 = _mm_xor_si128 (*temp1, temp4);
	temp4 = _mm_slli_si128 (temp4, 0x4);
	*temp1 = _mm_xor_si128 (*temp1, temp4);
	*temp1 = _mm_xor_si128 (*temp1, *temp2);
}
static inline void KEY_256_ASSIST_2(__m128i* temp1, __m128i * temp3)
{
	__m128i temp2,temp4;
	temp4 = _mm_aeskeygenassist_si128 (*temp1, 0x0);
	temp2 = _mm_shuffle_epi32(temp4, 0xaa);
	temp4 = _mm_slli_si128 (*temp3, 0x4);
	*temp3 = _mm_xor_si128 (*temp3, temp4);
	temp4 = _mm_slli_si128 (temp4, 0x4);
	*temp3 = _mm_xor_si128 (*temp3, temp4);
	temp4 = _mm_slli_si128 (temp4, 0x4);
	*temp3 = _mm_xor_si128 (*temp3, temp4);
	*temp3 = _mm_xor_si128 (*temp3, temp2);
} 

/* ==== Public APIs ===== */

int aes128_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t enc_key[16])
{
	__m128i *Key_Schedule = (__m128i*)ctx->rk;
	/**/
	ctx->rtype = AES128;
	/**/
	Key_Schedule[0] = _mm_loadu_si128((const __m128i*) enc_key);
	Key_Schedule[1]  = AES_128_key_exp(Key_Schedule[0], 0x01);
	Key_Schedule[2]  = AES_128_key_exp(Key_Schedule[1], 0x02);
	Key_Schedule[3]  = AES_128_key_exp(Key_Schedule[2], 0x04);
	Key_Schedule[4]  = AES_128_key_exp(Key_Schedule[3], 0x08);
	Key_Schedule[5]  = AES_128_key_exp(Key_Schedule[4], 0x10);
	Key_Schedule[6]  = AES_128_key_exp(Key_Schedule[5], 0x20);
	Key_Schedule[7]  = AES_128_key_exp(Key_Schedule[6], 0x40);
	Key_Schedule[8]  = AES_128_key_exp(Key_Schedule[7], 0x80);
	Key_Schedule[9]  = AES_128_key_exp(Key_Schedule[8], 0x1B);
	Key_Schedule[10] = AES_128_key_exp(Key_Schedule[9], 0x36);

	return 0;
}


int aes256_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t enc_key[32])
{
	__m128i temp1, temp2, temp3;
	__m128i *Key_Schedule = (__m128i*)ctx->rk;
	/**/
	ctx->rtype = AES256;
	/**/
	temp1 = _mm_loadu_si128((__m128i*)enc_key);
	temp3 = _mm_loadu_si128((__m128i*)(enc_key+16));
	Key_Schedule[0] = temp1;
	Key_Schedule[1] = temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x01);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[2]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[3]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x02);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[4]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[5]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x04);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[6]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[7]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x08);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[8]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[9]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x10);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[10]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[11]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x20);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[12]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[13]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x40);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[14]=temp1;

	return 0;
}
 
int rijndael256_aes_ni_setkey_enc(rijndael_aes_ni_ctx *ctx, const uint8_t enc_key[32])
{
	__m128i temp1, temp2, temp3;
	__m128i *Key_Schedule = (__m128i*)ctx->rk;
	/**/
	ctx->rtype = RIJNDAEL_256_256;
	/**/
	temp1 = _mm_loadu_si128((__m128i*)enc_key);
	temp3 = _mm_loadu_si128((__m128i*)(enc_key+16));
	Key_Schedule[0] = temp1;
	Key_Schedule[1] = temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x01);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[2]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[3]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x02);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[4]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[5]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x04);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[6]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[7]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x08);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[8]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[9]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x10);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[10]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[11]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x20);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[12]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[13]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x40);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[14]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[15]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x80);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[16]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[17]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x1b);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[18]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[19]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x36);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[20]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[21]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x6c);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[22]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[23]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0xd8);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[24]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[25]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0xab);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[26]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[27]=temp3;
	temp2 = _mm_aeskeygenassist_si128 (temp3,0x4d);
	KEY_256_ASSIST_1(&temp1, &temp2);
	Key_Schedule[28]=temp1;
	KEY_256_ASSIST_2(&temp1, &temp3);
	Key_Schedule[29]=temp3;

	return 0;

}

int aes128_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t plainText[16], uint8_t cipherText[16]){
	__m128i m;
	int ret = -1;

	if((ctx == NULL) || (ctx->rtype != AES128)){
		goto err;
	}

	m = _mm_loadu_si128((__m128i *) plainText);

	DO_ENC_BLOCK_128_128(m, ctx->rk);

	_mm_storeu_si128((__m128i *) cipherText, m);

	ret = 0;
err:
	return ret;
}

int aes128_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]){
	__m128i m1, m2;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != AES128)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES128)){
		goto err;
	}

	m1 = _mm_loadu_si128((__m128i *) plainText1);
	m2 = _mm_loadu_si128((__m128i *) plainText2);

	DO_ENC_BLOCK_128_128_x2(m1, m2, ctx1->rk, ctx2->rk);

	_mm_storeu_si128((__m128i *) cipherText1, m1);
	_mm_storeu_si128((__m128i *) cipherText2, m2);

	ret = 0;
err:
	return ret;
}

int aes128_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
		const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
		uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]){
	__m128i m1, m2, m3, m4;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != AES128)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES128)){
		goto err;
	}
	if((ctx3 == NULL) || (ctx3->rtype != AES128)){
		goto err;
	}
	if((ctx4 == NULL) || (ctx4->rtype != AES128)){
		goto err;
	}

	m1 = _mm_loadu_si128((__m128i *) plainText1);
	m2 = _mm_loadu_si128((__m128i *) plainText2);
	m3 = _mm_loadu_si128((__m128i *) plainText3);
	m4 = _mm_loadu_si128((__m128i *) plainText4);

	DO_ENC_BLOCK_128_128_x4(m1, m2, m3, m4, ctx1->rk, ctx2->rk, ctx3->rk, ctx4->rk);

	_mm_storeu_si128((__m128i *) cipherText1, m1);
	_mm_storeu_si128((__m128i *) cipherText2, m2);
	_mm_storeu_si128((__m128i *) cipherText3, m3);
	_mm_storeu_si128((__m128i *) cipherText4, m4);

	ret = 0;
err:
	return ret;
}

int aes256_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t plainText[16], uint8_t cipherText[16]){
	__m128i m;
	int ret = -1;

	if((ctx == NULL) || (ctx->rtype != AES256)){
		goto err;
	}

	m = _mm_loadu_si128((__m128i *) plainText);

	DO_ENC_BLOCK_128_256(m, ctx->rk);

	_mm_storeu_si128((__m128i *) cipherText, m);

	ret = 0;
err:
	return ret;
}

int aes256_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16]){
	__m128i m1, m2;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != AES256)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES256)){
		goto err;
	}

	m1 = _mm_loadu_si128((__m128i *) plainText1);
	m2 = _mm_loadu_si128((__m128i *) plainText2);

	DO_ENC_BLOCK_128_256_x2(m1, m2, ctx1->rk, ctx2->rk);

	_mm_storeu_si128((__m128i *) cipherText1, m1);
	_mm_storeu_si128((__m128i *) cipherText2, m2);

	ret = 0;
err:
	return ret;
}

int aes256_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
		const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
		uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16]){
	__m128i m1, m2, m3, m4;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != AES256)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != AES256)){
		goto err;
	}
	if((ctx3 == NULL) || (ctx3->rtype != AES256)){
		goto err;
	}
	if((ctx4 == NULL) || (ctx4->rtype != AES256)){
		goto err;
	}

	m1 = _mm_loadu_si128((__m128i *) plainText1);
	m2 = _mm_loadu_si128((__m128i *) plainText2);
	m3 = _mm_loadu_si128((__m128i *) plainText3);
	m4 = _mm_loadu_si128((__m128i *) plainText4);

	DO_ENC_BLOCK_128_256_x4(m1, m2, m3, m4, ctx1->rk, ctx2->rk, ctx3->rk, ctx4->rk);

	_mm_storeu_si128((__m128i *) cipherText1, m1);
	_mm_storeu_si128((__m128i *) cipherText2, m2);
	_mm_storeu_si128((__m128i *) cipherText3, m3);
	_mm_storeu_si128((__m128i *) cipherText4, m4);

	ret = 0;
err:
	return ret;
}



int rijndael256_aes_ni_enc(const rijndael_aes_ni_ctx *ctx, const uint8_t plainText[32], uint8_t cipherText[32]){
	__m128i ml, mr;
	int ret = -1;

	if((ctx == NULL) || (ctx->rtype != RIJNDAEL_256_256)){
		goto err;
	}

	ml = _mm_loadu_si128((__m128i *) &plainText[0]);
	mr = _mm_loadu_si128((__m128i *) &plainText[16]);

	DO_ENC_BLOCK_256_256(ml, mr, ctx->rk);

	_mm_storeu_si128((__m128i *) &cipherText[0], ml);
	_mm_storeu_si128((__m128i *) &cipherText[16], mr);

	ret = 0;
err:
	return ret;
}

int rijndael256_aes_ni_enc_x2(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2,
			const uint8_t plainText1[32], const uint8_t plainText2[32],
			uint8_t cipherText1[32], uint8_t cipherText2[32]){
	__m128i ml1, mr1, ml2, mr2;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != RIJNDAEL_256_256)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != RIJNDAEL_256_256)){
		goto err;
	}

	ml1 = _mm_loadu_si128((__m128i *) &plainText1[0]);
	mr1 = _mm_loadu_si128((__m128i *) &plainText1[16]);
	ml2 = _mm_loadu_si128((__m128i *) &plainText2[0]);
	mr2 = _mm_loadu_si128((__m128i *) &plainText2[16]);

	DO_ENC_BLOCK_256_256_x2(ml1, mr1, ml2, mr2, ctx1->rk, ctx2->rk);

	_mm_storeu_si128((__m128i *) &cipherText1[0], ml1);
	_mm_storeu_si128((__m128i *) &cipherText1[16], mr1);
	_mm_storeu_si128((__m128i *) &cipherText2[0], ml2);
	_mm_storeu_si128((__m128i *) &cipherText2[16], mr2);

	ret = 0;
err:
	return ret;
}

int rijndael256_aes_ni_enc_x4(const rijndael_aes_ni_ctx *ctx1, const rijndael_aes_ni_ctx *ctx2, const rijndael_aes_ni_ctx *ctx3, const rijndael_aes_ni_ctx *ctx4,
		const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
		uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32]){
	__m128i ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4;
	int ret = -1;

	if((ctx1 == NULL) || (ctx1->rtype != RIJNDAEL_256_256)){
		goto err;
	}
	if((ctx2 == NULL) || (ctx2->rtype != RIJNDAEL_256_256)){
		goto err;
	}
	if((ctx3 == NULL) || (ctx3->rtype != RIJNDAEL_256_256)){
		goto err;
	}
	if((ctx4 == NULL) || (ctx4->rtype != RIJNDAEL_256_256)){
		goto err;
	}

	ml1 = _mm_loadu_si128((__m128i *) &plainText1[0]);
	mr1 = _mm_loadu_si128((__m128i *) &plainText1[16]);
	ml2 = _mm_loadu_si128((__m128i *) &plainText2[0]);
	mr2 = _mm_loadu_si128((__m128i *) &plainText2[16]);
	ml3 = _mm_loadu_si128((__m128i *) &plainText3[0]);
	mr3 = _mm_loadu_si128((__m128i *) &plainText3[16]);
	ml4 = _mm_loadu_si128((__m128i *) &plainText4[0]);
	mr4 = _mm_loadu_si128((__m128i *) &plainText4[16]);

	DO_ENC_BLOCK_256_256_x4(ml1, mr1, ml2, mr2, ml3, mr3, ml4, mr4, ctx1->rk, ctx2->rk, ctx3->rk, ctx4->rk);

	_mm_storeu_si128((__m128i *) &cipherText1[0], ml1);
	_mm_storeu_si128((__m128i *) &cipherText1[16], mr1);
	_mm_storeu_si128((__m128i *) &cipherText2[0], ml2);
	_mm_storeu_si128((__m128i *) &cipherText2[16], mr2);
	_mm_storeu_si128((__m128i *) &cipherText3[0], ml3);
	_mm_storeu_si128((__m128i *) &cipherText3[16], mr3);
	_mm_storeu_si128((__m128i *) &cipherText4[0], ml4);
	_mm_storeu_si128((__m128i *) &cipherText4[16], mr4);

	ret = 0;
err:
	return ret;
}

#else /* !RIJNDAEL_AES_NI */
/*
 * Dummy definition to avoid the empty translation unit ISO C warning
 */
typedef int dummy;
#endif
