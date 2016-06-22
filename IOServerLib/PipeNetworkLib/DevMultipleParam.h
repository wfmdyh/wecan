/*
实时多参数
*/
#pragma once
class DevMultipleParam
{
private:
	DWORD m_DeviceID;
	wstring m_DeviceIDStr;
public:
	
	BYTE m_State;
	BYTE m_TotalBubble[4];			//累计气泡数
	double m_PositiveTotalFlow;		//正累计流量
	double m_NegativeTotalFlow;		//负累计流量
	float m_Flow;
	SYSTEMTIME m_MaxFlowTime;
	float m_MaxFlow;
	SYSTEMTIME m_MinFlowTime;
	float m_MinFlow;
	BYTE m_Pressure;				//压力 10KPa
	short m_Temperature;			//温度 0.1
	SYSTEMTIME m_DateTime;			//时间
	BYTE m_CommunicationState;		//通讯状态
public:
	DevMultipleParam();
	~DevMultipleParam();

	void SetDeviceID(DWORD dwID);
	DWORD GetDeviceID();
	wstring GetDeviceIDStr();
};

