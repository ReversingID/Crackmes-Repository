/*

  Fake ptrace - C Language - Linux/x86 and x86_64
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


long ptrace(int request, int pid, int addr, int data)
{
  printf("I'm fake");
  return 0;
}
