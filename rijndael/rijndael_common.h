#ifndef __RIJNDAEL_COMMON_H__
#define __RIJNDAEL_COMMON_H__

/* Common includes for all the implementation variants of
 * Rijndael */
#include <stdint.h>
#include <string.h>

/* Namespacing with the appropriate prefix */
#ifndef MQOM_NAMESPACE
#ifdef APPLY_NAMESPACE
#ifndef concat2
#define _concat2(a, b) a ## b
#define concat2(a, b) _concat2(a, b)
#endif
#define MQOM_NAMESPACE(s) concat2(APPLY_NAMESPACE, s)
#else
#define MQOM_NAMESPACE(s) s
#endif
#endif

typedef enum {
    AES128 = 0, /* Actually Rijndael_128_128 */
    AES256 = 1, /* Actually Rijndael_128_256  */
    RIJNDAEL_256_256 = 2,
} rijndael_type;

#endif /* __RIJNDAEL_COMMON_H__ */
