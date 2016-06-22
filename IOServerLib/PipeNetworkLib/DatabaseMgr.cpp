#include "stdafx.h"
#include "DatabaseMgr.h"
#include "DevPipeNet.h"
#include "DevPressureAlarm.h"
#include "DevMultipleParam.h"
#include "DataDef.h"


DatabaseMgr::DatabaseMgr()
{

}

//打开数据库连接
bool DatabaseMgr::OpenDB()
{
	m_DBSession = gcnew Session();
	return m_DBSession->Open();
}

//关闭数据库连接
bool DatabaseMgr::CloseDB()
{
	return m_DBSession->Close();
}

bool DatabaseMgr::SaveData(list<DevPipeNet*> arrDev)
{
	if (arrDev.size() < 1)
	{
		return true;
	}
	wstringstream wss;
	wstring strMsg;

	list<DevPipeNet*>::iterator ite = arrDev.begin();
	for (; ite != arrDev.end(); ite++)
	{
		DevPipeNet* pDev = *ite;
		String^ strID = marshal_as<String^>(pDev->m_DeviceID.c_str());
		//压力报警 多参数
		if (pDev->m_PAState != 0xFF)
		{
			AlarmData^ alarm = gcnew AlarmData();
			alarm->AlarmTime = DateTime().Now;
			alarm->DeviceCode = strID;
			alarm->AlarmType = pDev->m_PAState;
			double db = (double)pDev->m_PAValue;
			db = db / 1000;
			//double db = (double)((double)(pDev->m_PAValue) / 1000);
			alarm->AlarmValue = db;
			wss << L"存储多参数报警数据，表号：" << pDev->m_DeviceID;
			strMsg = wss.str();
			ShowMessage(strMsg.c_str());
			strMsg = L"";
			wss.str(L"");
			wss.clear();
			SaveAlarm(alarm);
		}
		//压力报警 新表
		if (pDev->m_ArrPressureAlarm.size() > 0)
		{
			vector<DevPressureAlarm*>::iterator iteNewAlarm = pDev->m_ArrPressureAlarm.begin();
			for (; iteNewAlarm != pDev->m_ArrPressureAlarm.end(); iteNewAlarm++)
			{
				DevPressureAlarm* pNewAlarm = *iteNewAlarm;
				
				AlarmData^ alarm = gcnew AlarmData();
				DateTime^ alarmTime = gcnew DateTime(pNewAlarm->m_Time.wYear, pNewAlarm->m_Time.wMonth, pNewAlarm->m_Time.wDay, pNewAlarm->m_Time.wHour, pNewAlarm->m_Time.wMinute, 0);
				alarm->AlarmTime = *alarmTime;
				/*wstring alarmID = pNewAlarm->GetDeviceIDStr();
				alarm->DeviceCode = marshal_as<String^>(alarmID.c_str());*/
				alarm->DeviceCode = strID;
				alarm->AlarmType = pNewAlarm->m_State;
				//压力单位修正 0.01MPa，例如值90，则表示0.9MPa
				//double db = (double)((double)(pNewAlarm->m_Pressure) / 1000);
				double db = (double)pNewAlarm->m_Pressure;
				db = db * 0.01;
				alarm->AlarmValue = db;
				wss << L"存储新表报警数据，表号：" << pDev->m_DeviceID;
				strMsg = wss.str();
				ShowMessage(strMsg.c_str());
				strMsg = L"";
				wss.str(L"");
				wss.clear();
				SaveAlarm(alarm);
			}
		}
		
		//存储数据
		if (pDev->m_ArrMultipleParam.size() > 0)
		{
			//新表数据
			vector<DevMultipleParam*>::iterator iteMultipleParam = pDev->m_ArrMultipleParam.begin();
			for (; iteMultipleParam != pDev->m_ArrMultipleParam.end(); iteMultipleParam++)
			{
				CollectData^ dev = gcnew CollectData();

				DevMultipleParam* pMP = *iteMultipleParam;

				//表具时间
				SYSTEMTIME deviceTime = pMP->m_DateTime;
				DateTime^ colTime = gcnew DateTime(deviceTime.wYear, deviceTime.wMonth, deviceTime.wDay, deviceTime.wHour, deviceTime.wMinute, deviceTime.wSecond);
				dev->DeviceTime = *colTime;

				/*wstring paramID = pMP->GetDeviceIDStr();
				dev->DeviceCode = marshal_as<String^>(paramID.c_str());*/
				dev->DeviceCode = strID;
				string stdstrValue;
				stringstream ss;

				//表状态
				ss << (int)pMP->m_State;
				stdstrValue = ss.str();
				String^ strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data1 = strValue;
				ss.str("");
				ss.clear();

				//小无线状态
				ss << (int)pDev->m_SmallState;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data2 = strValue;
				ss.str("");
				ss.clear();

				//压力状态
				ss << (int)pMP->m_CommunicationState;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data3 = strValue;
				ss.str("");
				ss.clear();

				//温度值
				float fTemp = pMP->m_Temperature * 0.1f;
				ss << fTemp;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data4 = strValue;
				ss.str("");
				ss.clear();

				//压力值，单位 10KPa
				double dbPressValue = pMP->m_Pressure * 10;
				dbPressValue /= 1000;
				ss << dbPressValue;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data5 = strValue;
				ss.str("");
				ss.clear();

				//累计热量
				//正累计流量
				ss << pMP->m_PositiveTotalFlow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data7 = strValue;
				ss.str("");
				ss.clear();

				//负累计流量
				ss << pMP->m_NegativeTotalFlow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data9 = strValue;
				ss.str("");
				ss.clear();

				//瞬时流量
				ss << pMP->m_Flow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data8 = strValue;
				ss.str("");
				ss.clear();

				wss << L"存储新表数据，表号：" << pDev->m_DeviceID;
				strMsg = wss.str();
				ShowMessage(strMsg.c_str());
				strMsg = L"";
				wss.str(L"");
				wss.clear();
				//保存数据库
				SaveDevice(dev);
			}
		}
		else{
			CollectData^ dev = gcnew CollectData();
			//表具时间
			SYSTEMTIME devTime = pDev->m_CurrentTime;
			DateTime^ deviceTime = gcnew DateTime(devTime.wYear, devTime.wMonth, devTime.wDay, devTime.wHour, devTime.wMinute, devTime.wSecond);
			dev->DeviceTime = *deviceTime;
			//老表数据
			string stdstrValue;
			stringstream ss;
			
			dev->DeviceCode = strID;

			//表状态
			ss << (int)pDev->m_State;
			stdstrValue = ss.str();
			String^ strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data1 = strValue;
			ss.str("");
			ss.clear();

			//小无线状态
			ss << (int)pDev->m_SmallState;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data2 = strValue;
			ss.str("");
			ss.clear();

			//压力状态
			ss << (int)pDev->m_PAState;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data3 = strValue;
			ss.str("");
			ss.clear();

			//温度值
			ss << pDev->m_Temperature;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data4 = strValue;
			ss.str("");
			ss.clear();

			//温度值数组
			vector<WORD>::iterator iteTemp = pDev->m_ArrTemperature.begin();
			wstringstream wss;
			if (pDev->m_ArrTemperature.size() > 0)
			{
				wss << *iteTemp;
				iteTemp++;
			}
			for (; iteTemp != pDev->m_ArrTemperature.end(); iteTemp++)
			{
				WORD wTemp = *iteTemp;
				wss << L"," << wTemp;
			}
			wstring arrStrTemp = wss.str();
			//有值才传
			if (arrStrTemp.size() > 0)
			{
				dev->Data16 = marshal_as<String^>(arrStrTemp.c_str());
			}
			wss.str(L"");
			wss.clear();

			//压力值数组
			vector<WORD>::iterator itePress = pDev->m_ArrPressure.begin();
			if (pDev->m_ArrPressure.size() > 0)
			{
				wss << *itePress;
				itePress++;
			}
			for (; itePress != pDev->m_ArrPressure.end(); itePress++)
			{
				wss << L"," << *itePress;
			}
			wstring arrStrPress = wss.str();
			if (arrStrPress.size() > 0)
			{
				dev->Data17 = marshal_as<String^>(arrStrPress.c_str());
			}

			//压力值
			ss << (pDev->m_Pressure / 1000);
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data5 = strValue;
			ss.str("");
			ss.clear();

			//累计热量
			ss << pDev->m_QuantityOfHeat;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data6 = strValue;
			ss.str("");
			ss.clear();

			//正累计流量
			ss << pDev->m_TotalFlow;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data7 = strValue;
			ss.str("");
			ss.clear();

			//瞬时流量
			ss << pDev->m_Flow;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data8 = strValue;
			ss.str("");
			ss.clear();


			wss << L"存储多参数数据，表号：" << pDev->m_DeviceID;
			strMsg = wss.str();
			ShowMessage(strMsg.c_str());
			strMsg = L"";
			wss.str(L"");
			wss.clear();
			SaveDevice(dev);
		}
		
		delete pDev;
		
	}
	arrDev.clear();
	
	return true;
}

//保存报警信息
void DatabaseMgr::SaveAlarm(AlarmData^ alarm)
{
	wstringstream wss;
	if (!m_DBSession->SaveDeviceAlarm(alarm))
	{
		wss << L"报警信息存储失败：";
		wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(m_DBSession->Error);
		wss << wcsMsg;
		ShowMessage(wss.str().c_str());
		OutputDebugString(L"\n");
		OutputDebugString(wcsMsg);
		OutputDebugString(L"\n");
	}
}

//保存设备数据
void DatabaseMgr::SaveDevice(CollectData^ dev)
{
	wstringstream wss;
	if (!m_DBSession->SaveDeviceData(dev))
	{
		wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(m_DBSession->Error);
		wss << L"\r\n数据库存储错误：";
		wss << wcsMsg;
		ShowMessage(wss.str().c_str());
		OutputDebugString(L"\n");
		OutputDebugString(wcsMsg);
		OutputDebugString(L"\n");
	}
}