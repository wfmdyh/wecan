#pragma once
class DevWaterMeter
{
public:
	wstring m_DeviceID;
	BYTE m_State;				//ˮ��״̬
	double m_TotalFlow;			//�ۼ�����
	double m_HeatQuantity;		//����
	float m_Flow;				//˲ʱ����
	float m_Pressure;			//ѹ��
	float m_Temperature;		//�¶�
	BYTE m_RFState;				//RF״̬
public:
	DevWaterMeter();
	~DevWaterMeter();
};

