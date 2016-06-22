/*
ʵʱ�����
*/
#pragma once
class DevMultipleParam
{
private:
	DWORD m_DeviceID;
	wstring m_DeviceIDStr;
public:
	
	BYTE m_State;
	BYTE m_TotalBubble[4];			//�ۼ�������
	double m_PositiveTotalFlow;		//���ۼ�����
	double m_NegativeTotalFlow;		//���ۼ�����
	float m_Flow;
	SYSTEMTIME m_MaxFlowTime;
	float m_MaxFlow;
	SYSTEMTIME m_MinFlowTime;
	float m_MinFlow;
	BYTE m_Pressure;				//ѹ�� 10KPa
	short m_Temperature;			//�¶� 0.1
	SYSTEMTIME m_DateTime;			//ʱ��
	BYTE m_CommunicationState;		//ͨѶ״̬
public:
	DevMultipleParam();
	~DevMultipleParam();

	void SetDeviceID(DWORD dwID);
	DWORD GetDeviceID();
	wstring GetDeviceIDStr();
};

