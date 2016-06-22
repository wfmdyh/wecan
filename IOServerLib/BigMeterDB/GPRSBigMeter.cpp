// GPRSBigMeter.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "GPRSBigMeter.h"


CGPRSBigMeter::CGPRSBigMeter()
{
	m_DeviceID = "";
	for (UINT i = 0; i < REG_COUNT; i++)
	{
		VariantInit(&m_values[i]);
	}
	m_LastTime = 0;
}

CGPRSBigMeter::~CGPRSBigMeter()
{
	//ж���ַ�����Դ
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

void CGPRSBigMeter::operator=(CGPRSBigMeter &meter)
{
	this->m_DeviceID = meter.m_DeviceID;
	m_LastTime = meter.m_LastTime;
	for (unsigned i = 0; i < REG_COUNT; i++)
	{
		VariantCopy(&(this->m_values[i]), &(meter.m_values[i]));
	}
}


void CGPRSBigMeter::SetLastTime(time_t t)
{
	m_LastTime = t;
}

time_t CGPRSBigMeter::GetLastTime()
{
	return m_LastTime;
}