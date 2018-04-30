#include <stdio.h>

int main()
{
    printf("%s\n", WELCOME);

    puts("Sending secret message to void...");

    FILE* fp = fopen("/dev/null", "w");
    fprintf(fp, "password: %s\n", PASS);
    fclose(fp);
    
    puts("Secret message has been sent.");
}
