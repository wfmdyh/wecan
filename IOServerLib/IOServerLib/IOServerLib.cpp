// IOServerLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//���ڣ�2015.3.25
//

#include "stdafx.h"
#include "IOServerLib.h"
#include "channel.h"
#include "MyTools.h"
#include "IOServerManager.h"
#include "OPCServer.h"
#include "DataDef.h"

//point�Ļ����С
#define POINT_DATA_BUF 6000
//��ȡ���ݵĻص�����
PReadColData g_fnReadColData = NULL;
IOServerManager *g_ioMgr = NULL;
vector<wchar_t*> g_vecChannels;					//���ظ�����չʾ������
fnShowLog g_lpfnShowLog = 0;					//��־����

//��ͨ��������ʱ�Ļص����� pChannel���ɼ���ɵ�ͨ�����ݣ����߳�
void DataArrive(CHANNEL_INFO *pChannel)
{
	EnterCriticalSection(&g_ioMgr->m_csChannels);
	vector<CHANNEL_INFO*>::iterator ite = g_ioMgr->m_Channels->begin();
	for (; ite != g_ioMgr->m_Channels->end(); ite++)
	{
		if (pChannel->astrName.compare((*ite)->astrName) == 0)
		{
			*(*ite) = *pChannel;
		}
	}
	LeaveCriticalSection(&g_ioMgr->m_csChannels);
	OutputDebugString(L"���ݵ���\n");
}

//�ص����� ������pTraManager����ֵ��ϵ�ת�����ݣ���ת��ͨ����һ������
bool UpdataTransmit(TransmitManager *pTraManager)
{
	//��ת���㸳ֵ
	vector<TransmitChannel*> vecTraChannel = pTraManager->GetVecChannel();
	vector<TransmitChannel*>::iterator iteTraChannel = vecTraChannel.begin();
	for (; iteTraChannel != vecTraChannel.end(); iteTraChannel++)
	{
		TransmitChannel *pTraChannel = *iteTraChannel;
		vector<TransmitPoint*> vecTraPoint = pTraChannel->GetVecPoint();
		vector<TransmitPoint*>::iterator iteTraPoint = vecTraPoint.begin();
		for (; iteTraPoint != vecTraPoint.end(); iteTraPoint++)
		{
			TransmitPoint *pTraPoint = *iteTraPoint;
			
			if (pTraPoint != NULL)
			{
				//�Ƿ��ж�Ӧ�Ĳɼ���
				int iColChannel = pTraPoint->GetColChannel();
				int iColDevice = pTraPoint->GetColDevice();
				int iColPoint = pTraPoint->GetColPoint();
				if (iColChannel == -1 || iColDevice == -1 || iColPoint == -1)
				{
					continue;
				}
				//�̻߳���
				EnterCriticalSection(&g_ioMgr->m_csChannels);
				VARIANT *varValue = g_ioMgr->m_Channels->at(iColChannel)->vecDevice.at(iColDevice)->vecPoint.at(iColPoint)->varData;
				//��ֵ
				if (varValue != NULL)
				{
					pTraPoint->SetValue(*varValue);
				}
				LeaveCriticalSection(&g_ioMgr->m_csChannels);
			}
			
		}
	}
	//OutputDebugString(L"��ȡ���ݵ�ת��ͨ��\n");
	return true;
}

//ָ���·�
extern "C" __declspec(dllexport) BOOL IOS_WriteData(wstring strChannel, wstring strDevice, wstring strPoint, wstring strValue)
{
	//ͨ������
	//for (unsigned int i = 0; i<g_pvecChannels->size(); i++)
	//{
	//	//COL_DATA cd = g_pvecChannels->at(i);
	//	CHANNEL_INFO *pChannel = g_pvecChannels->at(i);
	//	if (pChannel->astrName == UnicodeToANSI(strChannel))
	//	{
	//		for (unsigned int d=0;d<g_vecDriverInfo.size();d++)
	//		{
	//			DRIVER_INFO di = g_vecDriverInfo.at(d);
	//			string astrDllFile = di.astrDllFullPath;
	//			//�ҳ���Ӧ�Ķ�̬�⣬����д�뺯��
	//			if (astrDllFile == pChannel->astrDllFile)
	//			{
	//				//PDeviceWrite DeviceWrite = (PDeviceWrite)GetProcAddress(di.hDriver,"IOS_DeviceWrite");
	//				//if (DeviceWrite == NULL)
	//				{
	//					return FALSE;
	//				}
	//				//return (*DeviceWrite)(strChannel,strDevice,strPoint,strValue);
	//			}
	//		}
	//	}
	//}
	return FALSE;
}


