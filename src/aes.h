#ifndef AES_IMPL_H
#define AES_IMPL_H

#include <stdint.h>

extern uint8_t * encrypt(const char * message, const char * key);
extern uint8_t * decrypt(const char * cipher, const char * key);

#endif
