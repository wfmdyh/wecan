#include "stdafx.h"
#include "point.h"


COL_POINT::COL_POINT()
{
	varData = new VARIANT;
	VariantInit(varData);

	memset(&m_CollectTime, 0, sizeof(struct tm));
}

COL_POINT::~COL_POINT()
{
	delete varData;
	varData = NULL;
}

void COL_POINT::operator = (COL_POINT &cp)
{
	this->astrName = cp.astrName;
	//this->m_IsChange = cp.m_IsChange;
	this->mapParam = cp.mapParam;
	if (cp.varData != NULL)
	{
		VariantCopy(varData, cp.varData);
	}
}


wstring COL_POINT::GetValue()
{
	wstring strValue;
	wchar_t *wcsValue = NULL;
	if (varData->vt == VT_BSTR)
	{
		size_t varLen = wcslen(varData->bstrVal);
		wcsValue = new wchar_t[varLen + 2];
		memset(wcsValue, 0, varLen + 2);
	}
	else{
		//默认的大小
		wcsValue = new wchar_t[50];
		memset(wcsValue, 0, 50);
	}

	if (m_tools.MapVariantToString(*varData, wcsValue))
	{
		strValue = wcsValue;
	}
	delete[] wcsValue;

	return strValue;
}