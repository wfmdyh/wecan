/*
������·�ָ�����
*/
#pragma once
class DevMultipleParamCommand
{
public:
	SYSTEMTIME m_time;					//ʱ��У׼
	BYTE m_PressureUpperLimit;			//ѹ������
	BYTE m_PressureLowerLimit;			//ѹ������
	BYTE m_MeasuringPeriodical;			//ѹ����������
	BYTE m_PressureRange;				//ѹ������
	BYTE m_GPRSUploadRate;				//GPRS�ϴ�Ƶ��
	SYSTEMTIME m_GPRSUploadTime1;		//GPRS�ϴ�ʱ��1
	SYSTEMTIME m_GPRSUploadTime2;		//GPRS�ϴ�ʱ��2
	BYTE m_PhoneNumber[11];				//ѹ�����ϱ��õ绰����
	WORD m_GPRSSendRate;				//GPRS����ʱ����
	BYTE m_Flag[16];					//����ϵ��

public:
	DevMultipleParamCommand();
	~DevMultipleParamCommand();
};

