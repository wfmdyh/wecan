#pragma once
class DevWaterMeter;
class DevPressureAlarm;
class DevMultipleParam;

class DevPipeNet
{
private:
	SYSTEMTIME m_UploadTime;			//���ݵ���ʱ��
public:
	wstring m_DeviceID;					//�豸ID

	BYTE m_AlarmState;					//��ǰ�澯��״̬��־
	double m_TotalFlow;					//�ۼ�����
	double m_QuantityOfHeat;			//����
	WORD m_PAValue;						//ʵʱѹ���澯 ѹ��ֵ
	BYTE m_PAState;						//ʵʱѹ���澯 ״̬
	
	vector<WORD> m_ArrTemperature;		//ǰһ�������¶� 0.1
	vector<WORD> m_ArrPressure;			//ǰһ������ѹ������
	float m_Flow;						//��ǰ˲ʱ����
	float m_Pressure;					//��ǰѹ��
	float m_Temperature;				//��ǰ�¶�
	SYSTEMTIME m_Date;					//ǰһ�������
	BYTE m_SucessRate;					//��ǰͨѶ�ɹ���
	SYSTEMTIME m_CurrentTime;			//��ǰ����
	BYTE m_State;						//ˮ��״̬
	BYTE m_SmallState;					//С����״̬
	vector<DevWaterMeter*> m_ArrMeter;	//�������
	DWORD m_Verify[2];					//У��ϵ��
	vector<DevPressureAlarm*> m_ArrPressureAlarm;	//ʵʱѹ���澯���䳤��
	vector<DevMultipleParam*> m_ArrMultipleParam;	//ʵʱ���������
	//����ģ������汾��
	DWORD m_VerMeasurementID;
	BYTE m_VerMeasurementNum[6];
	//ͨѶģ��汾Ӳ��������汾��
	WORD m_VerCommunicationHNum;
	WORD m_VerCommunicationSNum;
public:
	DevPipeNet();
	~DevPipeNet();

	void SetUploadTime(SYSTEMTIME* t);
	SYSTEMTIME GetUploadTime();
	void operator=(DevPipeNet &dev);
};

