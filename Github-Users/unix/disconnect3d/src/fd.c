#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[32];

int main(int argc, char* argv[]) {
    printf("%s\n", WELCOME);

    if (argc != 2) {
        printf("pass argv[1] a number\n");
        return 0;
    }

    int fd = atoi(argv[1]) - 0x1234;
    read(fd, buf, 32);

    if (!strcmp("hi\n", buf)) {
        puts("Good job :)");
        printf("The next pasword is: %s\n", PASS);
        exit(0);
    }
    else {
        puts("First make me read from proper file descriptor.");
        puts("Next, tell me 'hi'"); 
        puts("Hint: Read syscall errors");
        puts("Do you know any useful fds?");
    }

    printf("-------------------------------------------------------------------\n");
    printf("The code is a modified version of fd exercise taken from pwnable.kr\n");
    return 0;
}

