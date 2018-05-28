#include <stdlib.h>
#include <stdio.h>

int check_pw(char * guess)
{
  // Password is letmein
  char pw[] = "letmein";
  // Check if the password matches the pw string
  return (0 == strcmp(pw, guess));
}

int main()
{
  char buf[64] = {0};
  printf("PASSWORD: ");
  scanf("%63s", buf);

  if (check_pw(buf))
	  printf("ACCESS GRANTED!\n");
  else
	  printf("ACCESS DENIED!\n");
	  
  return 0;
}
