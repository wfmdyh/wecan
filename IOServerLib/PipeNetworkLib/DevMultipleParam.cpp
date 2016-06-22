#include "stdafx.h"
#include "DevMultipleParam.h"


DevMultipleParam::DevMultipleParam()
{
	m_DeviceID = 0;
	m_State = 0;
	memset(m_TotalBubble, 0, 4);	//�ۼ�������
	m_PositiveTotalFlow = 0;		//���ۼ�����
	m_NegativeTotalFlow = 0;		//���ۼ�����
	m_Flow = 0;
	memset(&m_MaxFlowTime, 0, sizeof(SYSTEMTIME));
	m_MaxFlow = 0;
	memset(&m_MinFlowTime, 0, sizeof(SYSTEMTIME));
	m_MinFlow = 0;
	m_Pressure = 0;					//ѹ�� 10KPa
	m_Temperature = 0;				//�¶� 0.1
	memset(&m_DateTime, 0, sizeof(SYSTEMTIME));		//ʱ��
	m_CommunicationState = 0;		//ͨѶ״̬
}


DevMultipleParam::~DevMultipleParam()
{
}


void DevMultipleParam::SetDeviceID(DWORD dwID)
{
	m_DeviceID = dwID;
	wstringstream wss;
	wss << m_DeviceID;
	m_DeviceIDStr = wss.str();
}

DWORD DevMultipleParam::GetDeviceID()
{
	return m_DeviceID;
}

wstring DevMultipleParam::GetDeviceIDStr()
{
	return m_DeviceIDStr;
}