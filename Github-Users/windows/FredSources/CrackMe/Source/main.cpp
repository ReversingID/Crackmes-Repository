#include <stdio.h>
#include <stdarg.h>

#include "CTrace.h"

#include "ObfuscatedString.h"
#include "ObfuscatedCall.h"
#include "CheckPassword.h"
#include "sha256.h"
#include "aes.h"
#include "Tools.h"

#ifdef OBFUSCATED
static size_t Obfuscated_printf (const char *format, ...)
{
	size_t ret ;
	va_list args;
	va_start (args, format);
	ret = vprintf (format, args);
	va_end (args);
	return ret ;
}

#undef printf
#define printf(...) (OBFUSCATED_CALL_RET(Obfuscated_printf,__VA_ARGS__))
#endif

//#define COMPUTE_SOLUCE

#ifndef COMPUTE_SOLUCE
const char * SecretUser = OBFUSCATED_STRING("Utilisateur");
const char * SecretPassword = OBFUSCATED_STRING("dr0wss@P-P@ssw0rd");
#define MESSAGE_SIZE 80
//message clair : Si tu es le premier tu pourras venir chercher une boite de Haribo ...
char  messageChiffre[MESSAGE_SIZE] = { '\x35','\xfa','\xc3','\x88','\xb3','\xbc','\xd5','\x54',
										'\xd0','\xdd','\x5c','\xce','\xa4','\x2b','\x6d','\xca',
										'\xd9','\xd2','\xcb','\x6f','\x74','\x1b','\x5e','\x67',
										'\x03','\x24','\xb1','\x7e','\xd7','\x23','\x56','\x9b',
										'\x4e','\x2c','\x54','\xed','\x25','\x4d','\x58','\x3a',
										'\x95','\x8e','\x77','\x02','\x46','\x2f','\x69','\x1c',
										'\x4f','\x45','\xaa','\x4d','\x3d','\x4f','\x1a','\x36',
										'\x3a','\x0e','\x09','\x52','\xa9','\xd5','\x72','\xf7',
										'\x1d','\xbb','\x19','\x59','\xbe','\x70','\xba','\x4b',
										'\x31','\xa0','\x75','\x69','\x7a','\x91','\x2f','\x0b'};

#endif

/**************************************************************************/
/* int  main(int argc, char * argv[])                                     */
/**************************************************************************/
int  main(int argc, char * argv[])
{
	int ret = 0;
	char *user = NULL;
	char *password = NULL;
	sha256_context sha_ctx;
	aes_context aes_ctx;
	uint8 digest[32];

	TRACE_ACTIVATE();
	TRACE_DEFAULT("");

#ifdef COMPUTE_SOLUCE
	uint8_t* messageClair = NULL;
	uint8_t* messageChiffre = NULL;
	size_t tailleMessage;

	printf("message Clair : ");
	tailleMessage = my_fgets(stdin,(char**)&messageClair);
	tailleMessage = tailleMessage - (tailleMessage % 16) + 16;
	messageClair = (uint8_t *)realloc(messageClair, tailleMessage);
	messageChiffre = (uint8_t *)malloc(tailleMessage);
	if (messageChiffre == NULL)
	{
		fprintf(stderr, "Allocation Failed !!! ");
		exit(-1);
	}
	
	printf("User : ");
	my_fgets(stdin,&user);

	printf("Password : ");
	my_fgets(stdin,&password);

	sha256_starts(&sha_ctx);
	sha256_update(&sha_ctx, (unsigned char *)user, (unsigned long)strlen(user));
	sha256_update(&sha_ctx, (unsigned char *)password, (unsigned long)strlen(password));
	sha256_finish(&sha_ctx, digest);
	
	aes_set_key(&aes_ctx, digest, 256);

	for (int i = 0; i < tailleMessage; i += 16)
	{
		aes_encrypt(&aes_ctx, &messageClair[i], &messageChiffre[i]);
	}

	printf("const char * SecretUser = OBFUSCATED_STRING(\"%s\");\n", user);
	printf("const char * SecretPassword = OBFUSCATED_STRING(\"%s\");\n",password);
	printf("#define MESSAGE_SIZE %u\n", (unsigned int)tailleMessage);
	printf("//message clair : %s\n", messageClair);
	printf("char messageChiffre[MESSAGE_SIZE] = { ");
	for (size_t i=0; i < tailleMessage; i++)
	{
		if (((i % 8) == 0) && i!=0) { printf("\n\t\t\t\t"); }
		printf("'\\x%02x'", messageChiffre[i]);
		if (i != tailleMessage - 1)  { printf(",");    }
	}
	printf("};\n");
	
	if (messageChiffre != NULL)
	{
		free(messageChiffre);
		messageChiffre = NULL;
	}

	if (messageClair != NULL)
	{
		free(messageClair);
		messageClair = NULL;
	}
#else

	char messageClair[MESSAGE_SIZE] ;

	printf(STATIC_OBFUSCATED_STRING("*************************************************\n"));
	printf(STATIC_OBFUSCATED_STRING("*****    CrackMe (by Fred. )          ***********\n"));
	printf(STATIC_OBFUSCATED_STRING("*************************************************\n\n"));

	#ifdef _DEBUG
		printf("Soluce = '%s'/'%s' \n", (char *)(SecretUser), (char *)(SecretPassword));
	#endif

	printf(STATIC_OBFUSCATED_STRING("User : "));
	my_fgets(stdin, &user);

	printf(STATIC_OBFUSCATED_STRING("Password : "));
	my_fgets(stdin, &password);

	if (checkPassword(user, password, SecretUser, SecretPassword))
	{

		printf(OBFUSCATED_STRING("Password OK!\n"));
		sha256_starts(&sha_ctx);
		sha256_update(&sha_ctx, (unsigned char *)(user), (unsigned long)(strlen(user)));
		sha256_update(&sha_ctx, (unsigned char *)(password), (unsigned long)(strlen(password)));
		sha256_finish(&sha_ctx, digest);

		aes_set_key(&aes_ctx, digest, 256);
		
		for (size_t i = 0; i < MESSAGE_SIZE; i += 16)
		{
			aes_decrypt(&aes_ctx, (uint8_t*)&messageChiffre[i], (uint8_t*)&messageClair[i]);
		}
		messageClair[sizeof(messageChiffre)-1] = '\0';
		
		printf(OBFUSCATED_STRING("Message secret : %s\n"), (char*)messageClair);

		
	}
	else
	{
		printf(STATIC_OBFUSCATED_STRING("Mauvais password ... !\n"));
	}

#endif

	if (user != NULL)
	{
		free(user);
		user=NULL;
	}
	if (password != NULL)
	{
		free(password);
		password=NULL;
	}

	printf(STATIC_OBFUSCATED_STRING("Press a key !!!"));
	getc(stdin);
	return ret ;
}

