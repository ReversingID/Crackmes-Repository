/*

  Crackme 04 - C Language - Linux/x86
  Copyright (C) 2017 - Geyslan G. Bem, Hacking bits

  http://hackingbits.github.io
  geyslan@gmail.com

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define A(c) (char) (((int) (c)) ^ 144)

char passwd[11] = { A('h'), A('w'), A('t'), A('a'), A('h'), A('a'), A('l'), A('3'), A('l') };
char tmp[11];

int result = 0;
char t = 0;
void (*fn)(char) = (void *) &t;

void antidebug(void) __attribute__((constructor));

void antidebug(void)
{
	FILE *fd = fopen("/tmp", "r");
	if (fileno(fd) > 5) {
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

	if (result < 0)	{
		printf("Tracing is not allowed... Bye!\n");
		exit(1);
	}
	t = (char) time(0);
	for (int i = 0; i < 9; ++i)
		passwd[i] ^= t;
}

char stage2(char p)
{
	result = p;

	if (!t) {
		(*fn)(p);
	}

	__asm__ ("jmp .+4\n\t"
		 ".byte 0xc9\n\t"
		 ".byte 0x35\n\t"

		 "pushl %edx\n\t"
		 "xor %edx, %edx\n\t"
		 "movl result, %edx\n\t"

		 "jmp .+4\n\t"
		 ".byte 0x08\n\t"
		 ".byte 0x5a\n\t"
		 "xorb t, %dl\n\t"

		 "jmp .+4\n\t"
		 ".byte 0x09\n\t"
		 ".byte 0x5b\n\t"
		 "xorb $144, %dl\n\t"
		 "movb %dl, result\n\t"
		 "popl %edx");

	return result;
}

void stage1(char *p)
{
	int i;

	for (i = 0; i < 8; i += 2) {
		tmp[i+1] = stage2(p[i]);
		tmp[i] = stage2(p[i+1]);
	}
	tmp[i] = stage2(p[i]);
	tmp[i+1] = '\0';
}

int compare(char *input, char *passwd)
{
	while (*input && *passwd) {
		if (*input != *passwd)
			return -1;
		++input;
		++passwd;
	}

	return (*input == *passwd) ? 0 : -1;
}

int main(void)
{
	char input[11];

	fputs("Please tell me my password: ", stdout);

	fgets(input, sizeof(input) - 1, stdin);
	stage1(input);

	if (compare(tmp, passwd) == 0)
		fputs("The password is correct!\nCongratulations!!!\n", stdout);
	else
		fputs("No! No! No! No! Try again.\n", stdout);

	return 0;
}
