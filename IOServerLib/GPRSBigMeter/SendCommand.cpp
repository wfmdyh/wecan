#include "stdafx.h"
#include "SendCommand.h"


SendCommand::SendCommand()
{
	InitializeCriticalSection(&m_csCommand);
}


SendCommand::~SendCommand()
{
	DeleteCriticalSection(&m_csCommand);
}

BOOL SendCommand::CreateOrder()
{
	
	//��������
	DevCommand cmdTemp;
	cmdTemp.SetDeviceID(153000);
	cmdTemp.SetPressureRange(3);
	//���ݲ�������ָ��
	cmdTemp.GetCommand(DevCommand::CommandType::PARAM, DEVICE_TYPE::TYPE_BM);

	EnterCriticalSection(&m_csCommand);
	m_CommandOrder[153000] = cmdTemp;
	LeaveCriticalSection(&m_csCommand);

	return TRUE;
}

BOOL SendCommand::GetOrderById(DWORD dwID, DevCommand &cmd, DEVICE_TYPE devType)
{
	EnterCriticalSection(&m_csCommand);
	cmd.SetDeviceID(dwID);
	//����
	map<DWORD, DevCommand>::iterator ite = m_CommandOrder.find(dwID);
	if (ite != m_CommandOrder.end())
	{
		cmd = ite->second;
		//ɾ������ָ��
		m_CommandOrder.erase(ite);
		LeaveCriticalSection(&m_csCommand);
		return TRUE;
	}
	else{
		//���ɲ���������ָ��
		cmd.GetCommand(DevCommand::CommandType::NO_PARAM, devType);
	}
	LeaveCriticalSection(&m_csCommand);
	return FALSE;
}