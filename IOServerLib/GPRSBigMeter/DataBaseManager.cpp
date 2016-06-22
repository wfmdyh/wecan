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



//开始
BOOL CDataBaseManager::Start()
{
	//m_hSave = (HANDLE)_beginthreadex(NULL, 0, ThreadSave, this, 0, NULL);
	//加载数据库动态库
	/*m_hDBLib = LoadLibrary(L"BigMeterDB");
	if (m_hDBLib == NULL)
	{
		MessageBox(NULL, L"BigMeterDB.dll未找到", L"错误", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	m_SaveDataToDB = (fnSaveDataToDB)GetProcAddress(m_hDBLib, "SaveDataToDB");
	if (m_SaveDataToDB == NULL)
	{
		return FALSE;
	}*/

	
	

	return TRUE;
}

//停止
BOOL CDataBaseManager::Stop()
{
	
	/*if (m_hDBLib != NULL)
	{
		FreeLibrary(m_hDBLib);
		m_hDBLib = NULL;
	}*/

	return TRUE;
}

//加载配置文件
BOOL CDataBaseManager::LoadConfig()
{
	wifstream configFile(L"C:\\IOServerConfig.cfg");
	if (!configFile.is_open())
	{
		cout << "无法打开数据库配置文件" << endl;
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
//更新想要存储的数据
BOOL CDataBaseManager::UpdateMeters(CProtocolManager* pProtocolMgr)
{
	vector<CGPRSBigMeter> vecBigMeters = pProtocolMgr->GetBigMeters();
	vector<DevConcentrator> vecConcentrators = pProtocolMgr->GetConcentrators();
	if (!LoadConfig())
	{
		return FALSE;
	}
	//打开数据库
	if (!m_dbOracle.ConnectSQLServer(m_IP, m_PORT, m_DBNAME, m_DBUSER, m_DBPWD))
	{
		return FALSE;
	}
	if (!UpdateBigMeterData(vecBigMeters))
	{
		m_dbOracle.CloseSQLServer();
		return FALSE;
	}
	//集中器数据
	if (!UpdateConcentratorData(vecConcentrators))
	{
		m_dbOracle.CloseSQLServer();
		return FALSE;
	}
	//关闭连接
	m_dbOracle.CloseSQLServer();
	return TRUE;
}

//存储大表数据
BOOL CDataBaseManager::UpdateBigMeterData(vector<CGPRSBigMeter> vecBigMeters)
{
	BOOL bResult = TRUE;
	wostringstream wss;
	wss << L"当前大表设备数量：";
	wss << vecBigMeters.size();
	wss << L"\n";
	wstring strMsg = wss.str();
	OutputDebugString(strMsg.c_str());

	//vector<CGPRSBigMeter> vecSave;
	//需要保存只有更新过的数据才进行存储
	vector<CGPRSBigMeter>::iterator ite = vecBigMeters.begin();
	for (; ite != vecBigMeters.end(); ite++)
	{
		vector<CGPRSBigMeter>::iterator iteNewMeter = find_if(m_listMeters.begin(), m_listMeters.end(), MeterCompare(ite->GetDeviceID()));
		if (iteNewMeter == m_listMeters.end())
		{
			//新设备
			m_listMeters.push_back(*ite);
			UpdateOne(*ite);
		}
		else{
			//判断时间戳是否改变过，改变则存储
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
			//存在
			break;
		}
	}
	//不存在
	return iteList;
}

//存储集中器数据
BOOL CDataBaseManager::UpdateConcentratorData(vector<DevConcentrator> vecCon)
{
	vector<DevConcentrator> vecSaves;
	auto iteNew = vecCon.begin();
	for (; iteNew != vecCon.end(); iteNew++)
	{
		//判断是否历史列表中
		auto iteHistory = ConHistoryExist(*iteNew);
		if (iteHistory == m_listConcentrators.end())
		{
			m_listConcentrators.push_back(*iteNew);
			//添加到存储列表中
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
	//往临时表中插入数据
	m_dbOracle.BeginTrans();
	if (!m_dbOracle.UpdateConSMTempTable(vecSaves))
	{
		cout << "更新临时表失败" << endl;
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	cout << "更新小表临时表" << endl;
	//执行存储过程
	if (!m_dbOracle.ExecSaveSmallMeter())
	{
		cout << "执行保存小表存储过程失败" << endl;
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	cout << "执行小表存储过程" << endl;
	return m_dbOracle.CommitTrans();
}

BOOL CDataBaseManager::UpdateOne(CGPRSBigMeter meter)
{
	wstring strDevID = ANSIToUnicode(meter.GetDeviceID());
	//开始事务
	m_dbOracle.BeginTrans();
	//实时数据
	BOOL bResult = m_dbOracle.InsertRealMeterData(meter);
	if (!bResult)
	{
		m_dbOracle.RollbackTrans();
		return FALSE;
	}
	//历史数据
	int nRet = m_dbOracle.QueryRecordExist(strDevID, meter.GetBeforeDateString());
	switch (nRet)
	{
	case -1:
		//出错了
		m_dbOracle.RollbackTrans();
		return FALSE;
		break;
	case 0:
		//新记录
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
	
	//提交事务
	return m_dbOracle.CommitTrans();
}