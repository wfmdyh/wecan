#pragma once
/************************************************************************
���ߣ�wfm
���ڣ�2015.05.29
���ܣ�Э�������
************************************************************************/

//�Ĵ�������
//#define REG_SRV_ADD			"��������ַ"
//#define REG_SRV_PORT		"�������˿�"
//#define REG_TCNT			"���Ӽ��"
//#define REG_TCOM			"ͨѶ���"
//#define REG_Temperature		"�¶�ֵ"
//#define REG_HUMI			"ʪ��ֵ"
//#define REG_LOWPOWER		"��ص͵���"
//#define REG_CCID			"CCID"
//#define REG_CSQ				"�ź�ǿ��"

//�Ĵ�������
const static int REG_COUNT = 14;

enum REG_INDEX
{
	RI_CURRENT_ALARM = 0,				//��ǰ�澯״̬
	RI_CURRENT_TOTAL_FLOW,				//��ǰ�ۼ�����
	RI_CURRENT_PRESSURE,				//��ǰѹ��
	RI_REAL_TIME_ALARM,					//ʵʱѹ���澯
	RI_MAXIMUM_INSTANTANEOUS,			//ǰһ�����˲ʱ����
	RI_MAX_I_HOUR,						//ǰһ�����˲ʱ������������ʱ
	RI_MAX_I_MINUTE,					//ǰһ�����˲ʱ�����������ķ�
	RI_MINIMUM_INSTANTANEOUS,			//ǰһ����С˲ʱ����
	RI_MIN_I_HOUR,						//ǰһ����С˲ʱ������������ʱ
	RI_MIN_I_MINUTE,					//ǰһ����С˲ʱ�����������ķ�
	RI_THE_DAY_BEFORE_DATE,				//ǰһ������ ������
	RI_COMMUNICATION_SUCCESS_RATE,		//��ǰͨѶ�ɹ���
	RI_ALL_PRESSURE,					//ǰһ������ѹ�����ݣ�����
	RI_ALL_TEMPERATURE					//ǰһ�������¶� ,����
};

class CGPRSBigMeter {

public:
	//��׼ֵ
	VARIANT m_values[REG_COUNT];
private:
	string m_DeviceID;
	//���һ�����ݲɼ�ʱ��
	time_t m_LastTime;
public:
	CGPRSBigMeter(void);
	~CGPRSBigMeter(void);

	void SetDeviceID(string strID);
	string GetDeviceID();

	void SetLastTime(time_t t);
	time_t GetLastTime();
	void operator=(CGPRSBigMeter &Meter);
};
