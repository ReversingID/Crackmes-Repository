#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "d.h"

/* return pass string */
char* A(void)
{
    char* s = calloc(A_LEN, sizeof(char));

    if(s == NULL)
    {
        return NULL;
    }

    /* hardcode string */
    s[0] = 80;
    s[1] = 65;
    s[2] = 83;
    s[3] = 83;
    s[4] = 0;

    return s;
}

/* return fail string */
char* B(void)
{
    char* s = calloc(B_LEN, sizeof(char));
   
    if(s == NULL)
    {
        return NULL;
    }

    sprintf(s, "FAIL");

    return s;
}

/* return passphrase */
char* C(void)
{
    unsigned int tmp = 0; /* useless */

    char* s = malloc(C_LEN);

    if(s == NULL)
    {
        return NULL;
    }

    char* res = D(s);

    /* useless */
    if(res == NULL)
    {
        tmp++;
    }

    return s;
}

/* build the passphrase */
char* D(char* d)
{
    /* useless */
    if(d == NULL)
    {
        return (char*)0x12;
    }

    /* build string */
    for(unsigned int i=0;i<C_LEN;i++)
    {
        d[i%C_LEN] = 0x70 + i;
        write_string_to_memory(d); /* useless */
    }

    return NULL;
}

/* useless */
void write_string_to_memory(char* d)
{
    memcpy((void*)0x55f4, d, 0);
}

int main(void)
{
    char* soln = C();
    char buf[BUF_LEN];

    scanf("%s", buf);

    if(strcmp(buf, soln) == 0)
    {
        puts(A());
    }
    else
    {
        puts(B());
    }

    return EXIT_SUCCESS;
}

