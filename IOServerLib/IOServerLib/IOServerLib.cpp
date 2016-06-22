// IOServerLib.cpp : 定义 DLL 应用程序的导出函数。
//日期：2015.3.25
//

#include "stdafx.h"
#include "IOServerLib.h"
#include "channel.h"
#include "MyTools.h"
#include "IOServerManager.h"
#include "OPCServer.h"
#include "DataDef.h"

//point的缓存大小
#define POINT_DATA_BUF 6000
//读取数据的回调函数
PReadColData g_fnReadColData = NULL;
IOServerManager *g_ioMgr = NULL;
vector<wchar_t*> g_vecChannels;					//返回给界面展示的数据
fnShowLog g_lpfnShowLog = 0;					//日志函数

//当通道有数据时的回调函数 pChannel：采集完成的通道数据，多线程
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
	OutputDebugString(L"数据到达\n");
}

//回调函数 参数：pTraManager，赋值完毕的转发数据，是转发通道的一个副本
bool UpdataTransmit(TransmitManager *pTraManager)
{
	//给转发点赋值
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
				//是否有对应的采集点
				int iColChannel = pTraPoint->GetColChannel();
				int iColDevice = pTraPoint->GetColDevice();
				int iColPoint = pTraPoint->GetColPoint();
				if (iColChannel == -1 || iColDevice == -1 || iColPoint == -1)
				{
					continue;
				}
				//线程互斥
				EnterCriticalSection(&g_ioMgr->m_csChannels);
				VARIANT *varValue = g_ioMgr->m_Channels->at(iColChannel)->vecDevice.at(iColDevice)->vecPoint.at(iColPoint)->varData;
				//赋值
				if (varValue != NULL)
				{
					pTraPoint->SetValue(*varValue);
				}
				LeaveCriticalSection(&g_ioMgr->m_csChannels);
			}
			
		}
	}
	//OutputDebugString(L"提取数据到转发通道\n");
	return true;
}

//指令下发
extern "C" __declspec(dllexport) BOOL IOS_WriteData(wstring strChannel, wstring strDevice, wstring strPoint, wstring strValue)
{
	//通道类型
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
	//			//找出对应的动态库，调用写入函数
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


//wstrPath项目路径
//返回值：非0代表成功，0失败
extern "C" __declspec(dllexport) int IOS_Start(wchar_t *wszProjectPath)
{
	//设置系统默认的
	setlocale(LC_ALL, "");
	//获取项目路径
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

//停止IOServer
extern "C" __declspec(dllexport) void IOS_Stop()
{
	if (g_ioMgr != NULL)
	{
		g_ioMgr->Stop();
		delete g_ioMgr;
		g_ioMgr = NULL;
	}
}


//注册回调函数
extern "C" __declspec(dllexport) void IOS_RegColReadDataFun(PReadColData fnReadColData)
{
	g_fnReadColData = fnReadColData;
}



//根据通道名称，获取值的数组
extern "C" __declspec(dllexport) BOOL IOS_GetDataArrByChannel(wchar_t *wszChannelName)
{
	int iPointCount = 0;
	char szChannelName[1024] = {0};
	wstring strValue;
	//清除数据
	vector<wchar_t*>::iterator iteData = g_vecChannels.begin();
	for (; iteData != g_vecChannels.end(); iteData++)
	{
		wchar_t *pStr = *iteData;
		delete[] pStr;
	}
	g_vecChannels.clear();
	//初始化为60000
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
			//通道名相同，取出通道所有的值
			for (;iteDev != pChannel->vecDevice.end(); iteDev++)
			{
				DEVICE_INFO *pDev = *iteDev;
				vector<COL_POINT*>::iterator itePoint = pDev->vecPoint.begin();
				for(; itePoint != pDev->vecPoint.end() ; itePoint++)
				{
					COL_POINT *pPoint = *itePoint;
					//设备名称
					strValue = ANSIToUnicode(pDev->astrDeviceName);
					strValue += L"&";
					
					////点名
					strValue += ANSIToUnicode(pPoint->astrName);
					strValue += L"&";

					////点值
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

//反注册OPCServer
extern "C" __declspec(dllexport) BOOL UnRegOPCServer()
{
	return COPCServer::UnRegOPCServer();
}

//注册打印日志函数
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
		va_start(maker, wcsMsg);						//初始化变量参数
		int iLength = _vscwprintf(wcsMsg, maker) + 1;		//格式化以后的字符串长度
		//动态数组
		vector<wchar_t> vecWsz(iLength, L'\0');
		int iWritten = _vsnwprintf_s(&(vecWsz[0]), iLength, 1024, wcsMsg, maker);
		if (iWritten > 0)
		{
			strResult = &(vecWsz[0]);
		}
		va_end(maker);								//重置变量参数
	}
	if (g_lpfnShowLog != NULL)
	{
		(*g_lpfnShowLog)(strResult.c_str());
	}
}