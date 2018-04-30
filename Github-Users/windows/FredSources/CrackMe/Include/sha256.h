#ifndef _SHA256_H
#define _SHA256_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define SHA_256_SIZE 32

typedef struct s_sha256_context
{
    uint32 total[2];
    uint32 state[8];
    uint8 buffer[64];
} sha256_context;

void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, uint8 *input, uint32 length );
void sha256_finish( sha256_context *ctx, uint8 digest[SHA_256_SIZE] );

#ifdef OBFUSCATED

static inline void Obfuscated_sha256_starts( sha256_context *ctx )
{
	sha256_starts( ctx ) ;
}
#undef sha256_starts
#define sha256_starts(...) (OBFUSCATED_CALL(Obfuscated_sha256_starts,__VA_ARGS__))

static inline void Obfuscated_sha256_update( sha256_context *ctx, uint8 *input, uint32 length  )
{
	sha256_update( ctx, input, length ) ;
}
#undef sha256_update
#define sha256_update(...) (OBFUSCATED_CALL(Obfuscated_sha256_update,__VA_ARGS__))

static inline void Obfuscated_sha256_finish( sha256_context *ctx, uint8 digest[SHA_256_SIZE] )
{
	sha256_finish( ctx, digest) ;
}
#undef sha256_finish
#define sha256_finish(...) (OBFUSCATED_CALL(Obfuscated_sha256_finish,__VA_ARGS__))

#endif

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* sha256.h */
