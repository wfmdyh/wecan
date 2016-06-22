#include "stdafx.h"
#include "DevCommand.h"
#include "CommandMgr.h"


CommandMgr::CommandMgr()
{
}


CommandMgr::~CommandMgr()
{
}

void CommandMgr::GetCommandByID(DWORD dwID, DevCommand& cmd)
{
	wstring strID;
	wstringstream wss;
	wss << dwID;
	strID = wss.str();
	map<wstring, DevCommand>::iterator ite = m_ArrCommand.find(strID);
	if (ite != m_ArrCommand.end())
	{
		//带参数
		cmd = ite->second;
	}
	else{
		//不带参数
		cmd.SetDeviceID(dwID);
		cmd.GenerateNOPARAMCMD();
	}

}
