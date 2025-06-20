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
    printf("===== SCHEME CONFIG =====\n");
    printf("[API] Algo Name: " CRYPTO_ALGNAME "\n");
    printf("[API] Algo Version: " CRYPTO_VERSION "\n");
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
#if defined(__AVX512BW__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__)
    printf(" AVX512BW AVX512F AVX512VL AVX512VPOPCNTDQ");
#endif
#ifdef __GFNI__
    printf(" GFNI");
#endif
#ifdef __AES__
    printf(" AES-NI");
#endif
    printf("\n");


    printf("Rijndael implementation: %s\n", rijndael_conf);
    printf("Fields implementation: %s\n", fields_conf);

#ifndef NDEBUG
    printf("Debug: On\n");
#else
    printf("Debug: Off\n");
#endif
}

#endif /* MQOM_BENCH_UTILS_H */
