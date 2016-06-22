#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#define DES_BLOCK_SIZE		(8)
#define DES_KEY_SIZE        (8)
#define DOUBLE_KEY_SIZE     (DES_KEY_SIZE * 2)

int pboc_des3_encrypt(const uint8_t enkey[DOUBLE_KEY_SIZE], const uint8_t *inbuf, int len, uint8_t *outbuf);
int pboc_des3_decrypt(const uint8_t dekey[DOUBLE_KEY_SIZE], const uint8_t *inbuf, int len, uint8_t *outbuf);
void pboc_keydiv(uint8_t kr[DOUBLE_KEY_SIZE], uint8_t divfac[DES_KEY_SIZE], uint8_t doutk[DOUBLE_KEY_SIZE]);

#endif //_CRYPTO_H_
