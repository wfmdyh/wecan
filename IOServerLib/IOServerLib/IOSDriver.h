/*
驱动结构体
时间：2015-12-08
*/
#pragma once
class CIOSDriver
{
public:
	enum IOSDTYPE
	{
		TYPE_COL,		//采集通道
		TYPE_TRA		//转发通道
	};
private:
	string m_strName;			//驱动名称，动态库全路径
	HMODULE m_hMod;				//驱动句柄
	int m_nType;				//驱动类型
public:
	CIOSDriver();
	~CIOSDriver();

	string GetName();
	HMODULE GetHandle();
	void SetName(string strName);
	void SetHandle(HMODULE hMod);
	void SetType(IOSDTYPE type);
	int GetType();
};

