#include "stdafx.h"
#include "DevConcentrator.h"


DevConcentrator::DevConcentrator()
{
	m_SuccessRate = 0;
	m_Date = { 0 };
	m_Status1 = 0;
	m_Status2 = 0;
	m_DeviceID = 0;
}


DevConcentrator::~DevConcentrator()
{
}

wstring DevConcentrator::GetDeviceIDStr()
{
	wstringstream wss;
	wss << m_DeviceID;
	return wss.str();
}

wstring DevConcentrator::GetDateStr()
{
	if (m_Date.wYear == 0)
	{
		GetLocalTime(&m_Date);
	}
	wstringstream wss;
	wss << m_Date.wYear;
	wss << L"-";
	wss << m_Date.wMonth;
	wss << L"-";
	wss << m_Date.wDay;
	return wss.str();
}

void DevConcentrator::SetLastTime(time_t t)
{
	m_LastTime = t;
}

time_t DevConcentrator::GetLastTime()
{
	return m_LastTime;
}