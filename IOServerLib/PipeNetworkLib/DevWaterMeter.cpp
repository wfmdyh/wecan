#include "stdafx.h"
#include "DevWaterMeter.h"


DevWaterMeter::DevWaterMeter()
{
	m_State = 0;				//水表状态
	m_TotalFlow = 0;			//累计流量
	m_HeatQuantity = 0;			//热量
	m_Flow = 0;					//瞬时流量
	m_Pressure = 0;				//压力
	m_Temperature = 0;			//温度
	m_RFState = 0;				//RF状态
}


DevWaterMeter::~DevWaterMeter()
{
}
