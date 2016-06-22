#include "stdafx.h"
#include "IOSDriver.h"


CIOSDriver::CIOSDriver()
{
	m_hMod = NULL;
	m_nType = CIOSDriver::IOSDTYPE::TYPE_COL;
}


CIOSDriver::~CIOSDriver()
{
	/*if (m_hMod != NULL)
	{
		FreeLibrary(m_hMod);
		m_hMod = NULL;
	}*/
}

string CIOSDriver::GetName()
{
	return m_strName;
}

HMODULE CIOSDriver::GetHandle()
{
	if (m_hMod == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	return m_hMod;
}

void CIOSDriver::SetName(string strName)
{
	m_strName = strName;
}

void CIOSDriver::SetHandle(HMODULE hMod)
{
	m_hMod = hMod;
}

void CIOSDriver::SetType(IOSDTYPE type)
{
	m_nType = type;
}

int CIOSDriver::GetType()
{
	return m_nType;
}