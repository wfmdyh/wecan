#include "stdafx.h"
#include "DevWaterMeter.h"
#include "DevPressureAlarm.h"
#include "DevMultipleParam.h"
#include "DevPipeNet.h"


DevPipeNet::DevPipeNet()
{
	m_AlarmState = 0;					//��ǰ�澯��״̬��־
	m_TotalFlow = -1;					//�ۼ�����
	m_QuantityOfHeat = 0;				//����
	m_PAValue = 0;
	m_PAState = 0xFF;
	m_Flow = 0;						//��ǰ˲ʱ����
	m_Pressure = 0;					//��ǰѹ��
	m_Temperature = 0;				//��ǰ�¶�
	memset(&m_Date, 0, sizeof(SYSTEMTIME));
	m_SucessRate = 0;					//��ǰͨѶ�ɹ���
	memset(&m_CurrentTime, 0, sizeof(SYSTEMTIME));
	m_State = 0;						//ˮ��״̬
	m_SmallState = 0;					//С����״̬
	m_Verify[0] = 0;
	m_Verify[1] = 0;
	m_VerMeasurementID = 0;
	memset(m_VerMeasurementNum, 0, 6);
	//memset(m_VerCommunicationHNum, 0, 2);
	//memset(m_VerCommunicationSNum, 0, 2);
	m_VerCommunicationHNum = 0;
	m_VerCommunicationSNum = 0;
}


DevPipeNet::~DevPipeNet()
{
	vector<DevWaterMeter*>::iterator iteWM = m_ArrMeter.begin();
	for (; iteWM != m_ArrMeter.end(); iteWM++)
	{
		DevWaterMeter* pMeter = *iteWM;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrMeter.clear();
	m_ArrMeter.swap(vector<DevWaterMeter*>());

	vector<DevPressureAlarm*>::iterator itePA = m_ArrPressureAlarm.begin();
	for (; itePA != m_ArrPressureAlarm.end(); itePA++)
	{
		DevPressureAlarm* pMeter = *itePA;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrPressureAlarm.clear();
	m_ArrPressureAlarm.swap(vector<DevPressureAlarm*>());

	;
	vector<DevMultipleParam*>::iterator iteMP = m_ArrMultipleParam.begin();
	for (; iteMP != m_ArrMultipleParam.end(); iteMP++)
	{
		DevMultipleParam* pMeter = *iteMP;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrMultipleParam.clear();
	m_ArrMultipleParam.swap(vector<DevMultipleParam*>());
}

void DevPipeNet::SetUploadTime(SYSTEMTIME* t)
{
	if (t == NULL)
	{
		GetLocalTime(&m_UploadTime);
	}
	else{
		m_UploadTime = *t;
	}
}

SYSTEMTIME DevPipeNet::GetUploadTime()
{
	return m_UploadTime;
}

void DevPipeNet::operator=(DevPipeNet& dev)
{
	m_UploadTime = dev.m_UploadTime;			//���ݵ���ʱ��
	m_DeviceID = dev.m_DeviceID;					//�豸ID
	m_AlarmState = dev.m_AlarmState;					//��ǰ�澯��״̬��־
	m_TotalFlow = dev.m_TotalFlow;					//�ۼ�����
	m_QuantityOfHeat = dev.m_QuantityOfHeat;			//����
	m_PAState = dev.m_PAState;
	m_PAValue = dev.m_PAValue;
	m_ArrTemperature = dev.m_ArrTemperature;		//ǰһ�������¶� 0.1
	m_ArrPressure = dev.m_ArrPressure;			//ǰһ������ѹ������
	m_Flow = dev.m_Flow;						//��ǰ˲ʱ����
	m_Pressure = dev.m_Pressure;					//��ǰѹ��
	m_Temperature = dev.m_Temperature;				//��ǰ�¶�
	m_Date = dev.m_Date;					//ǰһ�������
	m_SucessRate = dev.m_SucessRate;					//��ǰͨѶ�ɹ���
	m_CurrentTime = dev.m_CurrentTime;			//��ǰ����
	m_State = dev.m_State;						//ˮ��״̬
	m_SmallState = dev.m_SmallState;					//С����״̬
	//�������
	vector<DevWaterMeter*>::iterator iteWM = dev.m_ArrMeter.begin();
	for (; iteWM != dev.m_ArrMeter.end(); iteWM++)
	{
		DevWaterMeter* pWM = new DevWaterMeter;
		*pWM = **iteWM;
		m_ArrMeter.push_back(pWM);
	}

	memcpy(m_Verify, dev.m_Verify, 2);
	//ʵʱѹ���澯���䳤��
	vector<DevPressureAlarm*>::iterator itePA = dev.m_ArrPressureAlarm.begin();
	for (; itePA != dev.m_ArrPressureAlarm.end(); itePA++)
	{
		DevPressureAlarm* pPA = new DevPressureAlarm;
		*pPA = **itePA;
		m_ArrPressureAlarm.push_back(pPA);
	}
	//ʵʱ���������
	vector<DevMultipleParam*>::iterator iteMP = dev.m_ArrMultipleParam.begin();
	for (; iteMP != dev.m_ArrMultipleParam.end(); iteMP++)
	{
		DevMultipleParam* pMP = new DevMultipleParam;
		*pMP = **iteMP;
		m_ArrMultipleParam.push_back(pMP);
	}
	//����ģ������汾��
	m_VerMeasurementID = dev.m_VerMeasurementID;
	memcpy(m_VerMeasurementNum, dev.m_VerMeasurementNum, 6);
	//ͨѶģ��汾Ӳ��������汾��
	m_VerCommunicationHNum = dev.m_VerCommunicationHNum;
	m_VerCommunicationSNum = dev.m_VerCommunicationSNum;
}