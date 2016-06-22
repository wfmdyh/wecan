#include "stdafx.h"
#include "DatabaseMgr.h"
#include "DevPipeNet.h"
#include "DevPressureAlarm.h"
#include "DevMultipleParam.h"
#include "DataDef.h"


DatabaseMgr::DatabaseMgr()
{

}

//�����ݿ�����
bool DatabaseMgr::OpenDB()
{
	m_DBSession = gcnew Session();
	return m_DBSession->Open();
}

//�ر����ݿ�����
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
		//ѹ������ �����
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
			wss << L"�洢������������ݣ���ţ�" << pDev->m_DeviceID;
			strMsg = wss.str();
			ShowMessage(strMsg.c_str());
			strMsg = L"";
			wss.str(L"");
			wss.clear();
			SaveAlarm(alarm);
		}
		//ѹ������ �±�
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
				//ѹ����λ���� 0.01MPa������ֵ90�����ʾ0.9MPa
				//double db = (double)((double)(pNewAlarm->m_Pressure) / 1000);
				double db = (double)pNewAlarm->m_Pressure;
				db = db * 0.01;
				alarm->AlarmValue = db;
				wss << L"�洢�±������ݣ���ţ�" << pDev->m_DeviceID;
				strMsg = wss.str();
				ShowMessage(strMsg.c_str());
				strMsg = L"";
				wss.str(L"");
				wss.clear();
				SaveAlarm(alarm);
			}
		}
		
		//�洢����
		if (pDev->m_ArrMultipleParam.size() > 0)
		{
			//�±�����
			vector<DevMultipleParam*>::iterator iteMultipleParam = pDev->m_ArrMultipleParam.begin();
			for (; iteMultipleParam != pDev->m_ArrMultipleParam.end(); iteMultipleParam++)
			{
				CollectData^ dev = gcnew CollectData();

				DevMultipleParam* pMP = *iteMultipleParam;

				//���ʱ��
				SYSTEMTIME deviceTime = pMP->m_DateTime;
				DateTime^ colTime = gcnew DateTime(deviceTime.wYear, deviceTime.wMonth, deviceTime.wDay, deviceTime.wHour, deviceTime.wMinute, deviceTime.wSecond);
				dev->DeviceTime = *colTime;

				/*wstring paramID = pMP->GetDeviceIDStr();
				dev->DeviceCode = marshal_as<String^>(paramID.c_str());*/
				dev->DeviceCode = strID;
				string stdstrValue;
				stringstream ss;

				//��״̬
				ss << (int)pMP->m_State;
				stdstrValue = ss.str();
				String^ strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data1 = strValue;
				ss.str("");
				ss.clear();

				//С����״̬
				ss << (int)pDev->m_SmallState;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data2 = strValue;
				ss.str("");
				ss.clear();

				//ѹ��״̬
				ss << (int)pMP->m_CommunicationState;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data3 = strValue;
				ss.str("");
				ss.clear();

				//�¶�ֵ
				float fTemp = pMP->m_Temperature * 0.1f;
				ss << fTemp;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data4 = strValue;
				ss.str("");
				ss.clear();

				//ѹ��ֵ����λ 10KPa
				double dbPressValue = pMP->m_Pressure * 10;
				dbPressValue /= 1000;
				ss << dbPressValue;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data5 = strValue;
				ss.str("");
				ss.clear();

				//�ۼ�����
				//���ۼ�����
				ss << pMP->m_PositiveTotalFlow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data7 = strValue;
				ss.str("");
				ss.clear();

				//���ۼ�����
				ss << pMP->m_NegativeTotalFlow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data9 = strValue;
				ss.str("");
				ss.clear();

				//˲ʱ����
				ss << pMP->m_Flow;
				stdstrValue = ss.str();
				strValue = marshal_as<String^>(stdstrValue.c_str());
				dev->Data8 = strValue;
				ss.str("");
				ss.clear();

				wss << L"�洢�±����ݣ���ţ�" << pDev->m_DeviceID;
				strMsg = wss.str();
				ShowMessage(strMsg.c_str());
				strMsg = L"";
				wss.str(L"");
				wss.clear();
				//�������ݿ�
				SaveDevice(dev);
			}
		}
		else{
			CollectData^ dev = gcnew CollectData();
			//���ʱ��
			SYSTEMTIME devTime = pDev->m_CurrentTime;
			DateTime^ deviceTime = gcnew DateTime(devTime.wYear, devTime.wMonth, devTime.wDay, devTime.wHour, devTime.wMinute, devTime.wSecond);
			dev->DeviceTime = *deviceTime;
			//�ϱ�����
			string stdstrValue;
			stringstream ss;
			
			dev->DeviceCode = strID;

			//��״̬
			ss << (int)pDev->m_State;
			stdstrValue = ss.str();
			String^ strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data1 = strValue;
			ss.str("");
			ss.clear();

			//С����״̬
			ss << (int)pDev->m_SmallState;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data2 = strValue;
			ss.str("");
			ss.clear();

			//ѹ��״̬
			ss << (int)pDev->m_PAState;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data3 = strValue;
			ss.str("");
			ss.clear();

			//�¶�ֵ
			ss << pDev->m_Temperature;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data4 = strValue;
			ss.str("");
			ss.clear();

			//�¶�ֵ����
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
			//��ֵ�Ŵ�
			if (arrStrTemp.size() > 0)
			{
				dev->Data16 = marshal_as<String^>(arrStrTemp.c_str());
			}
			wss.str(L"");
			wss.clear();

			//ѹ��ֵ����
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

			//ѹ��ֵ
			ss << (pDev->m_Pressure / 1000);
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data5 = strValue;
			ss.str("");
			ss.clear();

			//�ۼ�����
			ss << pDev->m_QuantityOfHeat;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data6 = strValue;
			ss.str("");
			ss.clear();

			//���ۼ�����
			ss << pDev->m_TotalFlow;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data7 = strValue;
			ss.str("");
			ss.clear();

			//˲ʱ����
			ss << pDev->m_Flow;
			stdstrValue = ss.str();
			strValue = marshal_as<String^>(stdstrValue.c_str());
			dev->Data8 = strValue;
			ss.str("");
			ss.clear();


			wss << L"�洢��������ݣ���ţ�" << pDev->m_DeviceID;
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

//���汨����Ϣ
void DatabaseMgr::SaveAlarm(AlarmData^ alarm)
{
	wstringstream wss;
	if (!m_DBSession->SaveDeviceAlarm(alarm))
	{
		wss << L"������Ϣ�洢ʧ�ܣ�";
		wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(m_DBSession->Error);
		wss << wcsMsg;
		ShowMessage(wss.str().c_str());
		OutputDebugString(L"\n");
		OutputDebugString(wcsMsg);
		OutputDebugString(L"\n");
	}
}

//�����豸����
void DatabaseMgr::SaveDevice(CollectData^ dev)
{
	wstringstream wss;
	if (!m_DBSession->SaveDeviceData(dev))
	{
		wchar_t *wcsMsg = (wchar_t*)(void*)Marshal::StringToHGlobalUni(m_DBSession->Error);
		wss << L"\r\n���ݿ�洢����";
		wss << wcsMsg;
		ShowMessage(wss.str().c_str());
		OutputDebugString(L"\n");
		OutputDebugString(wcsMsg);
		OutputDebugString(L"\n");
	}
}