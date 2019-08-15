#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "b.h"

char* getMagicString(void)
{
    char* s = malloc(MAGIC_STRING_LEN * sizeof(char));

    if(s == NULL) /* allocation check */
    {
        return NULL;
    }

    unsigned int acc = MAGIC_STRING_LEN + 40;

    /* generate magic string */
    for(unsigned int i=0;i<MAGIC_STRING_LEN;i++)
    {
        s[i] = acc - i;
    }

    return s;
}

#define BUF_LEN 32

int main(void)
{
    char buf[BUF_LEN];

    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = 0;

    if(strcmp(buf, getMagicString()) == 0)
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return EXIT_SUCCESS;
}

