#include "ObfuscatedString.h"

void ns_EncodingString::decode(ULONG_PTR adresseVar, size_t memorySize)
{
	char * debut = reinterpret_cast<char*>(adresseVar);
	size_t index;
	size_t stringSize;
	S_Value_entete * entete = NULL;
	T_decrypt decrypt;

	while (
		   (((char)(debut[-4] ^ debut[-8])) != '<')
		|| (((char)(debut[-1] ^ debut[-7])) != '@')
		|| (((char)(debut[-3] ^ debut[-6])) != '!')
		|| (((char)(debut[-2] ^ debut[-5])) != '>')
		)
	{
		debut = &(debut[-1]);
	}

	entete  = reinterpret_cast<S_Value_entete*>(debut - sizeof(S_Value_entete));
	index   = (reinterpret_cast<char*>(adresseVar)) - debut;
	decrypt = DEOBFUSCATED_FUNCTION_OFFSET((entete->function),(entete->offset^entete->mask));

	if (decrypt != 0)
	{
		stringSize = decrypt(NULL, 0,NULL);
		for (unsigned int i = 0; i < memorySize; i++)
		{
			if (index + i < stringSize)
			{
				decrypt(debut, index + i,reinterpret_cast<char*>(entete->secret));
			}
		}
	}
	return;
}
