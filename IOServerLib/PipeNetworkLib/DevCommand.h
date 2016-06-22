#pragma once
class DevCommand
{
public:
	enum CommandType
	{
		PARAM,
		NO_PARAM
	};
private:
	vector<BYTE> m_vecCommand;			//ָ�����е�16������ʽ
	CommandType m_Type;					//��������
	BYTE m_DeviceID[4];					//�豸ID
	BYTE m_Time[6];						//ʱ��У׼��Ϣ �ꡢ�¡��ա�ʱ���֡���
public:
	DevCommand();
	~DevCommand();
	void SetTime();
	//����һ����������������
	void GenerateNOPARAMCMD();
	CommandType GetCmdType();
	vector<BYTE> GetHexCommand();
	void SetDeviceID(DWORD dwID);
};

