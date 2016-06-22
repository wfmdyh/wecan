/*
集中器上传上来的大表数据对象
*/
#pragma once
class DevConBigMeter
{
public:
	wstring m_DeviceID;
	//表条目数据
	SYSTEMTIME m_Time;			//年月日时
	BYTE m_Status;
	double m_TotalFlow;
	float m_MaxFlow;
	SYSTEMTIME m_MaxFlowTime;
	float m_MinFlow;
	SYSTEMTIME m_MinFlowTime;
	wstring m_PressureData;					//压力或者温度数据，根据状态 2选1
	wstring m_TemperatureData;
public:
	DevConBigMeter();
	~DevConBigMeter();
};

