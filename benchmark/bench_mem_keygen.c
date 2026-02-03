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

int main(void) {
	srand((unsigned int) time(NULL));

	print_configuration();

	uint8_t pk[CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[CRYPTO_SECRETKEYBYTES];

	// Generate the key pair
	int ret = crypto_sign_keypair(pk, sk);
	if (ret) {
		printf("Failure: crypto_sign_keypair\n");
		return 0;
	}

	// Save keys
	FILE *fptr;
	fptr = fopen("bench-sig-keys.txt", "w");
	if (fptr == NULL) {
		printf("Failure: failed to open file\n");
		return 0;
	}
	for (unsigned int j = 0; j < CRYPTO_PUBLICKEYBYTES; j++) {
		fprintf(fptr, "%d ", pk[j]);
	}
	for (unsigned int j = 0; j < CRYPTO_SECRETKEYBYTES; j++) {
		fprintf(fptr, "%d ", sk[j]);
	}

	// Display Infos
	printf("===== SUMMARY =====\n");
	printf("Key pair saved in bench-sig-keys.txt\n");

	return 0;
}
