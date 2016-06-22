#pragma once
/************************************************************************
作者：wfm
日期：2015.05.29
功能：协议解析类
************************************************************************/

//寄存器名称
//#define REG_SRV_ADD			"服务器地址"
//#define REG_SRV_PORT		"服务器端口"
//#define REG_TCNT			"连接间隔"
//#define REG_TCOM			"通讯间隔"
//#define REG_Temperature		"温度值"
//#define REG_HUMI			"湿度值"
//#define REG_LOWPOWER		"电池低电量"
//#define REG_CCID			"CCID"
//#define REG_CSQ				"信号强度"

//寄存器个数
const static int REG_COUNT = 14;

enum REG_INDEX
{
	RI_CURRENT_ALARM = 0,				//当前告警状态
	RI_CURRENT_TOTAL_FLOW,				//当前累计流量
	RI_CURRENT_PRESSURE,				//当前压力
	RI_REAL_TIME_ALARM,					//实时压力告警
	RI_MAXIMUM_INSTANTANEOUS,			//前一天最大瞬时流量
	RI_MAX_I_HOUR,						//前一天最大瞬时流量，发生的时
	RI_MAX_I_MINUTE,					//前一天最大瞬时流量，发生的分
	RI_MINIMUM_INSTANTANEOUS,			//前一天最小瞬时流量
	RI_MIN_I_HOUR,						//前一天最小瞬时流量，发生的时
	RI_MIN_I_MINUTE,					//前一天最小瞬时流量，发生的分
	RI_THE_DAY_BEFORE_DATE,				//前一天日期 年月日
	RI_COMMUNICATION_SUCCESS_RATE,		//当前通讯成功率
	RI_ALL_PRESSURE,					//前一天所有压力数据，数组
	RI_ALL_TEMPERATURE					//前一天所有温度 ,数组
};

class CGPRSBigMeter {

public:
	//标准值
	VARIANT m_values[REG_COUNT];
private:
	string m_DeviceID;
	//最近一次数据采集时间
	time_t m_LastTime;
public:
	CGPRSBigMeter(void);
	~CGPRSBigMeter(void);

	void SetDeviceID(string strID);
	string GetDeviceID();

	void SetLastTime(time_t t);
	time_t GetLastTime();
	void operator=(CGPRSBigMeter &Meter);
};
