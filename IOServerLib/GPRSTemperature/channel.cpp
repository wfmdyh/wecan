#include "stdafx.h"
#include "channel.h"


CHANNEL_INFO::CHANNEL_INFO()
{

}

CHANNEL_INFO::~CHANNEL_INFO()
{
	for (unsigned i = 0; i < vecDevice.size(); i++)
	{
		DEVICE_INFO *pDev = vecDevice.at(i);
		if (pDev != NULL)
		{
			delete pDev;
			pDev = NULL;
		}
	}
	vecDevice.clear();
}

void CHANNEL_INFO::operator = (CHANNEL_INFO &ci)
{
	this->astrName = ci.astrName;
	this->astrDllFile = ci.astrDllFile;
	this->astrConfigFileFullPath = ci.astrConfigFileFullPath;
	this->astrType = ci.astrType;
	this->mapParam = ci.mapParam;
	//是否有设备
	bool bHave = false;
	if (vecDevice.size() > 0)
	{
		bHave = true;
	}
	for (unsigned i = 0; i < ci.vecDevice.size(); i++)
	{
		DEVICE_INFO *destDev = NULL;
		DEVICE_INFO *sourceDev = ci.vecDevice.at(i);
		if (bHave)
		{
			destDev = this->vecDevice.at(i);
			*destDev = *sourceDev;
		}
		else{
			destDev = new DEVICE_INFO;
			*destDev = *sourceDev;
			this->vecDevice.push_back(destDev);
		}

	}
}