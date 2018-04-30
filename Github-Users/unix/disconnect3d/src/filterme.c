#include <stdio.h>

char buf[2048];

char p = 'p';
char a = 'a';
char s = 's';
char w = 'w';
char o = 'o';
char r = 'r';
char d = 'd';
char space = ' ';
char i = 'i';
char f = 'f';

int main()
{
    puts(WELCOME);
    puts("'I read a lot, but I am so old that you have to put my whispers together to understand me...'");

    FILE *whispering = fopen("/dev/null", "w");
    FILE* reading = fopen("/dev/urandom", "r");
    
    fprintf(whispering, "%c", p); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", a); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", s); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", s); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", w); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", o); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", r); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", d); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", space); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", i); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", s); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", space); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", f); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);
    fprintf(whispering, "%c", d); fflush(whispering);
    for(int i=0; i<1024; ++i)
        fread(buf, sizeof(char), 2048, reading);

    fclose(reading);
    fclose(whispering);
}
