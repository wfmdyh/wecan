// OPCServer.cpp : ���� DLL Ӧ�ó���ĵ���������
//
#include "stdafx.h"
#include "WTOPCsvrAPI.h"
#include "OPCServer.h"
#include "MyTools.h"
#include "TransmitManager.h"

//{70EFA124-0DE1-4484-B058-6CF4BD965613}
const CLSID CLSID_WFM_SVR =
{ 0x0CD596C3, 0x3C68, 0x4216,
{ 0x85, 0xD4, 0xD2, 0x74, 0xC9, 0xCF, 0x55, 0x83 } };
#define SERVER_RATE	100
#define SVR_NAME		"wecan.opcs"
#define SVR_DESC		"wecan OPC Server"


COPCServer::COPCServer()
{

}

COPCServer::~COPCServer()
{

}

//��ʼ��
BOOL COPCServer::InitServer()
{
	int retval = 0;//Result of calling function
	LPSTR NumberSerial = "JVRPS53R5V64226N62H4";
	retval = Deactivate30MinTimer(NumberSerial);
	//��ʼ��
	return InitWTOPCsvr((BYTE*)&CLSID_WFM_SVR, SERVER_RATE);
}

/*����tag
pVecTraData: �������ļ��н�������������
*/
BOOL COPCServer::CreateTags()
{
	vector<TransmitChannel*> vecTraChannel = m_TraMgr->GetVecChannel();
	vector<TransmitChannel*>::iterator iteTraChannel = vecTraChannel.begin();
	for (; iteTraChannel != vecTraChannel.end(); iteTraChannel++)
	{
		TransmitChannel *pTraChannel = *iteTraChannel;
		vector<TransmitPoint*> vecTraPoint = pTraChannel->GetVecPoint();
		vector<TransmitPoint*>::iterator iteTraPoint = vecTraPoint.begin();
		for (; iteTraPoint != vecTraPoint.end(); iteTraPoint++)
		{
			OPCTag *pTag = new OPCTag;
			TransmitPoint *pTraPoint = *iteTraPoint;
			pTag->m_strName = pTraPoint->m_strName;

			pTag->m_hTag = CreateTag(pTag->m_strName.c_str(), pTag->m_varValue, pTag->m_wQuality, pTag->m_bIsWritable);
			m_vecTags.push_back(pTag);
		}

	}
	return TRUE;
}



void __stdcall OPCS_WriteProc(HANDLE hTag, VARIANT* varData, DWORD* dwResult)
{
	/*for (unsigned int i=0;i<g_pVecTag->size();i++)
	{
	TAG_STRUCT ts = g_pVecTag->at(i);
	if (ts.hTag == hTag)
	{
	wstring strVal;
	MapVariantToStringVal(*varData,strVal);
	IOS_WriteData(ANSIToUnicode(ts.astrTraChannel),
	ANSIToUnicode(ts.astrTraDevice),
	ANSIToUnicode(ts.astrTraPoint),strVal);
	}
	}*/


}

void COPCServer::SetTransmit(TransmitManager *pTransmitMgr)
{
	m_TraMgr = pTransmitMgr;
}

//ת������
BOOL COPCServer::TransmitData()
{
	//��ֵ �ⲿ�Ѿ��ı������ֵ��
	vector<TransmitChannel*> vecTraChannel = m_TraMgr->GetVecChannel();

	StartUpdateTags();
	//������ǩ
	vector<OPCTag*>::iterator iteTag = m_vecTags.begin();
	for (; iteTag != m_vecTags.end(); iteTag++)
	{
		OPCTag *pTag = *iteTag;
		//ƥ��ת��ͨ��
		vector<TransmitChannel*>::iterator iteTra = vecTraChannel.begin();
		for (; iteTra != vecTraChannel.end(); iteTra++)
		{
			TransmitChannel *pTraChannel = *iteTra;
			vector<TransmitPoint*> vecTraPoint = pTraChannel->GetVecPoint();
			vector<TransmitPoint*>::iterator iteTraPoint = vecTraPoint.begin();
			for (; iteTraPoint != vecTraPoint.end(); iteTraPoint++)
			{
				TransmitPoint *pTraPoint = *iteTraPoint;
				if (pTag->m_strName.compare(pTraPoint->m_strName) == 0)
				{
					//ƥ���ϣ�ת��
					VARIANT var = pTraPoint->GetVarValue();
					if (var.vt != VT_EMPTY)
					{
						pTag->m_wQuality = OPC_QUALITY_GOOD;
					}
					else{
						pTag->m_wQuality = OPC_QUALITY_BAD;
					}
					BOOL bRet = UpdateTagToList(pTag->m_hTag, var, pTag->m_wQuality);
					break;
				}
			}
		}

	}


	EndUpdateTags();
	return TRUE;
}

