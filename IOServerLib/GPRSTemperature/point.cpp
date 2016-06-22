#include "stdafx.h"
#include "point.h"


COL_POINT::COL_POINT()
{
	varData = new VARIANT;
	VariantInit(varData);
}

COL_POINT::~COL_POINT()
{
	delete varData;
	varData = NULL;
}

void COL_POINT::operator = (COL_POINT &cp)
{
	this->astrName = cp.astrName;
	this->m_IsChange = cp.m_IsChange;
	this->mapParam = cp.mapParam;
	if (cp.varData != NULL)
	{
		VariantCopy(varData, cp.varData);
	}
}