/*
ָ���·���
*/
#pragma once

class DevCommand;
class SendCommand
{
private:
	map<DWORD, DevCommand> m_CommandOrder;				//ָ�����
	CRITICAL_SECTION m_csCommand;					//�̻߳���
public:
	enum DEVICE_TYPE
	{
		TYPE_BM,		//���
		TYPE_CON		//������
	};
public:
	SendCommand();
	~SendCommand();
	//����ָ�����
	BOOL CreateOrder();
	//��ȡ��ӦID���豸ָ��ҵ�����TRUE��ָ�����cmd�С����û���ҵ��·�ָ�Ĭ�Ϸ��ز���������ָ��
	BOOL GetOrderById(DWORD dwID, DevCommand &cmd, DEVICE_TYPE devType);
};

