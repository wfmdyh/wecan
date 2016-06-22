#include "stdafx.h"
#include "device.h"

DEVICE_INFO::DEVICE_INFO()
{
}

//释放资源
DEVICE_INFO::~DEVICE_INFO()
{
	OutputDebugString(L"析构_Device\n");
	for (unsigned i = 0; i < vecPoint.size(); i++)
	{
		COL_POINT *pPoint = vecPoint.at(i);
		if (pPoint != NULL)
		{
			delete pPoint;
			pPoint = NULL;
		}
	}
	vecPoint.clear();
}

void DEVICE_INFO::operator = (DEVICE_INFO &di)
{
	this->astrDeviceName = di.astrDeviceName;
	this->mapParam = di.mapParam;
	//是否有设备
	bool bHave = false;
	if (vecPoint.size() > 0)
	{
		bHave = true;
	}
	for (unsigned i = 0; i < di.vecPoint.size(); i++)
	{
		COL_POINT *destPoint = NULL;
		COL_POINT *sourcePoint = di.vecPoint.at(i);
		if (bHave)
		{
			destPoint = this->vecPoint.at(i);
			*destPoint = *sourcePoint;
		}
		else{
			destPoint = new COL_POINT;
			*destPoint = *sourcePoint;
			this->vecPoint.push_back(destPoint);
		}
	}
}
