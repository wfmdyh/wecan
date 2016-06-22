/*
ָ���ʽ
*/
#pragma once
class SendCommand;

class DevCommand
{
public:
	enum CommandType
	{
		PARAM,
		NO_PARAM
	};
private:
	BYTE m_DeviceID[4];					//�豸ID
	BYTE m_PressureUpper;				//ѹ������
	BYTE m_PressureFloor;				//ѹ������
	BYTE m_MeasureCycle;				//ѹ����������5, 10, 15
	BYTE m_PressureRange;				//ѹ������1 2 3
	BYTE m_UploadRate;					//GPRS�ϴ�Ƶ��1 2
	BYTE m_UploadTime1[2];				//GPRS�ϴ�ʱ�� ʱ����2���ֽڵ�������ʾ
	BYTE m_UploadTime2[2];
	BYTE m_TelephoneNumber[11];			//ѹ�����ϱ��õ绰���� 11����Ч��ֵ
	BYTE m_Time[6];						//ʱ��У׼��Ϣ �ꡢ�¡��ա�ʱ���֡���

	vector<BYTE> m_vecCommand;			//ָ�����е�16������ʽ
public:
	DevCommand();
	~DevCommand();
	void operator=(DevCommand &cmd);

	//�豸ID
	void SetDeviceID(DWORD dwID);
	//����ѹ������0 - 1.6MPa
	BOOL SetPressureUpper(float fMPa);
	BOOL SetPressureFloor(float fMPa);

	BOOL SetMeasureCycle(int nCycle);
	BOOL SetPressureRange(int nRange);

	BOOL SetUploadRate(int nRate);
	BOOL SetUploadTime1(WORD wHour, WORD wMinute);
	BOOL SetUploadTime2(WORD wHour, WORD wMinute);
	BOOL SetTelephoneNumber(DWORD dwPhoneNumber);
	
	BOOL GetCommand(CommandType cType, SendCommand::DEVICE_TYPE devType);
	vector<BYTE> GetHexCommand();
private:
	//����ʱ��У׼
	void SetTime();
	//���ɴ��ָ��
	void GenerateBMCommand(CommandType cType);
	//�������·�ָ��
	void GenerateConCommand(CommandType cType);
};

