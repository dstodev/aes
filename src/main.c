#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"

int main(int argc, char * argv[])
{
	const char * key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c";

	aes_cipher_t cipher;
	char * message = 0;

	// for (int i = 1; i < argc; ++i) {
	// 	printf("%2d. %s\n", i, argv[i]);
	// }

	// cipher = encrypt("this is a test, lmao", "\x0f\x47\x0c\xaf\x15\xd9\xb7\x7f\x71\xe8\xad\x67\xc9\x59\xd6\x98");
	// cipher = encrypt("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "0123456789ABCDEF");

	cipher = encrypt("\x32\x43\xf6\xa8\x88\x5a\x30\x8d\x31\x31\x98\xa2\xe0\x37\x07\x34", key);

	if (cipher.cipherLen) {
		for (int i = 0; i < cipher.cipherLen; ++i) {
			printf("%02x ", cipher.cipher[i]);
		}
		printf("\n");

		message = decrypt(cipher, key);
		if (message) {
			free(message);
		}

		free(cipher.cipher);
	}

	return 0;
}
