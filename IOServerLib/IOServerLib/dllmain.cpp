// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "IOServerLib.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	//HRESULT hr;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//CoUninitialize();
		break;
	}
	return TRUE;
}

