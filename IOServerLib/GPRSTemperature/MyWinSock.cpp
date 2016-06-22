#include "stdafx.h"
#include "MyWinSock.h"


MyWinSock::MyWinSock()
{

}


MyWinSock::~MyWinSock()
{

}

BOOL MyWinSock::LoadWinsock()
{
	WSADATA wsaData;
	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		this->_ShowMessage("WSAStartup failed with error: %d\n", iResult);
		//printf("WSAStartup failed with error: %d\n", iResult);
		return FALSE;
	}
	return TRUE;
}

// ������������ʾ��ʾ��Ϣ
void MyWinSock::_ShowMessage(const char *szFormat, ...) const
{
	// ���ݴ���Ĳ�����ʽ���ַ���
	char   strMessage[1024] = { 0 };
	va_list   arglist;

	// ����䳤����
	va_start(arglist, szFormat);
	//strMessage.FormatV(szFormat,arglist);
	int iLen = _vscprintf(szFormat, arglist) + 1;	//�õ���ʽ���Ժ���ַ�������
	if (iLen > 0)
	{
		_vsnprintf_s(strMessage, iLen, szFormat, arglist);
	}
	va_end(arglist);

	// ������������ʾ
	//printf(strMessage);
	OutputDebugStringA(strMessage);
	OutputDebugStringA("\n");
}