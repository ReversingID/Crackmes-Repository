#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char w1[] = "banana";
char w2[] = "apple";
char w3[] = "pear";
char w4[] = "grapes";
char w5[] = "kumquats";

char * ws[] = {w1, w2, w3, w4, w5};
int is[] = {0, 4, 2, 1, 7};

int check_pw(char * guess)
{
  
  int i;

  for (i = 0; i < 5; i++)
  {
      if (guess[i] != ws[i][is[i]])
	  return 0;
  }
  
  // Check if the password matches the pw string
  return 1;
}

int main()
{
  char buf[16] = {0};
  printf("PASSWORD: ");
  scanf("%6s", buf);

  if (check_pw(buf))
	  printf("ACCESS GRANTED!\n");
  else
	  printf("ACCESS DENIED!\n");
	  
  return 0;
}
