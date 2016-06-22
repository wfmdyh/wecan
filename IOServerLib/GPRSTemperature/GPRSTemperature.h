#pragma once
/************************************************************************
  ���ߣ�wfm
  ���ڣ�2015.05.29
  ���ܣ�Э�������
************************************************************************/

//�Ĵ�������
#define REG_SRV_ADD			"��������ַ"
#define REG_SRV_PORT		"�������˿�"
#define REG_TCNT			"���Ӽ��"
#define REG_TCOM			"ͨѶ���"
#define REG_Temperature		"�¶�ֵ"
#define REG_HUMI			"ʪ��ֵ"
#define REG_LOWPOWER		"��ص͵���"
#define REG_CCID			"CCID"
#define REG_CSQ				"�ź�ǿ��"

//�Ĵ�������
#define REG_COUNT 9
enum REG_INDEX
{
	RI_SRV_ADD = 0,
	RI_SRV_PORT,
	RI_TCNT,
	RI_TCOM,
	RI_Temperature,		//�¶�ֵ
	RI_HUMI,			//ʪ��
	RI_LOWPOWER,
	RI_CCID,
	RI_CSQ
};

class CGPRSTemperature {

public:
	//��׼ֵ
	VARIANT m_values[REG_COUNT];

private:
	string m_DeviceID;

public:
	CGPRSTemperature(void);
	~CGPRSTemperature(void);
	
	//�����ݸ��³�devGPRS������
	void UpdateData(CGPRSTemperature devGPRS);

	void SetDeviceID(string strID);
	string GetDeviceID();
};
