#include <stdio.h>

#include "aes.h"

int main(int argc, char * argv[])
{
	for (int i = 1; i < argc; ++i) {
		printf("%2d. %s\n", i, argv[i]);
	}

	encrypt("this is a test, lmao", "\x0f\x47\x0c\xaf\x15\xd9\xb7\x7f\x71\xe8\xad\x67\xc9\x59\xd6\x98");

	return 0;
}
