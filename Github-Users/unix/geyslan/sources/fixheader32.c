/*

  Fix Header 32 - C Language - Linux/x86
  Copyright (C) 2013 - Geyslan G. Bem, Hacking bits

    http://hackingbits.com
    geyslan@gmail.com 

  It's a fork from

    about:blog
    https://dustri.org/b/?p=832

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
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <fcntl.h>
 
 
int main(int argc, char** argv){
  printf(".: Elf Repair :.\n");
 
  if(argc < 2){
    printf("Usage: %s file", argv[0]);
    return 1;
  }
  else{
 
    int f;
    static Elf32_Ehdr* header;
 
    if((f = open(argv[1], O_RDWR)) < 0){
      perror("open");
      return 1;
    }
 
    if((header = (Elf32_Ehdr *) mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE, MAP_SHARED, f, 0)) == MAP_FAILED){
      perror("mmap");
      close(f);
      return 1;
    }
    printf("[*] Current e_shnum: %d\n", header->e_shnum);
 
    // Here is the secret!
    header->e_shoff = 0;
    header->e_shnum = 0;
    header->e_shstrndx = 0;
 
    printf("[*] Patched e_shnum: %d\n", header->e_shnum);
 
    if(msync(NULL, 0, MS_SYNC) == -1){
      perror("msync");
      close(f);
      return 1;
    }
 
    close(f);
    munmap(header, 0);
  }
  return 0;
}
