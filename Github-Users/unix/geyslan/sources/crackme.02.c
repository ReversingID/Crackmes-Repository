/*

  Crackme 02 - C Language - Linux/x86 and x86_64
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


#define A(c) (char) (((int) (c)) | 144)

char passwd[11] = { A('w'), A('h'), A('a'), A('t'), A('a'), A('h'), A('3'), A('l'), A('l') };

int result = 0;

void antidebug(void) __attribute__((constructor));

void antidebug(void)
{

  
  FILE *fd = fopen("/tmp", "r");
  if (fileno(fd) > 5)
    {
      printf("I'm sorry GDB! You are not allowed!\n");
      exit(1);
    }
  fclose(fd);
  

  // ptrace(PTRACE_TRACEME, 0, 1, 0)
  // using the syscall and avoiding the LD_PRELOAD
 
  __asm__ ("pusha\n\t"

	   "jmp .+4\n\t"
	   ".word 0xc9c0\n\t"

	   "movl $26, %eax\n\t"

	   "jmp . +4\n\t"
           ".word 0xcd0f\n\t"

	   "movl $0, %ebx\n\t"

	   "jmp .+4\n\t"
           ".word 0xc9c0\n\t"

	   "movl $0, %ecx\n\t"

	   "jmp . +4\n\t"
           ".word 0xcd0f\n\t"

	   "movl $1, %edx\n\t"

	   "jmp . +4\n\t"
           ".word 0xc90f\n\t"

	   "movl $0, %esi\n\t"

	   "jmp . +4\n\t"
           ".word 0xcd0f\n\t"

	   "int $0x80\n\t"

           "jmp . +4\n\t"
           ".word 0x0fcd\n\t"

           "movl %eax, result\n\t"
           "popa");

  if (result < 0)
    {
      printf("Tracing is not allowed... Bye!\n");
      exit(1);
    }
}


char stage2(char p)
{
  result = p;

  __asm__ ("jmp .+4\n\t"
	   ".byte 0xc9\n\t"
	   ".byte 0x35\n\t"

	   "pushl %edx\n\t"
	   "movl result, %edx\n\t"

	   "jmp .+4\n\t"
           ".byte 0x08\n\t"
           ".byte 0x5a\n\t"

	   "or $144, %edx\n\t"
	   "movl %edx, result\n\t"
	   "popl %edx");

  return result;
}

void stage1(char *p)
{
  int i;

  for (i = 0; i < 9; i++)
    {
      p[i] = stage2(p[i]);
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

int main()
{

  char input[11];

  fputs("Please tell me my password: ", stdout);

  fgets(input, sizeof input -1, stdin);

  stage1(input);

  if (compare(input, passwd) == 0)
    fputs("The password is correct!\nCongratulations!!!\n", stdout);
  else
    fputs("No! No! No! No! Try again.\n", stdout);

  return 0;

}
