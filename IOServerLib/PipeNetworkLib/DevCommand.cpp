#include "stdafx.h"
#include "DevCommand.h"


DevCommand::DevCommand()
{
	m_Type = CommandType::NO_PARAM;					//参数类型
	memset(m_DeviceID, 0, 4);
	memset(m_Time, 0, 6);
}


DevCommand::~DevCommand()
{
}

void DevCommand::SetTime()
{
	//当前时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_Time[0] = (BYTE)(st.wYear - 2000);
	m_Time[1] = (BYTE)st.wMonth;
	m_Time[2] = (BYTE)st.wDay;
	m_Time[3] = (BYTE)st.wHour;
	m_Time[4] = (BYTE)st.wMinute;
	m_Time[5] = (BYTE)st.wSecond;
}

void DevCommand::GenerateNOPARAMCMD()
{
	//68 00 00 68 08 01 81 01 02 03 04 FF 00 00 00 00 00 00 CS 16
	m_vecCommand.clear();
	m_vecCommand.swap(vector<BYTE>());
	m_Type = CommandType::NO_PARAM;
	SetTime();

	WORD wPackageLen = 0;
	BYTE *byPL = (BYTE*)&wPackageLen;

	m_vecCommand.push_back(0x68);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x68);
	//协议版本
	m_vecCommand.push_back(0x08);
	m_vecCommand.push_back(0x01);
	//AFN
	//不带参数
	m_vecCommand.push_back(0x81);
	
	//ID
	for (unsigned i = 0; i < 4; i++)
	{
		m_vecCommand.push_back(m_DeviceID[i]);
	}

	//时间校验
	m_vecCommand.push_back(0xFF);
	for (unsigned i = 0; i < 6; i++)
	{
		m_vecCommand.push_back(m_Time[i]);
	}
	//整包长度
	wPackageLen = m_vecCommand.size() + 2;
	m_vecCommand.at(1) = byPL[1];
	m_vecCommand.at(2) = byPL[0];
	//校验和
	BYTE byCS = 0;
	DWORD dwSum = 0;
	for (unsigned i = 0; i < m_vecCommand.size(); i++)
	{
		dwSum += m_vecCommand.at(i);
	}
	byCS = dwSum % 256;
	m_vecCommand.push_back(byCS);
	//包尾标识
	m_vecCommand.push_back(0x16);
}

DevCommand::CommandType DevCommand::GetCmdType()
{
	return m_Type;
}

vector<BYTE> DevCommand::GetHexCommand()
{
	return m_vecCommand;
}

void DevCommand::SetDeviceID(DWORD dwID)
{
	BYTE byID[4] = { 0 };
	memcpy(byID, &dwID, 4);
	m_DeviceID[3] = byID[0];
	m_DeviceID[2] = byID[1];
	m_DeviceID[1] = byID[2];
	m_DeviceID[0] = byID[3];
}