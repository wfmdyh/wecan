/*
�������ϴ������Ĵ�����ݶ���
*/
#pragma once
class DevConBigMeter
{
public:
	wstring m_DeviceID;
	//����Ŀ����
	SYSTEMTIME m_Time;			//������ʱ
	BYTE m_Status;
	double m_TotalFlow;
	float m_MaxFlow;
	SYSTEMTIME m_MaxFlowTime;
	float m_MinFlow;
	SYSTEMTIME m_MinFlowTime;
	wstring m_PressureData;					//ѹ�������¶����ݣ�����״̬ 2ѡ1
	wstring m_TemperatureData;
public:
	DevConBigMeter();
	~DevConBigMeter();
};

