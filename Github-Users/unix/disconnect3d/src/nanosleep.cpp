#include <iostream>
#include <cstdio>
#include <unistd.h>

using namespace std;

int PASS()
{
    int sum=0;
    sleep(1);

    for(int i=0; i<1024*1024; ++i)
        sum += i;

    return sum;
}

int password_is_sumsth()
{
    int sum = 0;
    for(int i=0; i<8; ++i)
        sum += PASS();

    return sum;
}

int main()
{
    cout << WELCOME << endl;
    int x = password_is_sumsth();
    cout << "The function that took the most execution time is the next password" << endl;
}
