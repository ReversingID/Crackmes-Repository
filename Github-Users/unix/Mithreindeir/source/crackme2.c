#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char flag[] = "I'm a flag.";
int main()
{
	char buffer[20];
	printf("ENTER KEY: \n");
	fgets(buffer, 20, stdin);	
	buffer[strcspn(buffer, "\r\n")] = 0;
	int len = strlen(buffer);
	for (int i = 0; i < len; i++) {
		buffer[i] +=  i;
	}
	if (!strcmp(buffer, flag)) {
		printf("Yay! You got the flag!\n");
	}
	else {
		printf("Boo! You did not get the flag!\n");
	}
	return 0;
}
