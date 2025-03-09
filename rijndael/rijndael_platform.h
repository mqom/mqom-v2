#ifndef __RIJDAEL_PLATFORM_H__
#define __RIJDAEL_PLATFORM_H__

/* Select the best Rijndael implementation depending on the platform if
 * not overloaded by the user */
#if !defined(RIJNDAEL_CONSTANT_TIME_REF) && !defined(RIJNDAEL_TABLE) && !defined(RIJNDAEL_AES_NI)
/* When AES-NI is present, select the optimized implementation */
#ifdef __AES__
#define RIJNDAEL_AES_NI
#else
/* When no AES-NI is detected, select the constant time reference implementation */
#define RIJNDAEL_CONSTANT_TIME_REF
#endif
#endif

#endif /* __RIJDAEL_PLATFORM_H__ */
