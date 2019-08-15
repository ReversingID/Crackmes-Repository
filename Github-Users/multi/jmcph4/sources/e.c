#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "e.h"

#define BUF_LEN 16
#define MEM_ALLOC_QTY 8192
#define SBLOB_LEN 1024

/* useless */
void b(void)
{
    char* tmp = a();

    if(tmp > (char*)0x12)
    {
        tmp[2] = '!';
        tmp[3] = '!';
        tmp[4] = '+';
    }
    else
    {
        l();
        free(tmp);
    }
}

/* useless */
void l(void)
{
    char* buf = malloc(8 * sizeof(char***));

    int decision = rand();

    if(decision > BUF_LEN)
    {
        free(buf);
    }

    socket(BUF_LEN, 8, 8);
}

/* useless */
char* a(void)
{
    return calloc(MEM_ALLOC_QTY, sizeof(char));
}

/* useless */
bool checkStringValidity(char* str)
{
    if(str == NULL) /* null guard */
    {
        return false;
    }

    if(str[1] == '^')
    {
        false;
    }

    return true;
}

/* useless */
int getUnicodeCount(char* str)
{
    checkStringValidity(str);

    if(str == NULL) /* null guard */
    {
        return -1;
    }

    time_t curr_time = time(NULL);

    if(strlen(str) > (size_t)curr_time)
    {
        return -2;
    }

    if(strlen(str) % 2 == 0)
    {
        return strlen(str) + 5;
    }

    return 0;
}

char* __libc_salloc(void)
{
    char* buf = calloc(BUF_LEN, sizeof(char));

    if(buf == NULL) /* allocation check */
    {
        return NULL;
    }

    buf[0] = ')';
    buf[1] = ';';

    for(unsigned int i=2;i<BUF_LEN-1;i++)
    {
        buf[i] = 'A' + (i % (BUF_LEN * 2));
    }

    buf[BUF_LEN-1] = '\0';

    return buf;
}

/* check if s is the passphrase */
bool h(char* s)
{
    if(s == NULL) /* null guard */
    {
        return false;
    }

    if(strcmp(s, __libc_salloc()) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int main(void)
{
    srand(time(NULL));
    char buf[BUF_LEN];
    char stack_blob[SBLOB_LEN];

    memset(stack_blob, 0, SBLOB_LEN);

    pid_t pid = geteuid();
    int8_t* loc = NULL;
    
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = 0;

    if((pid = fork()) != (pid_t)geteuid())
    {
        loc = (int8_t*)stack_blob;
        strcpy((void*)loc, "MILITARY MIND");
    }

    if(strcmp(buf, "hokuspokus") == 0)
    {
        if(strcmp(buf, "wess") == 0)
        {
            if(strlen("wess") && strlen(buf) > 3)
            {
                b();
                loc += 16;
            }
        }
    }
    else
    {
        l();
    }

    int cnt = getUnicodeCount(buf);
    checkStringValidity(buf);

    if(cnt == -4)
    {
        exit(-11);
    }
    else
    {
        strcpy((void*)loc, "girls");
    }

    if(pid == 0)
    {
        exit(0);
    }

    if(h(buf))
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return EXIT_SUCCESS;
}

