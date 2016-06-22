#pragma once
/************************************************************************
  作者：wfm
  日期：2015.05.29
  功能：协议解析类
************************************************************************/

//寄存器名称
#define REG_SRV_ADD			"服务器地址"
#define REG_SRV_PORT		"服务器端口"
#define REG_TCNT			"连接间隔"
#define REG_TCOM			"通讯间隔"
#define REG_Temperature		"温度值"
#define REG_HUMI			"湿度值"
#define REG_LOWPOWER		"电池低电量"
#define REG_CCID			"CCID"
#define REG_CSQ				"信号强度"

//寄存器个数
#define REG_COUNT 9
enum REG_INDEX
{
	RI_SRV_ADD = 0,
	RI_SRV_PORT,
	RI_TCNT,
	RI_TCOM,
	RI_Temperature,		//温度值
	RI_HUMI,			//湿度
	RI_LOWPOWER,
	RI_CCID,
	RI_CSQ
};

class CGPRSTemperature {

public:
	//标准值
	VARIANT m_values[REG_COUNT];

private:
	string m_DeviceID;

public:
	CGPRSTemperature(void);
	~CGPRSTemperature(void);
	
	//将数据更新成devGPRS的数据
	void UpdateData(CGPRSTemperature devGPRS);

	void SetDeviceID(string strID);
	string GetDeviceID();
};
