#include "stdafx.h"
#include "DevBigMeterException.h"


DevBigMeterException::DevBigMeterException()
{
	m_Time = { 0 };
	m_PressureValue = 0;
	m_Status = 0;
}


DevBigMeterException::~DevBigMeterException()
{
}
