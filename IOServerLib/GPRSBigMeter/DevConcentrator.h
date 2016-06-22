/*
集中器对象
2016-3-22
wfm
*/
#pragma once

class SmallMeter;
class DevCollector;
class DevRepeater;
class DevConBigMeter;
class DevBigMeterException;

class DevConcentrator
{
public:
	DWORD m_DeviceID;

	BYTE m_SuccessRate;
	vector<SmallMeter> m_vecSmallMeters;
	SYSTEMTIME m_Date;						//年月日
	vector<DevCollector> m_vecCollectors;
	vector<DevRepeater> m_vecRepeaters;
	BYTE m_Status1;							//集中器状态
	BYTE m_Status2;
	vector<DevConBigMeter> m_vecBigMeters;	//大表数据
	vector<DevBigMeterException> m_vecBigExceptions;

private:
	time_t m_LastTime;
public:
	DevConcentrator();
	~DevConcentrator();

	wstring GetDeviceIDStr();
	wstring GetDateStr();

	void SetLastTime(time_t t);
	time_t GetLastTime();
};

