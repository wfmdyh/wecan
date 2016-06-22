#include "stdafx.h"
#include "DevPressureAlarm.h"


DevPressureAlarm::DevPressureAlarm()
{
	m_DeviceID = 0;
	
	GetLocalTime(&m_Time);
	m_Time.wHour = 0;
	m_Time.wMinute = 0;
	m_Time.wSecond = 0;
	m_Time.wMilliseconds = 0;
	m_Pressure = 0;
	m_State = 0;
}


DevPressureAlarm::~DevPressureAlarm()
{
}

void DevPressureAlarm::SetDeviceID(DWORD dwID)
{
	m_DeviceID = dwID;
	wstringstream wss;
	wss << m_DeviceID;
	m_strDeviceID = wss.str();
}

DWORD DevPressureAlarm::GetDeviceID()
{
	return m_DeviceID;
}

wstring DevPressureAlarm::GetDeviceIDStr()
{
	return m_strDeviceID;
}