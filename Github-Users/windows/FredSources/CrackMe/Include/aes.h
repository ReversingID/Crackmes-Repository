#ifndef _AES_H
#define _AES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct s_aes_context
{
    uint32_t erk[64];     /* encryption round keys */
    uint32_t drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
}
aes_context;

int  aes_set_key( aes_context *ctx, uint8_t *key, int nbits );
void aes_encrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] );
void aes_decrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] );


#ifdef __cplusplus
}
#endif //__cplusplus

#ifdef OBFUSCATED

static inline int Obfuscated_aes_set_key( aes_context *ctx, uint8_t *key, int nbits )
{
	return aes_set_key(ctx,key,nbits) ;
}
#undef aes_set_key
#define aes_set_key(...) (OBFUSCATED_CALL_RET(Obfuscated_aes_set_key,__VA_ARGS__))

static inline void Obfuscated_aes_encrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] )
{
	aes_encrypt(ctx,input,output) ;
}
#undef aes_encrypt
#define aes_encrypt(...) (OBFUSCATED_CALL(Obfuscated_aes_encrypt,__VA_ARGS__))

static inline void Obfuscated_aes_decrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] )
{
	aes_decrypt(ctx,input,output) ;
}
#undef aes_decrypt
#define aes_decrypt(...) (OBFUSCATED_CALL(Obfuscated_aes_decrypt,__VA_ARGS__))

#endif

#endif /* aes.h */
