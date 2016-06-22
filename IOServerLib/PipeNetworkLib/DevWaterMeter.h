#pragma once
class DevWaterMeter
{
public:
	wstring m_DeviceID;
	BYTE m_State;				//水表状态
	double m_TotalFlow;			//累计流量
	double m_HeatQuantity;		//热量
	float m_Flow;				//瞬时流量
	float m_Pressure;			//压力
	float m_Temperature;		//温度
	BYTE m_RFState;				//RF状态
public:
	DevWaterMeter();
	~DevWaterMeter();
};

