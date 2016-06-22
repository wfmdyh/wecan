#include <string.h>
#include <stdio.h>
#include "types.h"
#include "crypto.h"
#include "des3.h"

/*****************************************************************************
 函 数 名  : pboc_des3_encrypt
 功能描述  : 以3DES-ECB（EDE）模式加密数据，加密过程如下，
 2. 将第一步中生成的数据块分解成8字节数据块，标号为D1，D2，D3，D4等等。
 最后一个数据块长度有可能不足8位。
 3. 如果最后(或唯一)的数据块长度等于8字节，转入第四步；如果不足8字节，
 在右边添加16进制数字 '80'。如果长度已达8字节，转入第四步；否则，
 在其右边添加1字节16进制数字 '00', 直到长度达到8字节。
 4. 每一个数据块使用8.3.3.1中描述的数据加密方式加密。
 参见"PBOC 2.0 第一部分 卡片规范  8.3 数据可靠性"

 输入参数  : const uint8_t enkey[DOUBLE_KEY_SIZE] 双长度（16字节）加密密钥
 const uint8_t *inbuf 输入明文
 int len 输入明文长度，注意，明文长度不能超过255字节
 输出参数  : uint8_t *outbuf 输出密文
 返 回 值  : int  返回 0
 *****************************************************************************/
int pboc_des3_encrypt(const UINT8 enkey[DOUBLE_KEY_SIZE], const UINT8 *inbuf,
		int len, UINT8 *outbuf) {
	des3_context ctx = { { 0 }, { 0 } };
	UINT8 buf[DES_BLOCK_SIZE] = { 0 };
	UINT8 *pdata;

	/* compute length of encrypted data */
	int enclen = (len + 1 + (DES_BLOCK_SIZE - 1)) & (~7);

	des3_set_2keys(&ctx, (UINT8 *) enkey, (UINT8 *) (enkey + DES_KEY_SIZE));

#if 0 /* use in pin encrpty */
	/* encrypt the 1st block, prepad length UINT8 before data */
	buf[0] = (UINT8)len;
	if(len < DES_BLOCK_SIZE - 1)
	{
		memcpy(buf + 1, (void *)inbuf, len);
		buf[len + 1] = 0x80;
	}
	else
	{
		memcpy(buf + 1, (void *)inbuf, DES_BLOCK_SIZE - 1);
	}
#endif
	if (len < DES_BLOCK_SIZE - 1) {
		memcpy(buf, (void *) inbuf, len);
		buf[len] = 0x80;
	} else {
		memcpy(buf, (void *) inbuf, DES_BLOCK_SIZE);
	}

	des3_encrypt(&ctx, buf, outbuf);

	/* encrypt remaining blocks except the last one */
	len -= DES_BLOCK_SIZE - 1;
	pdata = (UINT8 *) inbuf + DES_BLOCK_SIZE - 1;
	outbuf += DES_BLOCK_SIZE;

	while (len >= DES_BLOCK_SIZE) {
		des3_encrypt(&ctx, pdata, outbuf);

		len -= DES_BLOCK_SIZE;
		pdata += DES_BLOCK_SIZE;
		outbuf += DES_BLOCK_SIZE;
	}

	/* encrypt the last block, padding it with '80 00 ...'
	 requested by PBOC STD. */
	if (len > 0) {
		memset(buf, 0, DES_BLOCK_SIZE);
		memcpy(buf, pdata, len);
		buf[len] = 0x80;
		des3_encrypt(&ctx, buf, outbuf);
	}

	memset(&ctx, 0, sizeof(des3_context));
	return enclen;
}

/*****************************************************************************
 函 数 名  : pboc_des3_decrypt
 功能描述  : 以3DES-ECB（EDE）模式解密数据。
 输入参数  : const uint8_t dekey[DOUBLE_KEY_SIZE] 双长度（16字节）解密密钥
 const uint8_t *inbuf 输入密文
 int len 输入密文长度，注意应为8的整数倍
 输出参数  : uint8_t *outbuf 输出明文
 返 回 值  : int  返回 0
 修改历史  :
 *****************************************************************************/
int pboc_des3_decrypt(const UINT8 dekey[DOUBLE_KEY_SIZE], const UINT8 *inbuf,
		int len, UINT8 *outbuf) {
	des3_context ctx = { { 0 } };
	int i;

	des3_set_2keys(&ctx, (UINT8 *) dekey, (UINT8 *) (dekey + DES_KEY_SIZE));

	/* decrypt data in blocks */
	for (i = 0; i < len / DES_BLOCK_SIZE; i++) {
		des3_decrypt(&ctx, (UINT8 *) (inbuf + i * DES_BLOCK_SIZE),
		outbuf + i*DES_BLOCK_SIZE);
	}

	memset(&ctx, 0, sizeof(des3_context));
	return 0;
}

/**
 * 密钥分散
 * 推到DK左半部分的方法是：
 * 1、将分散数据的最右8个字节作为输入数据；
 * 2、将MK作为加密密钥；
 * 3、用MK对输入数据进行3DES运算；
 * 推到DK右半部分的方法是：
 4、将分散数据的最右8个字节求反，作为输入数据；
 5、将MK作为加密密钥；
 6、用MK对输入数据进行3DES运算；
 */
void pboc_keydiv(uint8_t kr[DOUBLE_KEY_SIZE], uint8_t divfac[DES_KEY_SIZE],
		uint8_t doutk[DOUBLE_KEY_SIZE]) {
	int i;
	uint8_t tk[DES_KEY_SIZE];

	/* 1: */
	memcpy(tk, divfac, sizeof(tk));
	/* 2, 3: */
	pboc_des3_encrypt(kr, tk, DES_KEY_SIZE, &doutk[0]);

	/* 4: */
	for (i = 0; i < DES_KEY_SIZE; i++) {
		tk[i] = ~divfac[i];
	}
	/* 5, 6 */
	pboc_des3_encrypt(kr, tk, DES_KEY_SIZE, &doutk[DES_KEY_SIZE]);
}

#if 0
/**
 * test module
 */
void tst_encrypt_decrypt(void)
{
	int i;
	uint8_t tkey[DOUBLE_KEY_SIZE] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef,
		0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
	uint8_t tinbs[] = {0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t divfac[DES_KEY_SIZE] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36,0x37, 0x38};
	uint8_t divkey_result[16];
	uint8_t encrypt_result[8];
	uint8_t decrypt_result[8];

	printf("org data:");
	for (i=0; i<sizeof(tinbs); i++) {
		printf(" %02x,", tinbs[i]);
	}
	printf("\r\n");

	pboc_des3_encrypt(tkey, tinbs, sizeof(tinbs), encrypt_result);
	printf("encrpty result:");
	for (i=0; i<sizeof(encrypt_result); i++) {
		printf(" %02x,", encrypt_result[i]);
	}
	printf("\r\n");

	pboc_des3_decrypt(tkey, encrypt_result, sizeof(encrypt_result), decrypt_result);
	printf("decrypt with encrpty data result:");
	for (i=0; i<sizeof(decrypt_result); i++) {
		printf(" %02x,", decrypt_result[i]);
	}
	printf("\r\n");

	printf("div key result:");
	pboc_keydiv(tkey, divfac, divkey_result);
	for (i=0; i<sizeof(divkey_result); i++) {
		printf(" %02x,", divkey_result[i]);
	}

}

#endif
