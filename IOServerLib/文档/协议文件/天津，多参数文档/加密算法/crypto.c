#include <string.h>
#include <stdio.h>
#include "types.h"
#include "crypto.h"
#include "des3.h"

/*****************************************************************************
 �� �� ��  : pboc_des3_encrypt
 ��������  : ��3DES-ECB��EDE��ģʽ�������ݣ����ܹ������£�
 2. ����һ�������ɵ����ݿ�ֽ��8�ֽ����ݿ飬���ΪD1��D2��D3��D4�ȵȡ�
 ���һ�����ݿ鳤���п��ܲ���8λ��
 3. ������(��Ψһ)�����ݿ鳤�ȵ���8�ֽڣ�ת����Ĳ����������8�ֽڣ�
 ���ұ����16�������� '80'����������Ѵ�8�ֽڣ�ת����Ĳ�������
 �����ұ����1�ֽ�16�������� '00', ֱ�����ȴﵽ8�ֽڡ�
 4. ÿһ�����ݿ�ʹ��8.3.3.1�����������ݼ��ܷ�ʽ���ܡ�
 �μ�"PBOC 2.0 ��һ���� ��Ƭ�淶  8.3 ���ݿɿ���"

 �������  : const uint8_t enkey[DOUBLE_KEY_SIZE] ˫���ȣ�16�ֽڣ�������Կ
 const uint8_t *inbuf ��������
 int len �������ĳ��ȣ�ע�⣬���ĳ��Ȳ��ܳ���255�ֽ�
 �������  : uint8_t *outbuf �������
 �� �� ֵ  : int  ���� 0
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
 �� �� ��  : pboc_des3_decrypt
 ��������  : ��3DES-ECB��EDE��ģʽ�������ݡ�
 �������  : const uint8_t dekey[DOUBLE_KEY_SIZE] ˫���ȣ�16�ֽڣ�������Կ
 const uint8_t *inbuf ��������
 int len �������ĳ��ȣ�ע��ӦΪ8��������
 �������  : uint8_t *outbuf �������
 �� �� ֵ  : int  ���� 0
 �޸���ʷ  :
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
 * ��Կ��ɢ
 * �Ƶ�DK��벿�ֵķ����ǣ�
 * 1������ɢ���ݵ�����8���ֽ���Ϊ�������ݣ�
 * 2����MK��Ϊ������Կ��
 * 3����MK���������ݽ���3DES���㣻
 * �Ƶ�DK�Ұ벿�ֵķ����ǣ�
 4������ɢ���ݵ�����8���ֽ��󷴣���Ϊ�������ݣ�
 5����MK��Ϊ������Կ��
 6����MK���������ݽ���3DES���㣻
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
