/*
����쳣��Ϣ
*/
#pragma once
class DevBigMeterException
{
public:
	wstring m_DeviceID;
	SYSTEMTIME m_Time;
	BYTE m_PressureValue;
	BYTE m_Status;
public:
	DevBigMeterException();
	~DevBigMeterException();
};

