#include "stdafx.h"
#include "DataBaseManager.h"
#include "IOCPModel.h"
#include "MyTools.h"

CDataBaseManager::CDataBaseManager()
{
	m_FlagShutdown = TRUE;
	m_hDBLib = NULL;
	//m_SaveDataToDB = NULL;
}


CDataBaseManager::~CDataBaseManager()
{
}



//��ʼ
BOOL CDataBaseManager::Start()
{
	//m_hSave = (HANDLE)_beginthreadex(NULL, 0, ThreadSave, this, 0, NULL);
	//�������ݿ⶯̬��
	/*m_hDBLib = LoadLibrary(L"BigMeterDB");
	if (m_hDBLib == NULL)
	{
		MessageBox(NULL, L"BigMeterDB.dllδ�ҵ�", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	m_SaveDataToDB = (fnSaveDataToDB)GetProcAddress(m_hDBLib, "SaveDataToDB");
	if (m_SaveDataToDB == NULL)
	{
		return FALSE;
	}*/

	
	

	return TRUE;
}

//ֹͣ
BOOL CDataBaseManager::Stop()
{
	
	/*if (m_hDBLib != NULL)
	{
		FreeLibrary(m_hDBLib);
		m_hDBLib = NULL;
	}*/

	return TRUE;
}

//���������ļ�
BOOL CDataBaseManager::LoadConfig()
{
	wifstream configFile(L"C:\\IOServerConfig.cfg");
	if (!configFile.is_open())
	{
		cout << "�޷������ݿ������ļ�" << endl;
		return FALSE;
	}
	while (!configFile.eof())
	{
		wchar_t wcsLine[512] = { 0 };
		configFile.getline(wcsLine, 512);
		wstring strLine(wcsLine);
		int nFlag = strLine.find(L":");
		wstring strKey = strLine.substr(0, nFlag);
		wstring strValue = strLine.substr(nFlag + 1);
		if (strKey.compare(L"IP") == 0)
		{
			m_IP = strValue;
		}
		else if (strKey.compare(L"PORT") == 0)
		{
			m_PORT = strValue;
		}
		else if (strKey.compare(L"DBNAME") == 0)
		{
			m_DBNAME = strValue;
		}
		else if (strKey.compare(L"DBUSER") == 0)
		{
			m_DBUSER = strValue;
		}
		else if (strKey.compare(L"DBPWD") == 0)
		{
			m_DBPWD = strValue;
		}
	}


	return TRUE;
}

//vector<CGPRSBigMeter> vecSave;
//������Ҫ�洢������
BOOL CDataBaseManager::UpdateMeters(CProtocolManager* pProtocolMgr)
{
	vector<CGPRSBigMeter> vecBigMeters = pProtocolMgr->GetBigMeters();
	vector<DevConcentrator> vecConcentrators = pProtocolMgr->GetConcentrators();
	if (!LoadConfig())
	{
		return FALSE;
	}
	//�����ݿ�
	if (!m_dbOracle.ConnectSQLServer(m_IP, m_PORT, m_DBNAME, m_DBUSER, m_DBPWD))
	{
		return FALSE;
	}
	if (!UpdateBigMeterData(vecBigMeters))
	{
		m_dbOracle.CloseSQLServer();
		return FALSE;
	}
	//����������
	if (!UpdateConcentratorData(vecConcentrators))
	{
		m_dbOracle.CloseSQLServer();
		return FALSE;
	}
	//�ر�����
	m_dbOracle.CloseSQLServer();
	return TRUE;
}

