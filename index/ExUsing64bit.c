#define _FILE_OFFSET_BITS	64
#include <stdio.h>

int main(void)
{
	int buf[BUFSIZ];

	//fseeko(file, (long)..);

	//off_t  long long

	//(long)ftell(..);

	//(off_t)ftello(..);

	printf("%d\n", BUFSIZ);
	FILE *file = fopen("bigfile", "w+");
	while (1) {
		fwrite(buf, 1, BUFSIZ, file);
	}
}

