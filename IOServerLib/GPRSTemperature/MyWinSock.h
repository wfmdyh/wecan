#pragma once
class MyWinSock
{
public:
	BOOL LoadWinsock();
	// ж��Socket�⣬��������
	void UnloadSocketLib() { WSACleanup(); }
	// ������������ʾ��Ϣ
	void _ShowMessage(const char *szFormat, ...) const;
	MyWinSock();
	~MyWinSock();
};