//wstrPath��Ŀ·��
//����ֵ����0����ɹ���0ʧ��
extern "C" __declspec(dllexport) int IOS_Start(wchar_t *wszProjectPath)
{
	//����ϵͳĬ�ϵ�
	setlocale(LC_ALL, "");
	//��ȡ��Ŀ·��
	wchar_t wszTmpProPath[MAX_PATH] = {0};
	memcpy(wszTmpProPath,wszProjectPath,wcslen(wszProjectPath)*sizeof(wchar_t));
	char szProjectPath[MAX_PATH] = {0};
	chUnicodeToANSI(wszTmpProPath,szProjectPath);
	
	g_ioMgr = new IOServerManager(szProjectPath);
	if (!g_ioMgr->Start())
	{
		return 0;
	}

	return TRUE;
}

//ֹͣIOServer
extern "C" __declspec(dllexport) void IOS_Stop()
{
	if (g_ioMgr != NULL)
	{
		g_ioMgr->Stop();
		delete g_ioMgr;
		g_ioMgr = NULL;
	}
}


//ע��ص�����
extern "C" __declspec(dllexport) void IOS_RegColReadDataFun(PReadColData fnReadColData)
{
	g_fnReadColData = fnReadColData;
}



//����ͨ�����ƣ���ȡֵ������
extern "C" __declspec(dllexport) BOOL IOS_GetDataArrByChannel(wchar_t *wszChannelName)
{
	int iPointCount = 0;
	char szChannelName[1024] = {0};
	wstring strValue;
	//�������
	vector<wchar_t*>::iterator iteData = g_vecChannels.begin();
	for (; iteData != g_vecChannels.end(); iteData++)
	{
		wchar_t *pStr = *iteData;
		delete[] pStr;
	}
	g_vecChannels.clear();
	//��ʼ��Ϊ60000
	//g_vecChannels.resize(60000);
	unsigned nIndex = 0;

	if (g_ioMgr->m_Channels == NULL)
	{
		return FALSE;
	}
	chUnicodeToANSI(wszChannelName,szChannelName);

	vector<CHANNEL_INFO*>::iterator iteChannel = g_ioMgr->m_Channels->begin();
	for (; iteChannel != g_ioMgr->m_Channels->end(); iteChannel++)
	{
		CHANNEL_INFO *pChannel = *iteChannel;
		if (pChannel->astrName.compare(szChannelName) == 0)
		{
			vector<DEVICE_INFO*>::iterator iteDev = pChannel->vecDevice.begin();
			//ͨ������ͬ��ȡ��ͨ�����е�ֵ
			for (;iteDev != pChannel->vecDevice.end(); iteDev++)
			{
				DEVICE_INFO *pDev = *iteDev;
				vector<COL_POINT*>::iterator itePoint = pDev->vecPoint.begin();
				for(; itePoint != pDev->vecPoint.end() ; itePoint++)
				{
					COL_POINT *pPoint = *itePoint;
					//�豸����
					strValue = ANSIToUnicode(pDev->astrDeviceName);
					strValue += L"&";
					
					////����
					strValue += ANSIToUnicode(pPoint->astrName);
					strValue += L"&";

					////��ֵ
					strValue += pPoint->GetValue();
					
					wchar_t *pData = new wchar_t[strValue.size() + 2];
					wcscpy(pData, strValue.c_str());
					g_vecChannels.push_back(pData);
				}
			}
		}
		break;
	}
	unsigned int nlen = g_vecChannels.size();
	while (g_vecChannels.size() < 60000)
	{
		g_vecChannels.push_back(NULL);
	}
	wchar_t ** tmpVal = &g_vecChannels[0];
	(*g_fnReadColData)(tmpVal, nlen);
	return TRUE;
}


extern "C" __declspec(dllexport) BOOL RegOPCServer(LPCTSTR wszAppExePath)
{
	return COPCServer::RegOPCServer(wszAppExePath);
}

//��ע��OPCServer
extern "C" __declspec(dllexport) BOOL UnRegOPCServer()
{
	return COPCServer::UnRegOPCServer();
}

//ע���ӡ��־����
extern "C" __declspec(dllexport) void RegShowMessage(fnShowLog lpfnShowLog)
{
	g_lpfnShowLog = lpfnShowLog;
}

void ShowMessage(const wchar_t* wcsMsg, ...)
{
	wstring strResult = L"";
	if (NULL != wcsMsg)
	{
		va_list maker = NULL;
		va_start(maker, wcsMsg);						//��ʼ����������
		int iLength = _vscwprintf(wcsMsg, maker) + 1;		//��ʽ���Ժ���ַ�������
		//��̬����
		vector<wchar_t> vecWsz(iLength, L'\0');
		int iWritten = _vsnwprintf_s(&(vecWsz[0]), iLength, 1024, wcsMsg, maker);
		if (iWritten > 0)
		{
			strResult = &(vecWsz[0]);
		}
		va_end(maker);								//���ñ�������
	}
	if (g_lpfnShowLog != NULL)
	{
		(*g_lpfnShowLog)(strResult.c_str());
	}
}