#pragma once
class CommandMgr
{
private:
	map<wstring, DevCommand> m_ArrCommand;
public:
	CommandMgr();
	~CommandMgr();
	//����ID��ȡָ��
	void GetCommandByID(DWORD dwID, DevCommand& cmd);
};

