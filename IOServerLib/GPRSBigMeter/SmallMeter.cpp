#include "stdafx.h"
#include "SmallMeter.h"


SmallMeter::SmallMeter()
{
	m_MeterStatus = 0;
	m_MinSpeed = 0;
	m_TotalFlow = 0.0f;
	m_DeviceID = 0;
	m_LastTime = 0;
}


SmallMeter::~SmallMeter()
{

}

//void SmallMeter::SetDeviceID(wstring strID)
//{
//
//}

wstring SmallMeter::GetDeviceIDStr()
{
	wstringstream wss;
	wss << m_DeviceID;
	return wss.str();
}

void SmallMeter::SetMeterStatus(BYTE bValue)
{

}

BYTE SmallMeter::GetMeterStatus()
{
	return m_MeterStatus;
}

void SmallMeter::SetMinSpeed(BYTE bValue)
{

}

BYTE SmallMeter::GetMinSpeed()
{
	return m_MinSpeed;
}

void SmallMeter::SetTotalFlow(double fFlow)
{
	m_TotalFlow = fFlow;
}

double SmallMeter::GetTotalFlow()
{
	return m_TotalFlow;
}