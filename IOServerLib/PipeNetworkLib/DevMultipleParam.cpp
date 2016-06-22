#include "stdafx.h"
#include "DevMultipleParam.h"


DevMultipleParam::DevMultipleParam()
{
	m_DeviceID = 0;
	m_State = 0;
	memset(m_TotalBubble, 0, 4);	//累计气泡数
	m_PositiveTotalFlow = 0;		//正累计流量
	m_NegativeTotalFlow = 0;		//负累计流量
	m_Flow = 0;
	memset(&m_MaxFlowTime, 0, sizeof(SYSTEMTIME));
	m_MaxFlow = 0;
	memset(&m_MinFlowTime, 0, sizeof(SYSTEMTIME));
	m_MinFlow = 0;
	m_Pressure = 0;					//压力 10KPa
	m_Temperature = 0;				//温度 0.1
	memset(&m_DateTime, 0, sizeof(SYSTEMTIME));		//时间
	m_CommunicationState = 0;		//通讯状态
}


DevMultipleParam::~DevMultipleParam()
{
}


void DevMultipleParam::SetDeviceID(DWORD dwID)
{
	m_DeviceID = dwID;
	wstringstream wss;
	wss << m_DeviceID;
	m_DeviceIDStr = wss.str();
}

DWORD DevMultipleParam::GetDeviceID()
{
	return m_DeviceID;
}

wstring DevMultipleParam::GetDeviceIDStr()
{
	return m_DeviceIDStr;
}