#include "stdafx.h"
#include "channel.h"
#include "DataManager.h"


DataManager::DataManager()
{
	InitializeCriticalSection(&m_LockChannel);
}


DataManager::~DataManager()
{
	DeleteCriticalSection(&m_LockChannel);
}

//���һ��ͨ������
BOOL DataManager::AddChannel(CHANNEL_INFO channel)
{
	Lock();
	m_Channels.push_back(channel);

	Unlock();
	return TRUE;
}

void DataManager::Lock()
{
	EnterCriticalSection(&m_LockChannel);
}

void DataManager::Unlock()
{
	LeaveCriticalSection(&m_LockChannel);
}

int DataManager::GetChannelPort(string strName)
{
	vector<CHANNEL_INFO>::iterator ite = m_Channels.begin();
	for (; ite != m_Channels.end(); ite++)
	{
		string dllFile = ite->astrDllFile;
		if (string::npos != dllFile.rfind(strName))
		{
			//�˿ں�
			string strPort = ite->mapParam["Port"];
			//��ǰֻ����һ��ͨ��
			return atoi(strPort.c_str());
		}
	}
	return 0;
}