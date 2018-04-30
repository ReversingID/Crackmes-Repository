#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char flag[] = "I'm a flag";

int main(int argc, char * argv[])
{
	if (argc < 2) {
		printf("No input\n");
		return 1;
	}
	if (!strcmp(argv[1], flag)) {
		printf("YAY! You got the flag!\n");
	}
	else {
		printf("Umm... No.\n");
		
	}
	return 0;
}
