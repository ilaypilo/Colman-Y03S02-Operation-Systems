#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
	int fdin1, fdin2;   /* input file descriptor */
	int readBytes1, readBytes2; /* read bytes counter */
	int res; /* strcmp result */
	char buffer1[BUFFER_SIZE]; /* input (output) buffer */
	char buffer2[BUFFER_SIZE]; /* input (output) buffer */


	if (argc != 3) {
		printf("Usage: %s <1st file> <2nd file>\n", argv[0]);
		return -1;
	}

	fdin1 = open(argv[1], O_RDONLY);
	if (fdin1 < 0) /* means file open did not take place */
	{
		printf("error open 1st %s\n", argv[1]);
		return -1;
	}

	fdin2 = open(argv[2], O_RDONLY);
	if (fdin2 < 0) /* means file open did not take place */
	{
		printf("error open 2nd %s\n", argv[2]);
		return -1;
	}

	do
	{
		readBytes1 = read(fdin1, buffer1, BUFFER_SIZE);
		readBytes2 = read(fdin2, buffer2, BUFFER_SIZE);
		if (readBytes1 != readBytes2)
		{
			//printf("files are different 1\n");
			return 1;
		}

		// if we got here -> readBytes1 = readBytes2 
		// so we can use one of them as size
		if (0 != memcmp(buffer1, buffer2, readBytes1))
		{
			//printf("files are different 2\n");
			return 1;
		}

	} while ((readBytes1 == BUFFER_SIZE) && (readBytes2 == BUFFER_SIZE));

	// files are identical!
	return 2;
}

