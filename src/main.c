/*
Authored by
        Jorge Hernandez
        Colton Morris
        Daniel Stotts
 */

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"

static void print_hex(const char * message, size_t len);

int main()
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// AES NIST example key & input
	const char * key = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c";
	const char * input = "\x32\x43\xf6\xa8\x88\x5a\x30\x8d\x31\x31\x98\xa2\xe0\x37\x07\x34";

	aes_cipher_t cipher;
	char * message = 0;

	// Print input before processing
	print_hex(input, strlen(input));

	cipher = encrypt(input, key);
	if (cipher.cipherLen) {
		// Print encrypted message
		print_hex((char *) cipher.cipher, cipher.cipherLen);

		message = decrypt(cipher, key);
		if (message) {
			// Print decrypted message
			print_hex(message, strlen(message));
			free(message);
		}
		free(cipher.cipher);
	}

	return 0;
}

static void print_hex(const char * message, size_t len)
{
	if (message) {
		for (int i = 0; i < len; ++i) {
			printf("%02x ", (unsigned char) message[i]);
		}
		printf("\n");
	}
}
