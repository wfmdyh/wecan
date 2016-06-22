// GPRSBigMeter.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "GPRSBigMeter.h"


CGPRSBigMeter::CGPRSBigMeter()
{
	m_DeviceID = "";
	m_LastTime = 0;

	m_MeterStatus = 0;					//当前告警状态
	m_TotalFlow = 0.0f;
	m_PressValue = 0;					//当前压力，原始数据
	m_PressStatus = 0;				//实时压力告警
	m_MaxFlow = 0.0f;					//最大瞬时流量
	m_MaxFlowTime = { 0 };
	m_MaxFlowTime.wYear = 1900;
	m_MaxFlowTime.wMonth = 1;
	m_MaxFlowTime.wDay = 1;
	m_MinFlow = 0.0f;
	m_MinFlowTime = { 0 };
	m_MinFlowTime.wYear = 1900;
	m_MinFlowTime.wMonth = 1;
	m_MinFlowTime.wDay = 1;
	m_AllPressure = L" ";
	m_AllTemperature = L" ";
	m_BeforeDate = { 0 };			//前一天日期
	m_BeforeDate.wYear = 1900;
	m_BeforeDate.wMonth = 1;
	m_BeforeDate.wDay = 1;
	m_SuccessRate = 0;					//通讯成功率
}

CGPRSBigMeter::~CGPRSBigMeter()
{
	//卸载字符串资源
	/*for (UINT i = 0; i < REG_COUNT; i++)
	{
	if (m_values[i].vt == VT_BSTR && m_values[i].bstrVal != NULL)
	{
	free(m_values[i].bstrVal);
	m_values[i].bstrVal = NULL;
	}
	}*/
}


void CGPRSBigMeter::SetDeviceID(string strID)
{
	m_DeviceID = strID;
}

string CGPRSBigMeter::GetDeviceID()
{
	return m_DeviceID;
}

void CGPRSBigMeter::SetMeterStatus(BYTE ms)
{
	m_MeterStatus = ms;
}

BYTE CGPRSBigMeter::GetMeterStatus()
{
	return m_MeterStatus;
}

void CGPRSBigMeter::SetTotalFlow(float flFlow)
{
	m_TotalFlow = flFlow;
}

float CGPRSBigMeter::GetTotalFlow()
{
	return m_TotalFlow;
}

void CGPRSBigMeter::SetPressValue(BYTE bValue)
{
	m_PressValue = bValue;
}

BYTE CGPRSBigMeter::GetPressValue()
{
	return m_PressValue;
}

void CGPRSBigMeter::SetPressStatus(USHORT status)
{
	m_PressStatus = status;
}

USHORT CGPRSBigMeter::GetPressStatus()
{
	return m_PressStatus;
}

void CGPRSBigMeter::SetMaxFlow(float flow)
{
	m_MaxFlow = flow;
}

float CGPRSBigMeter::GetMaxFlow(bool bActual)
{
	float tmp = 0.0f;
	if (bActual)
	{
		tmp = m_MaxFlow;
	}
	else{
		tmp = (m_MaxFlow == 99999 ? 0 : m_MaxFlow);
	}
	return tmp;
}

void CGPRSBigMeter::SetMaxFlowTime(SYSTEMTIME tm)
{
	m_MaxFlowTime = tm;
}

SYSTEMTIME CGPRSBigMeter::GetMaxFlowTime()
{
	return m_MaxFlowTime;
}

wstring CGPRSBigMeter::GetMaxFlowTimeString()
{
	wstringstream wss;
	wss << GetBeforeDateString();
	wss << L" ";
	wss << m_MaxFlowTime.wHour << L"-";
	wss << m_MaxFlowTime.wMinute << L"-" << L"00";
	return wss.str();
}

void CGPRSBigMeter::SetMinFlow(float flow)
{
	m_MinFlow = flow;
}

float CGPRSBigMeter::GetMinFlow(bool bActual)
{
	float tmp = 0.0f;
	if (bActual)
	{
		tmp = m_MinFlow;
	}
	else{
		tmp = (m_MinFlow == 99999 ? 0 : m_MinFlow);
	}
	return tmp;
}

void CGPRSBigMeter::SetMinFlowTime(SYSTEMTIME tm)
{
	m_MinFlowTime = tm;
}

SYSTEMTIME CGPRSBigMeter::GetMinFlowTime()
{
	return m_MinFlowTime;
}

wstring CGPRSBigMeter::GetMinFlowTimeString()
{
	wstringstream wss;
	wss << GetBeforeDateString();
	wss << L" ";
	wss << m_MinFlowTime.wHour << L"-";
	wss << m_MinFlowTime.wMinute << L"-" << L"00";
	return wss.str();
}

void CGPRSBigMeter::SetAllPressure(wstring strValue)
{
	if (strValue.length() == 0)
	{
		m_AllPressure = L" ";
	}
	else{
		m_AllPressure = strValue;
	}
}

wstring CGPRSBigMeter::GetAllPressure()
{
	return m_AllPressure;
}

void CGPRSBigMeter::SetAllTemperature(wstring strValue)
{
	if (strValue.length() == 0)
	{
		m_AllTemperature = L" ";
	}
	else{
		m_AllTemperature = strValue;
	}
}

wstring CGPRSBigMeter::GetAllTemperature()
{
	return m_AllTemperature;
}


void CGPRSBigMeter::SetBeforeDate(SYSTEMTIME tm)
{
	m_BeforeDate = tm;
}

SYSTEMTIME CGPRSBigMeter::GetBeforeDate()
{
	return m_BeforeDate;
}

wstring CGPRSBigMeter::GetBeforeDateString()
{
	wstringstream wss;
	wss << m_BeforeDate.wYear;
	wss << L"-";
	wss << m_BeforeDate.wMonth;
	wss << L"-";
	wss << m_BeforeDate.wDay;
	return wss.str();
}


void CGPRSBigMeter::SetSuccessRate(BYTE bValue)
{
	m_SuccessRate = bValue;
}

BYTE CGPRSBigMeter::GetSuccessRate()
{
	return m_SuccessRate;
}

//void CGPRSBigMeter::operator=(CGPRSBigMeter &meter)
//{
//	this->m_DeviceID = meter.m_DeviceID;
//	m_LastTime = meter.m_LastTime;
//	for (unsigned i = 0; i < REG_COUNT; i++)
//	{
//		VariantCopy(&(this->m_values[i]), &(meter.m_values[i]));
//	}
//}


void CGPRSBigMeter::SetLastTime(time_t t)
{
	m_LastTime = t;
}

time_t CGPRSBigMeter::GetLastTime()
{
	return m_LastTime;
}