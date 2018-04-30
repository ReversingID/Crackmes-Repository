#include <stdlib.h>
#include <string.h>

#include "AntiDebug.h"
#include "CheckPassword.h"
#include "CTrace.h"
#include "ObfuscatedCall.h"

#ifdef OBFUSCATED
static inline int Obfuscated_strcmp (const char *s1, const char *s2)
{
	return strcmp(s1,s2) ;
}
#undef strcmp
#define strcmp(...) (OBFUSCATED_CALL_RET(Obfuscated_strcmp,__VA_ARGS__))


static inline size_t Obfuscated_strlen (const char *s1)
{
	return strlen(s1) ;
}
#undef strlen
#define strlen(...) (OBFUSCATED_CALL_RET(Obfuscated_strlen,__VA_ARGS__))

static inline unsigned int  Obfuscated_check_memory(void)
{
	return AntiDebug::getInstance().check_memory();
}
#undef check_memory
#define check_memory(...) (OBFUSCATED_CALL_RET(Obfuscated_check_memory,__VA_ARGS__))

static inline void Obfuscated_debugger(void)
{
	AntiDebug::getInstance().debugger();
}
#undef debugger
#define debugger(...) (OBFUSCATED_CALL(Obfuscated_debugger,__VA_ARGS__))

#endif

/**************************************************************************/
/* bool checkPassword(char * User , char * Password)                      */
/**************************************************************************/
bool checkPassword(char * User , char * Password, const char *SecretUser, const char *SecretPassword)
{
	TRACE_DEFAULT(_STR2WSTR("Start"));
	int ret_User;
	int ret_Password;

	if (Obfuscated_check_memory())
	{
		 Obfuscated_debugger();
	}

	if ((strlen(User)!=0) && (strlen(Password)!=0))
	{

		ret_User = strcmp(User, (char*)SecretUser);
		TRACE_DEFAULT(_STR2WSTR("strcmp(User, SecretUser) = %d"),ret_User);

		ret_Password = strcmp(Password, (char*)SecretPassword);
		TRACE_DEFAULT(_STR2WSTR("strcmp(Password, SecretPassword) = %d"),ret_Password);

		if ((ret_User == ret_Password) && (ret_User == 0) )
		{
			TRACE_DEFAULT(_STR2WSTR("Result = true"));
			return true;
		}
		else
		{
			TRACE_DEFAULT(_STR2WSTR("Result = false"));
			return false;
		}
	}

	TRACE_DEFAULT(_STR2WSTR("Result = false"));
	return false;
}