//����Server
BOOL COPCServer::Start()
{
	//��ʼ��
	if (!InitServer())
	{
		return FALSE;
	}
	////����д�뺯��
	//if(!EnableWriteNotification(OPCS_WriteProc,TRUE))
	//{
	//	return FALSE;
	//}
	//����tag����ʼ��
	if (!CreateTags())
	{
		return FALSE;
	}
	return TRUE;
}

void COPCServer::Stop()
{
	vector<OPCTag*>::iterator iteTag = m_vecTags.begin();
	for (; iteTag != m_vecTags.end(); iteTag++)
	{
		delete *iteTag;
	}
	m_vecTags.clear();

	UninitWTOPCsvr();
	OutputDebugString(L"OPC Server�˳�\n");
}

//ע��OPCServer
//szAppExePathӦ�ó����ȫ·���������ļ���������ƣ��˺���ֻ�����һ�Σ�ʹ��ʱ��ע��
//����ֵ���ɹ�����TRUE
BOOL COPCServer::RegOPCServer(LPCTSTR wszAppExePath)
{
	//��ʼ��
	if (!InitServer())
	{
		return FALSE;
	}

	string strAppExePath = UnicodeToANSI(wszAppExePath);
	BOOL bRet = UpdateRegistry((BYTE *)&CLSID_WFM_SVR,
		SVR_NAME,
		SVR_DESC,
		strAppExePath.c_str());
	UninitWTOPCsvr();

	return bRet;
}

//��ע��OPCServer
BOOL COPCServer::UnRegOPCServer()
{
	if (!InitServer())
	{
		return FALSE;
	}
	BOOL bRet = UnregisterServer((BYTE *)&CLSID_WFM_SVR, SVR_NAME);
	UninitWTOPCsvr();
	return bRet;
}

void SetValue(string strtype, VARIANT &value, const char* chvalue)
{
	VariantInit(&value);
	if (strtype == "VT_I1")
	{
		value.vt = VT_I1;
		value.cVal = atoi(chvalue);
	}
	else if (strtype == "VT_I2")
	{
		value.vt = VT_I2;
		value.iVal = atoi(chvalue);
	}
	else if (strtype == "VT_I4")
	{
		value.vt = VT_I4;
		value.lVal = atol(chvalue);
	}
	else if (strtype == "VT_BOOL")
	{
		value.vt = VT_BOOL;
		value.boolVal = atoi(chvalue);
	}
	else if (strtype == "VT_UI1")
	{
		value.vt = VT_UI1;
		value.bVal = (BYTE)strtoul(chvalue, NULL, 0);
	}
	else if (strtype == "VT_R4")
	{
		value.vt = VT_R4;
		value.fltVal = (float)atof(chvalue);
	}
	else if (strtype == "VT_R8")
	{
		value.vt = VT_R8;
		value.dblVal = atof(chvalue);
	}
	else if (strtype == "VT_BSTR")
	{
		value.vt = VT_BSTR;
		WCHAR wchBuffer[1024];
		// String is not in Unicode format, so convert and place result into buffer:
		if (!MultiByteToWideChar(CP_ACP, 0, chvalue, -1, wchBuffer, 1024))
		{
			return;
		}
		value.bstrVal = ::SysAllocString(wchBuffer);
	}
	else
	{
		value.vt = VT_UI4;
		value.ulVal = strtoul(chvalue, NULL, 0);
	}
}