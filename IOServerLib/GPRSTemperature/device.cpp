#include "stdafx.h"
#include "device.h"

DEVICE_INFO::DEVICE_INFO()
{
}

//�ͷ���Դ
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
	//�Ƿ����豸
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
