// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "channel.h"
#include "ProtocolMgr.h"
#include "DataDef.h"
#include "DataManager.h"

fnShowMessage g_lpfnShowMessage = NULL;


//���йܴ���
#pragma unmanaged
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

ProtocolMgr* g_ProMgr = NULL;

extern "C" __declspec(dllexport) BOOL IOS_DeviceOpen(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData)
{
	return FALSE;
}

//�����°溯�����µĹ���Ӧ�õ��øýӿ�
extern "C" __declspec(dllexport) BOOL IOS_DeviceOpenEx(fnShowMessage lpfnShowMessage, DataManager* pMgr)
{
	g_lpfnShowMessage = lpfnShowMessage;
	g_ProMgr = new ProtocolMgr;
	return g_ProMgr->Start(pMgr);
}

extern "C" __declspec(dllexport) void IOS_DeviceClose()
{
	g_ProMgr->Stop();
	delete g_ProMgr;
}

//��ӡ��־
void ShowMessage(const wchar_t* wcsMsg)
{
	if (g_lpfnShowMessage != NULL && wcsMsg != NULL)
	{
		(*g_lpfnShowMessage)(wcsMsg);
	}
}