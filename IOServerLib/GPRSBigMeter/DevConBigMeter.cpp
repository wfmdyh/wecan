#include "stdafx.h"
#include "DevConBigMeter.h"


DevConBigMeter::DevConBigMeter()
{
	m_Time = { 0 };			//年月日时
	m_Status = 0;
	m_TotalFlow = 0.0;
	m_MaxFlow = 0.0f;
	m_MaxFlowTime = { 0 };
	m_MinFlow = 0.0f;
	m_MinFlowTime = { 0 };
}


DevConBigMeter::~DevConBigMeter()
{
}
