/*
多参数下发指令对象
*/
#pragma once
class DevMultipleParamCommand
{
public:
	SYSTEMTIME m_time;					//时间校准
	BYTE m_PressureUpperLimit;			//压力上限
	BYTE m_PressureLowerLimit;			//压力下限
	BYTE m_MeasuringPeriodical;			//压力测量周期
	BYTE m_PressureRange;				//压力量程
	BYTE m_GPRSUploadRate;				//GPRS上传频率
	SYSTEMTIME m_GPRSUploadTime1;		//GPRS上传时间1
	SYSTEMTIME m_GPRSUploadTime2;		//GPRS上传时间2
	BYTE m_PhoneNumber[11];				//压力故障备用电话号码
	WORD m_GPRSSendRate;				//GPRS发送时间间隔
	BYTE m_Flag[16];					//修正系数

public:
	DevMultipleParamCommand();
	~DevMultipleParamCommand();
};

