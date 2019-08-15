#include "tools.h"

void printf(char* string,DWORD hex)
{
	char buf[512];
	DWORD tmp;
	if(lstrlenA(string))
	{ 
		wsprintfA(buf,"%s %p\n",string,hex);
		HANDLE std=GetStdHandle(STD_OUTPUT_HANDLE);
		WriteFile(std,buf,(DWORD)lstrlenA(buf),&tmp,0);
	}
}

void printf(char* string)
{
	char buf[512];
	DWORD tmp;
	if(lstrlenA(string))
	{
		wsprintfA(buf,"%s\n",string);
		HANDLE std=GetStdHandle(STD_OUTPUT_HANDLE);
		WriteFile(std,buf,(DWORD)lstrlenA(buf),&tmp,0);
	}
}

void mem_zero(void *mem, int size)
{
	for(int i=0;i<size;i++)
		((BYTE*)mem)[i]=0;
}

void mem_copy(void *src, void *dest,int size)
{
	for(int i=0;i<size;i++){
		((BYTE*)dest)[i]=((BYTE*)src)[i];
	}
}


bool CompareString2(byte* str1,byte* str2,int len)
{
	bool valid=true;
	for(int i=0;i<len;i++)
	{
		if( str1[i]!=str2[i] ) valid=false;
	}
	return valid;
}
