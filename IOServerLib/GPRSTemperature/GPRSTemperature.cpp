// GPRSTemperature.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "GPRSTemperature.h"


CGPRSTemperature::CGPRSTemperature()
{
	m_DeviceID = "";
	for (UINT i = 0; i < REG_COUNT; i++)
	{
		VariantInit(&m_values[i]);
	}
}

CGPRSTemperature::~CGPRSTemperature()
{
}

void CGPRSTemperature::UpdateData(CGPRSTemperature devGPRS)
{
	for (UINT i = 0; i < REG_COUNT;i++)
	{
		VariantCopy(&m_values[i], &devGPRS.m_values[i]);
	}
}

void CGPRSTemperature::SetDeviceID(string strID)
{
	m_DeviceID = strID;
}

string CGPRSTemperature::GetDeviceID()
{
	return m_DeviceID;
}