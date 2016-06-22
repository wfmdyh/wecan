#pragma once
class CommandMgr
{
private:
	map<wstring, DevCommand> m_ArrCommand;
public:
	CommandMgr();
	~CommandMgr();
	//根据ID获取指令
	void GetCommandByID(DWORD dwID, DevCommand& cmd);
};

