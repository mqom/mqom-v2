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
	uint8_t sm[MLEN + CRYPTO_BYTES];
	unsigned long long smlen;

	// Generate the keys
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
	uint8_t m2[MLEN] = {0};
	unsigned long long m2len;

	// Read the signature
	FILE *fptr;
	fptr = fopen("bench-sig.txt", "r");
	if (fptr == NULL) {
		printf("Failure: failed to open file 'bench-sig.txt'\n");
		return 0;
	}
	smlen = 0;
	int val = 1;
	while (val > 0) {
		val = fscanf(fptr, "%hhu ", &sm[smlen]);
		smlen++;
	}
	smlen--;
	fclose(fptr);

	// Verify/Open the signature
	int ret = crypto_sign_open(m2, &m2len, sm, smlen, pk);
	if (ret) {
		printf("Failure: crypto_sign_open\n");
		return 0;
	}

	// Test of correction of the primitives
	if (m2len != MLEN) {
		printf("Failure: message size does not match\n");
		return 0;
	}
	for (int h = 0; h < MLEN; h++)
		if (m[h] != m2[h]) {
			printf("Failure: message does not match (char %d)\n", h);
			return 0;
		}

	// Display Infos
	printf("===== SUMMARY =====\n");
	printf("Everything is fine.\n");

	return 0;
}
