// CPage.cpp : CCPage 的实现
#include "stdafx.h"
#include "CPage.h"
#include <comutil.h>
#include "dog_api.h"
#include "dog_vcode.h"

dog_status_t   status;
dog_handle_t   handle;
dog_size_t     fsize;
bool isFlag;

typedef int(__stdcall* InitSerialPort)(int prot, long baud);
typedef int(__stdcall* ReadT55x7)(int* len, byte* buff);
typedef int(__stdcall* ResetCommd)();
typedef int(__stdcall* GetBeep)(short id, byte t);
typedef int(__stdcall* StandardWrite)(byte pageNo, byte isLock, byte* data, byte block); //页号，是否锁定默认为0，数据，块号


InitSerialPort initcom;
ReadT55x7 readT55x7;
ResetCommd resetCommd;
GetBeep getBeep;

StandardWrite standardWrite;

// CCPage

STDMETHODIMP CCPage::Connect(BSTR m_IP, BSTR m_Port)
{
	// TODO: 在此添加实现代码
	
	return S_OK;
}

STDMETHODIMP CCPage::DisConnect(void)
{
	// TODO: 在此添加实现代码
	
	KillTimer(m_nTimer);
	
	return S_OK;
}

LRESULT CCPage::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	
	status = dog_login(0, (dog_vendor_code_t *)vendor_code, &handle);
	if (status != DOG_STATUS_OK)
	{
		if (isFlag)
		{
			isFlag = false;
			BSTR bstrText;
			bstrText = ::SysAllocString(L"1");
			Fire_DisplayMsgOnBrowser(bstrText);
			
		}
		
	}
	else
	{
		if (!isFlag)
		{
			isFlag = true;
			BSTR bstrText;
			bstrText = ::SysAllocString(L"0");
			Fire_DisplayMsgOnBrowser(bstrText);
		}
	}
	
	return 0;
}


STDMETHODIMP CCPage::GetDogCode(BSTR* pResult)
{
	// TODO:  在此添加实现代码
	
	isFlag = true;
	m_nTimer = SetTimer(0, 1000);//设置1秒钟一次

	char membuffer[128] = {0};
	char Temp[128] = { 0 };
	status = dog_login(0, (dog_vendor_code_t *)vendor_code, &handle);

	
	if (status != DOG_STATUS_OK)
	{
		*pResult = ::SysAllocString(L"NOT FIND DOG");
		return S_FALSE;
	}

	status = dog_get_size(handle,
		DOG_FILEID_RW,
		&fsize);

	if (status != DOG_STATUS_OK)
	{
		*pResult = ::SysAllocString(L"读取运行狗数据空间失败.....");
		return S_FALSE;
	}

	status = dog_read(handle,
		DOG_FILEID_RW,     /* file ID */
		0,                 /* offset */
		fsize,             /* length */
		&membuffer[0]);    /* file data */

	if (status != DOG_STATUS_OK)
	{
		*pResult = ::SysAllocString(L"");
		return S_FALSE;

	}
	/*status = dog_decrypt(handle, membuffer, strlen(membuffer));
	if (status != DOG_STATUS_OK)
	{
		*pResult = ::SysAllocString(L"");
		return FALSE;
	}*/
	/*MessageBoxA(NULL, membuffer, NULL, MB_OK);
	int len = membuffer[17] - '0';
	if (membuffer[18] != '=')
	{
		len = (10 * len) + (membuffer[18] - '0');
	}
	MessageBoxA(NULL, membuffer, NULL, MB_OK);
	int index = 0;
	for (int i = 16 - len; i < strlen(membuffer); i++)
	{
		Temp[index++] = membuffer[i];
		if (index == len)
		{
			break;
		}
	}*/
	*pResult = _com_util::ConvertStringToBSTR(membuffer);
	return S_OK;
}

STDMETHODIMP CCPage::GetCardCode(LONG ComNumber, LONG Baud, BSTR* pResult)
{
	// TODO:  在此添加实现代码

	byte buff[28] = { 0 };
	int len = 0;

	HMODULE m_hLib = ::LoadLibrary(L"MasterRD.dll");
	if (m_hLib == NULL)
	{
    	*pResult = ::SysAllocString(L"Load MasterRD.dll ERROR!");
		return S_FALSE;
	}

	initcom = (InitSerialPort)::GetProcAddress(m_hLib, "rf_init_com");
	readT55x7 = (ReadT55x7)::GetProcAddress(m_hLib, "T55x7_Read");
	resetCommd = (ResetCommd)::GetProcAddress(m_hLib, "Reset_Command");
	getBeep = (GetBeep)::GetProcAddress(m_hLib, "rf_beep");

	int i = (*initcom)(ComNumber, Baud);
	if (i == 0)
	{
		i = (*resetCommd)();
		if (i == 0)
		{
			i = (*readT55x7)(&len, buff);
			if (i == 0)
			{
				(*getBeep)(0, 10);

				byte key = buff[0];
				int index = 0;
				char str[28] = { 0 };
				for (int i = 2; i < 28; i++)
				{
					if (buff[i] != key)
					{
						buff[i - 2] = buff[i];
						index++;
					}
					else
					{
						break;
					}

				}

				memcpy(str, buff, index);

				*pResult = _com_util::ConvertStringToBSTR(str);

			}
			else
			{
				*pResult = ::SysAllocString(L"Read Failed");
			}
		}
		else
		{
			*pResult = ::SysAllocString(L"Reset Failed!");
		}

	}
	else
	{
		*pResult = ::SysAllocString(L"Initialization Failed!");
	}
	

	return S_OK;
}


STDMETHODIMP CCPage::SetCardCode(LONG ComNumber, LONG Baud, BSTR Code, BSTR* pResult)
{
	// TODO:  在此添加实现代码

	HMODULE m_hLib = ::LoadLibrary(L"MasterRD.dll");
	if (m_hLib == NULL)
	{
		*pResult = ::SysAllocString(L"Load MasterRD.dll ERROR!");
		return S_FALSE;
	}

	initcom = (InitSerialPort)::GetProcAddress(m_hLib, "rf_init_com");
	standardWrite = (StandardWrite)::GetProcAddress(m_hLib, "Standard_Write");

	int i = (*initcom)(ComNumber, Baud);
	if (i == 0)
	{
		byte buff[28] = { 0 };

		int kh = 1;

		SYSTEMTIME sys;
		GetLocalTime(&sys); //获取系统日期
		int y = sys.wYear;  //获取年份
		int m = sys.wMonth;   //获取当前月份

		byte CRC = 0;

		byte key = ~(y % 100 + m) & 0xff;  //协议key

		buff[0] = key;

		byte type = key ^ 3;   //卡类型 3为序列号卡
		buff[1] = type;

		CRC += type;

		_bstr_t b = Code;
		char *ch = b;

		int len =strlen(ch);

		int index = 0;
		for (int i = 0; i < len; i++)
		{
			buff[2 + i] = ch[i];
			index = 2 + i;
			CRC += ch[i];
		}


		for (int i = index + 1; i < 27; i++)
		{
			buff[i] = key;
		}

		buff[27] = CRC % 256;

		byte temp[4] = { 0 };

		index = 0;

		for (size_t i = 0; i < 28; i++)
		{
			temp[index++] = buff[i];
			if ((i + 1) % 4 == 0)
			{
				int s = (*standardWrite)(2, 0, temp, kh++);
				index = 0;
			}
		}
	}

	return S_OK;
}
