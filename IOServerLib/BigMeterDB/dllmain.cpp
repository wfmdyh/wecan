#include "stdafx.h"
#include "BigMeterDB.h"
#include "GPRSBigMeter.h"
#include "MyTools.h"
#include "BigMeterDB.h"

//·ÇÍÐ¹Ü´úÂë
#pragma unmanaged
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


extern "C" __declspec(dllexport) void dbtest()
{
	TestSave();
}

extern "C" __declspec(dllexport) BOOL SaveDataToDB(vector<CGPRSBigMeter>* listGprsBigMeter)
{
	vector<CGPRSBigMeter> tmpVec = *listGprsBigMeter;
	return SaveData(tmpVec);
}