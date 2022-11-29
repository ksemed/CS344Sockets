#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>     
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <time.h>

int main(int argc, char const *argv[])
{

	if (argc == 2)
	{
		srand(time(NULL));
		
		int keylength = atoi(argv[1]);

		char* key = calloc(keylength + 1, sizeof(char));

		for (int i = 0; i < keylength; i++)
		{
			int letter = 65 + rand() % 26;
			key[i] = letter;
		}

		key[keylength] = '\0';

		printf("%s\n", key);
	}
	else
	{
		printf("Error, Redo Input\n");
	}

}