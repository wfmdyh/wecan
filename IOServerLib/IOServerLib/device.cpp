#include "stdafx.h"
#include "device.h"

DEVICE_INFO::DEVICE_INFO()
{
}

//สอทลืสิด
DEVICE_INFO::~DEVICE_INFO()
{
	for (unsigned i = 0; i < vecPoint.size(); i++)
	{
		COL_POINT *pPoint = vecPoint.at(i);
		if (pPoint != NULL)
		{
			delete pPoint;
			vecPoint.at(i) = NULL;
		}
	}
	vecPoint.clear();
}

void DEVICE_INFO::operator = (DEVICE_INFO &di)
{
	this->astrDeviceName = di.astrDeviceName;
	this->mapParam = di.mapParam;
	vector<COL_POINT*>::iterator ite = vecPoint.begin();
	for (; ite != vecPoint.end(); ite++)
	{
		delete *ite;
	}
	vecPoint.clear();
	for (unsigned i = 0; i < di.vecPoint.size(); i++)
	{
		COL_POINT *destPoint = NULL;
		COL_POINT *sourcePoint = di.vecPoint.at(i);
		
		destPoint = new COL_POINT;
		*destPoint = *sourcePoint;
		this->vecPoint.push_back(destPoint);
	}
}