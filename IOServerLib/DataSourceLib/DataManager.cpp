#include "stdafx.h"
#include "DataManager.h"
#include <msclr\marshal.h>
//���ݿ�***************************
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib") 

using namespace DatabaseServer;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

using namespace msclr::interop;

DataManager::DataManager()
{
	m_Log.SetFileName(L"DB");
}

DataManager::~DataManager()
{

}

void DataManager::SetChannels(vector<CHANNEL_INFO*> vecChannels)
{
	m_vecChannels = vecChannels;
}

bool DataManager::SaveData()
{
	//OutputDebugString(L"Enter SaveData\n");

	Dictionary<String^, double> ^data = gcnew Dictionary<String^, double>;

	//ͨ��
	for (unsigned int nChannel = 0; nChannel<m_vecChannels.size(); nChannel++)
	{
		CHANNEL_INFO *pChannel = m_vecChannels.at(nChannel);
		
		for (unsigned int nDev = 0; nDev < pChannel->vecDevice.size(); nDev++)
		{
			DEVICE_INFO *pDev = pChannel->vecDevice.at(nDev);
			//for (unsigned int nPoint = 0; nPoint < pDev->vecPoint.size(); nPoint++)
			{
				double dbVal = -50;
				COL_POINT *pPoint = pDev->vecPoint.at(0);			//ֻȡ��һ���㣬ȡΨһ��һ����
				String^ strKey = marshal_as<String^>(pDev->astrDeviceName.c_str());

				if (!pPoint->IsChange)
				{
					continue;
				}
				else
				{
					pPoint->IsChange = false;
					/*wchar_t wcsMsg[100] = {0};
					size_t nLen = 0;
					mbstowcs_s(&nLen, wcsMsg, pDev->astrDeviceName.c_str(), 100);
					m_Log.LogMsgToFile(L"%s", wcsMsg);*/
				}

				if ((*pPoint->varData).vt != VT_EMPTY)
				{
					dbVal = pPoint->varData->fltVal;
				}

				if (dbVal != -50)
				{
					data->Add(strKey, dbVal);
				}
				
			}
		}
	}

	//�洢
	if (data->Count <= 0)
	{
		//û���κ����ݣ�����Ҫ�洢
		return true;
	}

	Session ^session = gcnew Session;

	if (!session->Open())
	{
		session->Close();
		wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(session->Error);
		m_Log.LogMsgToFile(L"%s", wcsMsg);
		return false;
	}

	if (!session->SaveCollectPointData(data))
	{
		session->Close();
		/*wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(session->Error);
		m_Log.LogMsgToFile(L"%s", wcsMsg);*/
		return false;
	}

	session->Close();

	//OutputDebugString(L"Leave SaveData\n");
	return true;
}