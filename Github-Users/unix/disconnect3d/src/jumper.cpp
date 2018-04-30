#include <iostream>
#include <sys/ptrace.h>

using namespace std;

void getpass()
{
    int offset = 5887;
    int tab[8] = {5994, 5994, 6003, 5991, 6007, 5985, 6008, 5988};

    cout << "\nYour password is: ";
    for (int i = 0; i < 8; ++i)
        cout << (char)(tab[i]-offset);

    cout << flush;
}

int main()
{
    cout << endl;

    if (ptrace(PTRACE_TRACEME, 0, NULL, 0) == -1)
    {
        cout << endl << "Sorry, debugging is not allowed. Exiting..." 
                 << endl << endl;
        return 0;
    }
    
    cout << endl << "Glad you don't use debugger." << endl;
    cout << endl << "Invoking function that shows your password...\n" 
             << flush;

    volatile int a = 0;
    cout << 5 / a;
    
    getpass();  

    return 0;
}
