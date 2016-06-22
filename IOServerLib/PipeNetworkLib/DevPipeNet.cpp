#include "stdafx.h"
#include "DevWaterMeter.h"
#include "DevPressureAlarm.h"
#include "DevMultipleParam.h"
#include "DevPipeNet.h"


DevPipeNet::DevPipeNet()
{
	m_AlarmState = 0;					//当前告警或状态标志
	m_TotalFlow = -1;					//累计流量
	m_QuantityOfHeat = 0;				//热量
	m_PAValue = 0;
	m_PAState = 0xFF;
	m_Flow = 0;						//当前瞬时流量
	m_Pressure = 0;					//当前压力
	m_Temperature = 0;				//当前温度
	memset(&m_Date, 0, sizeof(SYSTEMTIME));
	m_SucessRate = 0;					//当前通讯成功率
	memset(&m_CurrentTime, 0, sizeof(SYSTEMTIME));
	m_State = 0;						//水表状态
	m_SmallState = 0;					//小无线状态
	m_Verify[0] = 0;
	m_Verify[1] = 0;
	m_VerMeasurementID = 0;
	memset(m_VerMeasurementNum, 0, 6);
	//memset(m_VerCommunicationHNum, 0, 2);
	//memset(m_VerCommunicationSNum, 0, 2);
	m_VerCommunicationHNum = 0;
	m_VerCommunicationSNum = 0;
}


DevPipeNet::~DevPipeNet()
{
	vector<DevWaterMeter*>::iterator iteWM = m_ArrMeter.begin();
	for (; iteWM != m_ArrMeter.end(); iteWM++)
	{
		DevWaterMeter* pMeter = *iteWM;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrMeter.clear();
	m_ArrMeter.swap(vector<DevWaterMeter*>());

	vector<DevPressureAlarm*>::iterator itePA = m_ArrPressureAlarm.begin();
	for (; itePA != m_ArrPressureAlarm.end(); itePA++)
	{
		DevPressureAlarm* pMeter = *itePA;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrPressureAlarm.clear();
	m_ArrPressureAlarm.swap(vector<DevPressureAlarm*>());

	;
	vector<DevMultipleParam*>::iterator iteMP = m_ArrMultipleParam.begin();
	for (; iteMP != m_ArrMultipleParam.end(); iteMP++)
	{
		DevMultipleParam* pMeter = *iteMP;
		if (pMeter != NULL)
		{
			delete pMeter;
		}
	}
	m_ArrMultipleParam.clear();
	m_ArrMultipleParam.swap(vector<DevMultipleParam*>());
}

void DevPipeNet::SetUploadTime(SYSTEMTIME* t)
{
	if (t == NULL)
	{
		GetLocalTime(&m_UploadTime);
	}
	else{
		m_UploadTime = *t;
	}
}

SYSTEMTIME DevPipeNet::GetUploadTime()
{
	return m_UploadTime;
}

void DevPipeNet::operator=(DevPipeNet& dev)
{
	m_UploadTime = dev.m_UploadTime;			//数据到达时间
	m_DeviceID = dev.m_DeviceID;					//设备ID
	m_AlarmState = dev.m_AlarmState;					//当前告警或状态标志
	m_TotalFlow = dev.m_TotalFlow;					//累计流量
	m_QuantityOfHeat = dev.m_QuantityOfHeat;			//热量
	m_PAState = dev.m_PAState;
	m_PAValue = dev.m_PAValue;
	m_ArrTemperature = dev.m_ArrTemperature;		//前一天所有温度 0.1
	m_ArrPressure = dev.m_ArrPressure;			//前一天所有压力数据
	m_Flow = dev.m_Flow;						//当前瞬时流量
	m_Pressure = dev.m_Pressure;					//当前压力
	m_Temperature = dev.m_Temperature;				//当前温度
	m_Date = dev.m_Date;					//前一天的日期
	m_SucessRate = dev.m_SucessRate;					//当前通讯成功率
	m_CurrentTime = dev.m_CurrentTime;			//当前日期
	m_State = dev.m_State;						//水表状态
	m_SmallState = dev.m_SmallState;					//小无线状态
	//多表数据
	vector<DevWaterMeter*>::iterator iteWM = dev.m_ArrMeter.begin();
	for (; iteWM != dev.m_ArrMeter.end(); iteWM++)
	{
		DevWaterMeter* pWM = new DevWaterMeter;
		*pWM = **iteWM;
		m_ArrMeter.push_back(pWM);
	}

	memcpy(m_Verify, dev.m_Verify, 2);
	//实时压力告警（变长）
	vector<DevPressureAlarm*>::iterator itePA = dev.m_ArrPressureAlarm.begin();
	for (; itePA != dev.m_ArrPressureAlarm.end(); itePA++)
	{
		DevPressureAlarm* pPA = new DevPressureAlarm;
		*pPA = **itePA;
		m_ArrPressureAlarm.push_back(pPA);
	}
	//实时多参数数据
	vector<DevMultipleParam*>::iterator iteMP = dev.m_ArrMultipleParam.begin();
	for (; iteMP != dev.m_ArrMultipleParam.end(); iteMP++)
	{
		DevMultipleParam* pMP = new DevMultipleParam;
		*pMP = **iteMP;
		m_ArrMultipleParam.push_back(pMP);
	}
	//测量模块软件版本号
	m_VerMeasurementID = dev.m_VerMeasurementID;
	memcpy(m_VerMeasurementNum, dev.m_VerMeasurementNum, 6);
	//通讯模块版本硬件，软件版本号
	m_VerCommunicationHNum = dev.m_VerCommunicationHNum;
	m_VerCommunicationSNum = dev.m_VerCommunicationSNum;
}