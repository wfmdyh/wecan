#include "stdafx.h"
#include "channel.h"


CHANNEL_INFO::CHANNEL_INFO()
{
	GenerateID();
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

wstring CHANNEL_INFO::GetID()
{
	return m_ID;
}

//生成ID
void CHANNEL_INFO::GenerateID()
{
	GUID guid;
	memset(&guid, 0, sizeof(GUID));
	CoCreateGuid(&guid);
	wchar_t buf[64] = { 0 };
	_snwprintf_s(
		buf,
		sizeof(buf),
		L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	m_ID = buf;
}