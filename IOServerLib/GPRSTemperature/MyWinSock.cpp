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

// 在主界面中显示提示信息
void MyWinSock::_ShowMessage(const char *szFormat, ...) const
{
	// 根据传入的参数格式化字符串
	char   strMessage[1024] = { 0 };
	va_list   arglist;

	// 处理变长参数
	va_start(arglist, szFormat);
	//strMessage.FormatV(szFormat,arglist);
	int iLen = _vscprintf(szFormat, arglist) + 1;	//得到格式化以后的字符串长度
	if (iLen > 0)
	{
		_vsnprintf_s(strMessage, iLen, szFormat, arglist);
	}
	va_end(arglist);

	// 在主界面中显示
	//printf(strMessage);
	OutputDebugStringA(strMessage);
	OutputDebugStringA("\n");
}