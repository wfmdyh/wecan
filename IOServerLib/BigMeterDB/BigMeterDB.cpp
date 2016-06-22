#include "stdafx.h"
#include "BigMeterDB.h"
#include "GPRSBigMeter.h"




BOOL SaveData(vector<CGPRSBigMeter> listGprsBigMeter)
{
	List<MeterData^> ^listData = gcnew List<MeterData^>;
	auto ite = listGprsBigMeter.begin();
	for (; ite != listGprsBigMeter.end(); ite++)
	{
		MeterData ^md = gcnew MeterData();
		md->MeterCode = marshal_as<String^>(ite->GetDeviceID().c_str());
		
		md->MeterStatus = ite->m_values[RI_CURRENT_ALARM].bVal;
		md->TotalFlow = ite->m_values[RI_CURRENT_TOTAL_FLOW].fltVal;
		md->PressValue = (int)(ite->m_values[RI_CURRENT_PRESSURE].fltVal);
		md->PressStatus = ite->m_values[RI_REAL_TIME_ALARM].uiVal;
		///
		md->SourceMaxFlow = ite->m_values[RI_MAXIMUM_INSTANTANEOUS].fltVal;
		//时，分有效
		DateTime ^date = gcnew DateTime(2016, 3, 11,
			ite->m_values[RI_MAX_I_HOUR].bVal,
			ite->m_values[RI_MAX_I_MINUTE].bVal, 55);
		md->MaxFlowTime = *date;
		///
		md->SourceMinFlow = ite->m_values[RI_MINIMUM_INSTANTANEOUS].fltVal;
		date = gcnew DateTime(2016, 3, 11, 
			ite->m_values[RI_MIN_I_HOUR].bVal, 
			ite->m_values[RI_MIN_I_MINUTE].bVal, 55);
		md->MinFlowTime = *date;
		///
		String ^strValue = marshal_as<String^>(ite->m_values[RI_THE_DAY_BEFORE_DATE].bstrVal);
		md->CopyDate = strValue;
		strValue = marshal_as<String^>(ite->m_values[RI_ALL_PRESSURE].bstrVal);
		md->PressRecord = strValue;
		strValue = marshal_as<String^>(ite->m_values[RI_ALL_TEMPERATURE].bstrVal);
		md->TempRecord = strValue;

		listData->Add(md);
	}
	if (listData->Count <= 0)
	{
		OutputDebugString(L"没有数据，不存储\n");
		return TRUE;
	}
	Session ^se = gcnew Session;
	if (se->Open())
	{
		bool bResult = se->SaveMeterRecords(listData);
		se->Close();
		if (bResult)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void TestSave()
{
	/*
	RI_CURRENT_ALARM = 0,				//当前告警状态
	RI_CURRENT_TOTAL_FLOW,				//当前累计流量
	RI_CURRENT_PRESSURE,				//当前压力
	RI_REAL_TIME_ALARM,					//实时压力告警
	RI_MAXIMUM_INSTANTANEOUS,			//前一天最大瞬时流量
	RI_MAX_I_HOUR,						//前一天最大瞬时流量，发生的时
	RI_MAX_I_MINUTE,					//前一天最大瞬时流量，发生的分
	RI_MINIMUM_INSTANTANEOUS,			//前一天最小瞬时流量
	RI_MIN_I_HOUR,						//前一天最小瞬时流量，发生的时
	RI_MIN_I_MINUTE,					//前一天最小瞬时流量，发生的分
	RI_THE_DAY_BEFORE_DATE,				//前一天日期 年月日
	RI_COMMUNICATION_SUCCESS_RATE,		//当前通讯成功率
	RI_ALL_PRESSURE,					//前一天所有压力数据，数组
	RI_ALL_TEMPERATURE					//前一天所有温度 ,数组
	*/
	MeterData ^md = gcnew MeterData();
	md->MeterCode = "123456";
	md->MeterStatus = 0;
	md->TotalFlow = 8852;
	md->PressValue = 1;
	md->PressStatus = 1;
	md->SourceMaxFlow = 55;
	DateTime ^date = gcnew DateTime(2016, 3, 11, 14, 42, 55);
	md->MaxFlowTime = *date;
	md->SourceMinFlow = 33;
	date = gcnew DateTime(2016, 3, 11, 16, 22, 55);
	md->MinFlowTime = *date;
	md->CopyDate = "2016-3-11";
	md->PressRecord = "33,22,66,33,33,55";
	md->TempRecord = "55,66,44,33,22";
	List<MeterData^> ^listData = gcnew List<MeterData^>;
	listData->Add(md);


	Session ^se = gcnew Session;
	if (se->Open())
	{
		se->SaveMeterRecords(listData);
		se->Close();
	}
	else{
		//MessageBox(NULL, L"打开数据库失败", NULL, MB_OK);
	}


	

	
	////Persistent p;
	//String ^strErr = "";
	//if (!p.Open(strErr))
	//{

	//}
	//wchar_t *pErr = (wchar_t*)(void*)Marshal::StringToHGlobalUni(strErr);

	//MessageBox(NULL, pErr, NULL, MB_OK);

}