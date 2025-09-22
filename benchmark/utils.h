#ifndef MQOM_BENCH_UTILS_H
#define MQOM_BENCH_UTILS_H

#include <stdio.h>
#include "api.h"
#include "rijndael/rijndael.h"
#include "fields.h"

static inline int get_number_of_tests(int argc, char *argv[], int default_value) {
    int nb_tests = default_value;
    if(argc == 2) {
        if( sscanf(argv[1], "%d", &nb_tests) != 1) {
            printf("Integer awaited.\n");
            return -1;
        } else if( nb_tests <= 0 ) {
            printf("Need to positive integer.\n");
            return -1;
        }
    }
    return nb_tests;
}

static inline void print_configuration(void) {
    printf("===== SCHEME CONFIG =====\r\n");
    printf("[API] Algo Name: " CRYPTO_ALGNAME "\r\n");
    printf("[API] Algo Version: " CRYPTO_VERSION "\r\n");
    printf("Instruction Sets:");
#ifdef __SSE__
    printf(" SSE");
#endif
#ifdef __AVX__
    printf(" AVX");
#endif
#ifdef __AVX2__
    printf(" AVX2");
#endif
#if defined(__AVX512BW__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__) && defined(__AVX512VBMI__)
    printf(" AVX512BW AVX512F AVX512VL AVX512VPOPCNTDQ AVX512VBMI");
#endif
#if defined(__GFNI__) && !defined(NO_GFNI)
    printf(" GFNI");
#endif
#ifdef __AES__
    printf(" AES-NI");
#endif
    printf("\r\n");

    printf("Configuration elements:\r\n");
#ifdef MEMORY_EFFICIENT_KEYGEN
    printf("  Keygen: memopt\r\n");
#else
    printf("  Keygen: default\r\n");
#endif
#ifdef MEMORY_EFFICIENT_PIOP
    printf("  PIOP: memopt\r\n");
#else
    printf("  PIOP: default\r\n");
#endif
#ifdef MEMORY_EFFICIENT_BLC
    printf("  BLC: memopt\r\n");
#ifdef BLC_INTERNAL_X4
    printf("    BLC_INTERNAL_X4 ON\r\n");
#endif
#ifdef BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE
    printf("    BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE %d\r\n", BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE);
#else
    printf("    BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE 1 (default)\r\n");
#endif
#ifdef GGMTREE_NB_ENC_CTX_IN_MEMORY
    printf("    GGMTREE_NB_ENC_CTX_IN_MEMORY %d\r\n", GGMTREE_NB_ENC_CTX_IN_MEMORY);
#else
    printf("    GGMTREE_NB_ENC_CTX_IN_MEMORY 1 (default)\r\n");
#endif
#else
    printf("  BLC: default\r\n");
#endif

#ifdef USE_PRG_CACHE
    printf("  PRG cache ON\r\n");
#else
    printf("  PRG cache OFF\r\n");
#endif
#if defined(USE_PIOP_CACHE) && !defined(MEMORY_EFFICIENT_PIOP)
    printf("  PIOP cache ON\r\n");
#else
    printf("  PIOP cache OFF\r\n");
#endif

    printf("  Rijndael implementation: %s\r\n", rijndael_conf);
    printf("  Fields implementation: %s\r\n", fields_conf);

#ifndef NDEBUG
    printf("Debug: On\r\n");
#else
    printf("Debug: Off\r\n");
#endif
}

#endif /* MQOM_BENCH_UTILS_H */
