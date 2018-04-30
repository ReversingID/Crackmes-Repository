#ifndef __DECODE__H__
#define __DECODE__H__

bool checkPassword(char * User, char * Password, const char *SecretUser, const char *SecretPassword);

#ifdef OBFUSCATED

static inline bool Obfuscated_checkPassword(char * User, char * Password, const char *SecretUser, const char *SecretPassword)
{
	return checkPassword(User,Password,SecretUser,SecretPassword) ;
}
#undef checkPassword
#define checkPassword(...) (OBFUSCATED_CALL_RET(Obfuscated_checkPassword,__VA_ARGS__))

#endif

#endif
