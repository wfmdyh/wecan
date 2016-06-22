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
	
	//测试数据
	DevCommand cmdTemp;
	cmdTemp.SetDeviceID(153000);
	cmdTemp.SetPressureRange(3);
	//根据参数生成指令
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
	//查找
	map<DWORD, DevCommand>::iterator ite = m_CommandOrder.find(dwID);
	if (ite != m_CommandOrder.end())
	{
		cmd = ite->second;
		//删除这条指令
		m_CommandOrder.erase(ite);
		LeaveCriticalSection(&m_csCommand);
		return TRUE;
	}
	else{
		//生成不带参数的指令
		cmd.GetCommand(DevCommand::CommandType::NO_PARAM, devType);
	}
	LeaveCriticalSection(&m_csCommand);
	return FALSE;
}