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
	//如果有就删除原有的对象
	vector<DEVICE_INFO*>::iterator iteDev = vecDevice.begin();
	for (; iteDev != vecDevice.end(); iteDev++)
	{
		delete *iteDev;
	}
	vecDevice.clear();
	for (unsigned i = 0; i < ci.vecDevice.size(); i++)
	{
		DEVICE_INFO *destDev = NULL;
		DEVICE_INFO *sourceDev = ci.vecDevice.at(i);
		destDev = new DEVICE_INFO;
		*destDev = *sourceDev;
		this->vecDevice.push_back(destDev);
	}
}