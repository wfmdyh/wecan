/*
指令格式
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
	BYTE m_DeviceID[4];					//设备ID
	BYTE m_PressureUpper;				//压力上限
	BYTE m_PressureFloor;				//压力下限
	BYTE m_MeasureCycle;				//压力测量周期5, 10, 15
	BYTE m_PressureRange;				//压力量程1 2 3
	BYTE m_UploadRate;					//GPRS上传频率1 2
	BYTE m_UploadTime1[2];				//GPRS上传时间 时、分2个字节的整数表示
	BYTE m_UploadTime2[2];
	BYTE m_TelephoneNumber[11];			//压力故障备用电话号码 11个有效数值
	BYTE m_Time[6];						//时间校准信息 年、月、日、时、分、秒

	vector<BYTE> m_vecCommand;			//指令序列的16进制形式
public:
	DevCommand();
	~DevCommand();
	void operator=(DevCommand &cmd);

	//设备ID
	void SetDeviceID(DWORD dwID);
	//设置压力上限0 - 1.6MPa
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
	//设置时间校准
	void SetTime();
	//生成大表指令
	void GenerateBMCommand(CommandType cType);
	//集中器下发指令
	void GenerateConCommand(CommandType cType);
};

