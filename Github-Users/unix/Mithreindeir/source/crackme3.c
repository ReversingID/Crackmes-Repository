#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char * flag = "I'm a flag.";
void  esrever(char * str, int len)
{
	int j = 0;
	char * str2 = malloc(len);
	for (int i = len-1; i >= 0; i-- && j++) {
		str2[j] = str[i];
	}
	for (int i = 0; i < len; i++) {
		str[i] = str2[i];
	}
	free(str2);
}

int main()
{	
	char buffer[20];
	printf("ENTER KEY: \n");
	fgets(buffer, 20, stdin);
	buffer[strcspn(buffer, "\n")] = 0;	
	esrever(buffer, strlen(buffer));
	if (!strcmp(flag, buffer)) {
		printf("You got the flag!\n");
	}
	else {
		printf("You did NOT get the flag!\n");
	}
	return 0;
}
