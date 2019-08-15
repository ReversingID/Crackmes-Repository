#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 8
#define MAGIC_STRING "trance"

int main(void)
{
    char buf[BUF_LEN];

    scanf("%s", buf);

    if(strcmp(buf, MAGIC_STRING) == 0)
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return EXIT_SUCCESS;
}

