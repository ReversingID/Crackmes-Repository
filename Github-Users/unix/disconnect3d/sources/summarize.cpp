#include <iostream>
#include <cstdio>
#include <time.h>

using namespace std;

const struct timespec sleep_time = {0, 1};

void foobar()
{
    char buf[4096];
    nanosleep(&sleep_time, NULL);
    FILE* fp = fopen("/dev/urandom", "r");

    nanosleep(&sleep_time, NULL);
    for(int i=0; i<512; ++i)
    {
        FILE* of = fopen("/dev/null", "w");
        nanosleep(&sleep_time, NULL);
        fread(buf, sizeof(char), 4096, fp);
        fprintf(of, "asd");
        fclose(of);

        if (!(i%10))
            nanosleep(&sleep_time, NULL);
    }

    nanosleep(&sleep_time, NULL);
    fclose(fp);
}

int main()
{
    nanosleep(&sleep_time, NULL);
    cout << WELCOME << endl;
    foobar();
    nanosleep(&sleep_time, NULL);
    cout << "The syscall that was called the most times is the next password" << endl;

}
