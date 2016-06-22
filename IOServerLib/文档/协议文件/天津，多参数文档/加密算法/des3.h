#ifndef _DES_H_
#define _DES_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "types.h"

#define LITTLE_END  1

#define DES3_INSERT_ZERO_MODE 0
#define DES3_INSERT_OTHER_MODE 1

typedef struct
{
	unsigned long esk[32];
	unsigned long dsk[32];
}des_context;

typedef struct
{
	unsigned long esk[96];
	unsigned long dsk[96];
}des3_context;

int des_set_key( des_context *ctx, unsigned char key[8] );
void des_encrypt( des_context *ctx, unsigned char input[8], unsigned char output[8] );
void des_decrypt( des_context *ctx, unsigned char input[8], unsigned char output[8] );
void des_cbc_encrypt( des_context *ctx, unsigned char iv[8], unsigned char *input, unsigned char *output, int len );
void des_cbc_decrypt( des_context *ctx, unsigned char iv[8], unsigned char *input, unsigned char *output, int len );
int des3_set_2keys( des3_context *ctx, unsigned char key1[8], unsigned char key2[8] );
//int des3_set_3keys( des3_context *ctx, unsigned char key1[8], unsigned char key2[8], unsigned char key3[8] );
void des3_encrypt( des3_context *ctx, unsigned char input[8], unsigned char output[8] );
void des3_decrypt( des3_context *ctx, unsigned char input[8], unsigned char output[8] );
void des3_cbc_encrypt( des3_context *ctx, unsigned char iv[8], unsigned char *input, unsigned char *output, int len );
void des3_cbc_decrypt( des3_context *ctx, unsigned char iv[8], unsigned char *input, unsigned char *output, int len );
void GetDES3Key(char *key,char *Des3key);
#ifdef __cplusplus
}
#endif

#endif //_DES3_H_
