#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"

int main(int argc, char * argv[])
{
	aes_cipher_t cipher;

	// for (int i = 1; i < argc; ++i) {
	// 	printf("%2d. %s\n", i, argv[i]);
	// }

	// cipher = encrypt("this is a test, lmao", "\x0f\x47\x0c\xaf\x15\xd9\xb7\x7f\x71\xe8\xad\x67\xc9\x59\xd6\x98");
	cipher = encrypt("AAAAAAAAAAAAAAAAA", "0123456789ABCDEF");

	if (cipher.cipherLen) {
		for (int i = 0; i < cipher.cipherLen; ++i) {
			printf("%2x ", cipher.cipher[i]);
		}
		printf("\n");
		free(cipher.cipher);
	}

	return 0;
}
