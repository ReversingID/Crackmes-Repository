/*

  Crackme 01 - C Language - Linux/x86 and x86_64
  Copyright (C) 2013 - Geyslan G. Bem, Hacking bits

    http://hackingbits.com
    geyslan@gmail.com 

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>

#define A(c) (char) (((int) (c)) ^ 108)

char passwd[8] = { A('w'), A('h'), A('y'), A('n'), A('0'), A('t') };

void detect_gdb(void) __attribute__((constructor));

void detect_gdb(void)
{
  
  FILE *fd = fopen("/tmp", "r");
  if (fileno(fd) > 5)
    {
      printf("I'm sorry GDB! You are not allowed!\n");
      exit(1);
    }
  fclose(fd);

  if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
    printf("Tracing is not allowed... Bye\n");
    exit(1);
  }

}

void xor(char *p)
{
  int i;

  for (i = 0; i < 6; i++)
    {
      p[i] ^= 108;
    }
}

int compare(char *input, char *passwd)
{
  while (*input == *passwd)
    {
      if (*input == '\0' || *passwd == '\0' )
	break;

      input++;
      passwd++;
    }
  if (*input == '\0' && *passwd == '\0')
    return 0;
  else
    return -1;
}

int main() {

  char input[8];

  fputs("Please tell me my password: ", stdout);

  fgets(input, sizeof input -1, stdin);

  xor(input);

  if (compare(input, passwd) == 0)
    fputs("The password is correct!\nCongratulations!!!\n", stdout);
  else
    fputs("No! No! No! No! Try again.\n", stdout);

  return 0;

}
