/*
ʵʱѹ���澯
*/
#pragma once
class DevPressureAlarm
{
private:
	DWORD m_DeviceID;
	wstring m_strDeviceID;
public:
	SYSTEMTIME m_Time;
	BYTE m_Pressure;
	BYTE m_State;			//ѹ���澯״̬
public:
	DevPressureAlarm();
	~DevPressureAlarm();

	void SetDeviceID(DWORD dwID);
	DWORD GetDeviceID();
	wstring GetDeviceIDStr();
};

