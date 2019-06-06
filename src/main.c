#include <stdio.h>

#include "aes.h"

int main(int argc, char * argv[])
{
	for (int i = 1; i < argc; ++i) {
		printf("%2d. %s\n", i, argv[i]);
	}

	encrypt("this is a test, lmao", "this is a key");

	return 0;
}
