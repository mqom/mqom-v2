#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "api.h"
#include "utils.h"

int randombytes(unsigned char* x, unsigned long long xlen) {
	for (unsigned long long j = 0; j < xlen; j++) {
		x[j] = (uint8_t) rand();
	}
	return 0;
}

#define MLEN 32

int main(void) {
	srand((unsigned int) time(NULL));

	print_configuration();

	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];

	// Read the key pair
	FILE *fptr_keys;
	fptr_keys = fopen("bench-sig-keys.txt", "r");
	if (fptr_keys == NULL) {
		printf("Failure: failed to open file 'bench-sig-keys.txt'\n");
		return 0;
	}
	for (unsigned int j = 0; j < CRYPTO_PUBLICKEYBYTES; j++) {
		fscanf(fptr_keys, "%hhu ", &pk[j]);
	}
	for (unsigned int j = 0; j < CRYPTO_SECRETKEYBYTES; j++) {
		fscanf(fptr_keys, "%hhu ", &sk[j]);
	}
	fclose(fptr_keys);

	// Select the message
	uint8_t m[MLEN] = {1, 2, 3, 4};

	// Sign the message
	uint8_t sm[MLEN + CRYPTO_BYTES];
	unsigned long long smlen;
	int ret = crypto_sign(sm, &smlen, m, MLEN, sk);
	if (ret) {
		printf("Failure: crypto_sign\n");
		return 0;
	}

	// Save signature
	FILE *fptr;
	fptr = fopen("bench-sig.txt", "w");
	if (fptr == NULL) {
		printf("Failure: failed to open file\n");
		return 0;
	}
	for (unsigned int j = 0; j < smlen; j++) {
		fprintf(fptr, "%d ", sm[j]);
	}
	fclose(fptr);

	// Display Infos
	printf("===== SUMMARY =====\n");
	printf("Signature saved in bench-sig.txt\n");

	return 0;
}
