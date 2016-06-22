/*
小表对象
2016-3-22
wfm
*/
#pragma once
class SmallMeter
{
public:
	DWORD m_DeviceID;
	time_t m_LastTime;
private:
	//wstring m_strDeviceID;
	BYTE m_MeterStatus;
	BYTE m_MinSpeed;
	double m_TotalFlow;
public:
	SmallMeter();
	~SmallMeter();

	//void SetDeviceID(wstring strID);
	wstring GetDeviceIDStr();

	void SetMeterStatus(BYTE bValue);
	BYTE GetMeterStatus();

	void SetMinSpeed(BYTE bValue);
	BYTE GetMinSpeed();

	void SetTotalFlow(double fFlow);
	double GetTotalFlow();
};

