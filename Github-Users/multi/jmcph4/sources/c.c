#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 32
#define MAGIC_STRING_LEN 8

/* generate magic string */
char* getFailString(void)
{
    char* s = malloc(MAGIC_STRING_LEN);

    for(unsigned int i=0;i<MAGIC_STRING_LEN;i++)
    {
        s[i] = i + 65;
    }

    return s;
}

/* print success string */
void printSuccess(void)
{
    printf("PASS\n");
}

/* print failure string */
void printFailString(void)
{
    printf("FAIL\n");
}

int main(void)
{
    char buf[BUF_LEN];

    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = 0;

    if(strcmp(buf, getFailString()) == 0)
    {
        printSuccess();
    }
    else
    {
        printFailString();
    }

    return EXIT_SUCCESS;
}