//�洢�������
BOOL CDataBaseManager::UpdateBigMeterData(vector<CGPRSBigMeter> vecBigMeters)
{
	BOOL bResult = TRUE;
	wostringstream wss;
	wss << L"��ǰ����豸������";
	wss << vecBigMeters.size();
	wss << L"\n";
	wstring strMsg = wss.str();
	OutputDebugString(strMsg.c_str());

	//vector<CGPRSBigMeter> vecSave;
	//��Ҫ����ֻ�и��¹������ݲŽ��д洢
	vector<CGPRSBigMeter>::iterator ite = vecBigMeters.begin();
	for (; ite != vecBigMeters.end(); ite++)
	{
		vector<CGPRSBigMeter>::iterator iteNewMeter = find_if(m_listMeters.begin(), m_listMeters.end(), MeterCompare(ite->GetDeviceID()));
		if (iteNewMeter == m_listMeters.end())
		{
			//���豸
			m_listMeters.push_back(*ite);
			UpdateOne(*ite);
		}
		else{
			//�ж�ʱ����Ƿ�ı�����ı���洢
			time_t tNew = ite->GetLastTime();
			time_t tOld = iteNewMeter->GetLastTime();
			if (tNew > tOld)
			{
				UpdateOne(*ite);
				iteNewMeter->SetLastTime(tNew);
			}
		}
	}

	return bResult;
}

vector<DevConcentrator>::iterator CDataBaseManager::ConHistoryExist(DevConcentrator concentrator)
{
	auto iteList = m_listConcentrators.begin();
	for (; iteList != m_listConcentrators.end(); iteList++)
	{
		if (iteList->m_DeviceID == concentrator.m_DeviceID)
		{
			//����
			break;
		}
	}
	//������
	return iteList;
}

//�洢����������
BOOL CDataBaseManager::UpdateConcentratorData(vector<DevConcentrator> vecCon)
{
	vector<DevConcentrator> vecSaves;
	auto iteNew = vecCon.begin();
	for (; iteNew != vecCon.end(); iteNew++)
	{
		//�ж��Ƿ���ʷ�б���
		auto iteHistory = ConHistoryExist(*iteNew);
		if (iteHistory == m_listConcentrators.end())
		{
			m_listConcentrators.push_back(*iteNew);
			//��ӵ��洢�б���
			vecSaves.push_back(*iteNew);
		}
		else{
			time_t tNew = iteNew->GetLastTime();
			time_t tHistory = iteHistory->GetLastTime();
			if (tNew > tHistory)
			{
				vecSaves.push_back(*iteNew);
				iteHistory->SetLastTime(tNew);
			}
		}
	}
	if (vecSaves.size() < 1)
	{
		return TRUE;
	}
	//����ʱ���в�������
	m_dbOracle.BeginTrans();
	if (!m_dbOracle.UpdateConSMTempTable(vecSaves))
	{
		cout << "������ʱ��ʧ��" << endl;
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	cout << "����С����ʱ��" << endl;
	//ִ�д洢����
	if (!m_dbOracle.ExecSaveSmallMeter())
	{
		cout << "ִ�б���С��洢����ʧ��" << endl;
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	cout << "ִ��С��洢����" << endl;
	return m_dbOracle.CommitTrans();
}

BOOL CDataBaseManager::UpdateOne(CGPRSBigMeter meter)
{
	wstring strDevID = ANSIToUnicode(meter.GetDeviceID());
	//��ʼ����
	m_dbOracle.BeginTrans();
	//ʵʱ����
	BOOL bResult = m_dbOracle.InsertRealMeterData(meter);
	if (!bResult)
	{
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	//��ʷ����
	int nRet = m_dbOracle.QueryRecordExist(strDevID, meter.GetBeforeDateString());
	switch (nRet)
	{
	case -1:
		//������
		m_dbOracle.RollbackTrans();
		return FALSE;
		break;
	case 0:
		//�¼�¼
		if (!m_dbOracle.InsertMeterRecord(meter))
		{
			m_dbOracle.RollbackTrans();
			return FALSE;
		}
		break;
	default:
		if (!m_dbOracle.UpdateMeterRecord(meter))
		{
			m_dbOracle.RollbackTrans();
			return FALSE;
		}
		break;
	}
	
	//�ύ����
	return m_dbOracle.CommitTrans();
}