#ifndef AES_IMPL_H
#define AES_IMPL_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>

typedef struct aes_cipher_t
{
	uint8_t * cipher;
	size_t cipherLen;

} aes_cipher_t;

extern aes_cipher_t encrypt(const char * message, const char * key);
extern char * decrypt(aes_cipher_t cipher, const char * key);

#endif
