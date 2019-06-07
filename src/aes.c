#include "aes.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define AES_BLOCK_LEN 16
#define AES_KEY_LEN 16
#define AES_EXPANDED_KEY_LEN 44
#define AES_ROUNDS 9

#define BYTE_OF_WORD(word, byte_index) (((word) & (0xFF << ((byte_index) *8))) >> ((byte_index) *8))

typedef uint8_t state_t[4][4];

typedef struct aes_data_t
{
	state_t state;
	uint8_t key[AES_KEY_LEN];
	uint8_t (*blocks)[AES_BLOCK_LEN];
	size_t numBlocks;
	uint32_t roundKey[AES_EXPANDED_KEY_LEN];
} aes_data_t;

static const uint8_t sbox[16][16] = {
    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
    {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
    {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
    {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
    {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
    {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
    {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
    {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
    {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
    {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
    {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
    {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
    {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
    {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
    {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
    {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

static const uint8_t inverse_sbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, 0x7c, 0xe3, 0x39,
    0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2,
    0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76,
    0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc,
    0x5d, 0x65, 0xb6, 0x92, 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d,
    0x84, 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, 0xd0, 0x2c,
    0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, 0x3a, 0x91, 0x11, 0x41, 0x4f,
    0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62,
    0x0e, 0xaa, 0x18, 0xbe, 0x1b, 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd,
    0x5a, 0xf4, 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, 0x60,
    0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, 0xa0, 0xe0, 0x3b, 0x4d,
    0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6,
    0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

static const uint8_t rcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

static const uint8_t mix_con[4][4] = {{2, 3, 1, 1}, {1, 2, 3, 1}, {1, 1, 2, 3}, {3, 1, 1, 2}};

static void initData(aes_data_t * data, const char * message, const char * key);
static void freeData(aes_data_t * data);
static void expandKey(aes_data_t * data);
static void subBytes(aes_data_t * data);
static void invSubBytes(aes_data_t * data);
static void shiftRows(aes_data_t * data);
static void invShitfRows(aes_data_t * data);
static void mixColumns(aes_data_t * data);
static void invMixColumns(aes_data_t * data);
static void addRoundKey(aes_data_t * data, uint8_t round);

aes_cipher_t encrypt(const char * message, const char * key)
{
	aes_cipher_t result;
	memset(&result, 0, sizeof(result));

	aes_data_t data;
	uint8_t * cipher = 0;
	size_t cipherLen;

	if (message && key) {
		// Initialize data struct
		initData(&data, message, key);

		// Allocate memory for cipher text
		cipherLen = sizeof(uint8_t) * data.numBlocks * AES_BLOCK_LEN;
		cipher = (uint8_t *) malloc(cipherLen);
		memset(cipher, 0, cipherLen);

		// Expand key to round key schedule
		expandKey(&data);

		for (int i = 0; i < data.numBlocks; ++i) {
			// Populate state with message block
			memcpy(data.state, data.blocks[i], sizeof(data.state));

			// First round
			addRoundKey(&data, 0);

			// Rounds 1 to N-1
			for (uint8_t j = 1; j <= AES_ROUNDS; ++j) {
				subBytes(&data);
				shiftRows(&data);
				mixColumns(&data);
				addRoundKey(&data, j);
			}

			// Final round
			subBytes(&data);
			shiftRows(&data);
			addRoundKey(&data, 10);

			// Append data to ciphertext
			memcpy(&cipher[AES_BLOCK_LEN * i], data.state, sizeof(data.state));
		}

		freeData(&data);
		result.cipher = cipher;
		result.cipherLen = cipherLen;
	}

	return result;
}

aes_cipher_t decrypt(aes_cipher_t cipher, const char * key)
{
	aes_cipher_t result;
	memset(&result, 0, sizeof(result));

	// aes_data_t data;

	// // First round
	// addRoundKey(&data);

	// // Rounds 1 to N-1
	// for (int i = 0; i < AES_ROUNDS; ++i) {
	// 	invShitfRows(&data);
	// 	invSubBytes(&data);
	// 	addRoundKey(&data);
	// 	invMixColumns(&data);
	// }

	// // Final round
	// invShitfRows(&data);
	// invSubBytes(&data);
	// addRoundKey(&data);

	return result;
}

static void initData(aes_data_t * data, const char * message, const char * key)
{
	size_t size;

	if (data && message && key) {
		// Clear round key schedule
		memset(data->roundKey, 0, AES_EXPANDED_KEY_LEN);

		// Allocate memory for each block
		data->numBlocks = (size_t) ceil(strlen(message) / (double) AES_BLOCK_LEN);
		size = sizeof(uint8_t *) * data->numBlocks * AES_BLOCK_LEN;
		data->blocks = (uint8_t(*)[AES_BLOCK_LEN]) malloc(size);
		memset(data->blocks, 0, size);

		// Assign each block according to the input message
		strncpy((char *) data->blocks, message, strlen(message));

		// Assign key
		memcpy(data->key, key, AES_KEY_LEN);
	}
}

static void freeData(aes_data_t * data)
{
	if (data) {
		free(data->blocks);
	}
}

static void expandKey(aes_data_t * data)
{
	uint32_t w[4];
	uint32_t x, y, z;
	uint8_t byte;

	for (int i = 0; i < 4; ++i) {
		// On the first round, take data from the key
		w[i] = data->key[i] << 24 | data->key[i + 4] << 16 | data->key[i + 8] << 8 | data->key[i + 12];
	}
	memcpy(data->roundKey, w, sizeof(w));

	// Apply g function to generate the rest of the words
	for (int i = 0; i < 10; ++i) {
		// Rotate previous word
		x = ((w[3] << 8) | ((w[3] & 0xFF000000) >> 24));

		// Substitute each byte in the word
		y = 0;
		for (int j = 0; j < 4; ++j) {
			byte = BYTE_OF_WORD(x, j);
			byte = sbox[(byte & 0xF0) >> 4][byte & 0x0F];
			y |= byte << (j * 8);
		}

		// XOR byte with the round constant
		z = (rcon[i] << 24) ^ y;

		w[0] ^= z;     // w0 ^ z1
		w[1] ^= w[0];  // w4 ^ w1
		w[2] ^= w[1];  // w5 ^ w2
		w[3] ^= w[2];  // w6 ^ w3
		memcpy(&data->roundKey[(i + 1) * 4], w, sizeof(w));
	}
}

static void subBytes(aes_data_t * data)
{
	uint8_t byte;

	if (data) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				byte = data->state[i][j];
				byte = sbox[(byte & 0xF0) >> 4][byte & 0x0F];
				data->state[i][j] = byte;
			}
		}
	}
}

static void invSubBytes(aes_data_t * data)
{
	if (data) {
	}
}

static void shiftRows(aes_data_t * data)
{
	uint8_t temp;

	if (data) {
		for (int i = 1; i <= 4; ++i) {
			for (int j = 0; j < i; ++j) {
				temp = data->state[i][0];
				data->state[i][0] = data->state[i][1];
				data->state[i][1] = data->state[i][2];
				data->state[i][2] = data->state[i][3];
				data->state[i][3] = temp;
			}
		}
	}
}

static void invShitfRows(aes_data_t * data)
{
	uint8_t temp;

	if (data) {
		for (int i = 1; i <= 4; ++i) {
			for (int j = 0; j < i; ++j) {
				temp = data->state[i][3];
				data->state[i][3] = data->state[i][2];
				data->state[i][2] = data->state[i][1];
				data->state[i][1] = data->state[i][0];
				data->state[i][0] = temp;
			}
		}
	}
}

static void mixColumns(aes_data_t * data)
{
	if (data) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				data->state[j][i] =
				    ((data->state[0][i] * mix_con[j][0]) ^ (data->state[1][i] * mix_con[j][1]) ^
				     (data->state[2][i] * mix_con[j][2]) ^ (data->state[3][i] * mix_con[j][3]));
			}
		}
	}
}

static void invMixColumns(aes_data_t * data)
{}

static void addRoundKey(aes_data_t * data, uint8_t round)
{
	if (data) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				data->state[i][j] ^= BYTE_OF_WORD(data->roundKey[round * 4 + j], 3 - i);
			}
		}
	}
}
