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
	vector<BYTE> m_vecCommand;			//指令序列的16进制形式
	CommandType m_Type;					//参数类型
	BYTE m_DeviceID[4];					//设备ID
	BYTE m_Time[6];						//时间校准信息 年、月、日、时、分、秒
public:
	DevCommand();
	~DevCommand();
	void SetTime();
	//生成一条不带参数的命令
	void GenerateNOPARAMCMD();
	CommandType GetCmdType();
	vector<BYTE> GetHexCommand();
	void SetDeviceID(DWORD dwID);
};

