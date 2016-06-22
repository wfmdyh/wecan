#pragma once
class MyWinSock
{
public:
	BOOL LoadWinsock();
	// 卸载Socket库，彻底完事
	void UnloadSocketLib() { WSACleanup(); }
	// 在主界面中显示信息
	void _ShowMessage(const char *szFormat, ...) const;
	MyWinSock();
	~MyWinSock();
};

