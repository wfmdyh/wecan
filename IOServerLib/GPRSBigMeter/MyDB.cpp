#include "stdafx.h"
#include "MyDB.h"
#include "GPRSBigMeter.h"
#include "MyTools.h"

CMyDB::CMyDB()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	//超时时间 默认5秒
	m_TimeOut = 5;
	_CommandPtr pCommand;			//返回一个记录集,提供了一种简单的方法来执行返回记录集的存储过程和SQL语句
}


CMyDB::~CMyDB()
{
	CoUninitialize();
}

BOOL CMyDB::ConnectSQLServer(wstring strIP, wstring strPort, wstring strServer, wstring strUID, wstring strPWD)
{
	wstring strSRC = L"Provider=OraOLEDB.Oracle.1;Persist Security Info=True;User ID=";
	strSRC += strUID;
	strSRC += L";Password=";
	strSRC += strPWD;
	strSRC += L";Data Source=\"(DESCRIPTION =(ADDRESS_LIST =(ADDRESS = (PROTOCOL = TCP)(HOST = ";
	strSRC += strIP;
	strSRC += L")(PORT = ";
	strSRC += strPort;
	strSRC += L")) )(CONNECT_DATA = (SERVICE_NAME = ";
	strSRC += strServer;
	strSRC += L")))\"";

	//_variant_t varSRC(strSRC.c_str());
	_bstr_t bstrSRC(strSRC.c_str());

	//m_pConnection.CreateInstance("ADODB.Connection");
	try{
		m_pConnection.CreateInstance(__uuidof(Connection));
		//连接超时时间
		m_pConnection->ConnectionTimeout = m_TimeOut;
		m_pConnection->Open(bstrSRC, "", "", adConnectUnspecified);
	}
	catch (_com_error &e)
	{
		PrintComError(e);
		return FALSE;
	}
	return TRUE;
}

//释放RecordsetPtr对象
void CMyDB::ReleaseRecordset(_RecordsetPtr &pRecordset)
{
	if (pRecordset->State)
	{
		pRecordset->Close();
	}
	pRecordset.Release();
}

//获取数据 
//参数ppRecordset为返回值 调用此函数以后，需要调用ReleaseRecordset()函数 释放资源
BOOL CMyDB::GetData(_RecordsetPtr &pRecordset, wstring strSQL)
{
	//_RecordsetPtr pRecordset;		//对记录集提供了更多的控制功能，如记录锁定，游标控制等
	if ( FAILED(pRecordset.CreateInstance(__uuidof(Recordset))) )
	{
		printf("创建_RecordsetPtr实例失败\n");
		return FALSE;
		
	}
	_variant_t varSQL(strSQL.c_str());
	//BSTR bstrSQL = strSQL.c_str();
	if (FAILED(pRecordset->Open(varSQL, (IDispatch*)m_pConnection, adOpenDynamic, adLockOptimistic, adCmdText)))
	{
		ReleaseRecordset(pRecordset);
		printf("打开Recordset失败\n");
		return FALSE;
	}

	//读取数据
	//if ((*ppRecordset)->GetadoBOF())
	//{
	//	//ReleaseRecordset(ppRecordset);
	//	//CloseSQLServer();
	//	return TRUE;
	//}
	//while (!(*ppRecordset)->GetadoEOF())
	//{
	//	_variant_t varOPCCode;
	//	_variant_t varPBDataName;

	//	//varOPCCode = pRecordset->GetCollect("OPCCode");
	//	//varPBDataName = pRecordset->GetCollect("PBDataName");

	//	//pRecordset->MoveNext();
	//}
	return TRUE;
}

long CMyDB::BeginTrans()
{
	
	return m_pConnection->BeginTrans();
}

BOOL CMyDB::CommitTrans()
{
	try{
		m_pConnection->CommitTrans();
	}
	catch (_com_error &e)
	{
		PrintComError(e);
		return FALSE;
	}
	return TRUE;
}

BOOL CMyDB::RollbackTrans()
{
	try{
		m_pConnection->RollbackTrans();
	}
	catch (_com_error &e)
	{
		PrintComError(e);
		return FALSE;
	}
	return TRUE;
}


BOOL CMyDB::ExecuteSQL(wstring strSQL)
{
	_bstr_t bstrSQL(strSQL.c_str());
	_CommandPtr pCMD = NULL;
	try{
		pCMD.CreateInstance(__uuidof(Command));
		pCMD->ActiveConnection = m_pConnection;
		pCMD->CommandText = bstrSQL;
		pCMD->CommandType = adCmdText;

		pCMD->Execute(NULL, NULL, adCmdUnknown);
	}
	catch (_com_error &e)
	{
		pCMD.Release();
		PrintComError(e);
		return FALSE;
	}
	return TRUE;
}

//关闭数据库连接 释放资源
void CMyDB::CloseSQLServer()
{
	if (m_pConnection->State)
	{
		m_pConnection->Close();
	}
	m_pConnection.Release();
}

//打印错误
void CMyDB::PrintComError(_com_error &e)
{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	
	// Print Com errors.
	printf("Error\n");
	printf("\tCode = %08lx\n", e.Error());
	wcout << L"\tCode meaning = " << e.ErrorMessage() << endl;
	printf("\tSource = %s\n", (LPCSTR)bstrSource);
	printf("\tDescription = %s\n", (LPCSTR)bstrDescription);
}

//插入一条实时数据
BOOL CMyDB::InsertRealMeterData(CGPRSBigMeter meter)
{
	wostringstream oss;
	wstring strDevID = ANSIToUnicode(meter.GetDeviceID());
	wstring strSQL = L"delete from MeterData where MeterCode = '";
	strSQL += strDevID;
	strSQL += L"'";

	BOOL bResult = ExecuteSQL(strSQL.c_str());
	if (!bResult)
	{
		return FALSE;
	}
	oss << L"insert into MeterData (MeterCode," << L"CopyDate," << L"UploadTime," << L"MeterStatus," << L"TotalFlow," << L"PressValue," << L"PressStatus," << L"CollecterCode) ";
	oss << L"Values ('" << strDevID << L"','";
	oss << meter.GetBeforeDateString();
	oss << L"',to_date('";
	oss << NowTimeToString(TRUE, L"-", L"-");
	oss << L"','yyyy-mm-dd hh24-mi-ss'),";
	oss << meter.GetMeterStatus();
	oss << L"," << meter.GetTotalFlow();
	oss << L"," << meter.GetPressValue();
	oss << L"," << meter.GetPressStatus();
	oss << L",' ')";
	strSQL = oss.str();
	return ExecuteSQL(strSQL.c_str());
}

//查询历史记录是否已经存在
int CMyDB::QueryRecordExist(wstring strDevID, wstring strDate)
{
	int n = 0;
	wstring strSQL = L"select count(MeterRecordID) as n from MeterRecord where MeterCode = '";
	strSQL += strDevID;
	strSQL += L"' ";
	strSQL += L"and to_date(CopyDate,'yyyy-mm-dd') = to_date('";
	strSQL += strDate;
	strSQL += L"','yyyy-mm-dd')";

	_variant_t varSQL(strSQL.c_str());
	_RecordsetPtr pRecordset;
	try{
		pRecordset.CreateInstance(__uuidof(Recordset));
		pRecordset->Open(varSQL, (IDispatch*)m_pConnection, adOpenDynamic, adLockOptimistic, adCmdText);
		if (!pRecordset->GetadoEOF())
		{
			_variant_t varCount = pRecordset->GetCollect("N");
			n = varCount;
		}
	}
	catch (_com_error &e)
	{
		n = -1;
		//ReleaseRecordset(pRecordset);
		PrintComError(e);
	}
	ReleaseRecordset(pRecordset);
	return n;
}

BOOL CMyDB::InsertMeterRecord(CGPRSBigMeter meter)
{
	cout << "新增加一条大表数据：" << meter.GetDeviceID() << endl;
	wstring strSQL;
	wstring strDevID = ANSIToUnicode(meter.GetDeviceID());
	wostringstream oss;
	oss << L"insert into MeterRecord (MeterRecordID," << L"MeterCode," << L"CopyDate," << L"UploadTime," << L"MeterStatus,";
	oss << L"TotalFlow," << L"MinFlow," << L"RealMinFlow," << L"MinFlowTime," << L"MaxFlow," << L"RealMaxFlow," << L"MaxFlowTime,";
	oss << L"PressValue," << L"PressStatus," << L"PressRecord," << L"RealPressRecord," << L"TempRecord," << L"RealTempRecord," << L"CollecterCode) ";
	oss << L"values (MeterRecord_Seq.Nextval,'";
	oss << strDevID << L"','";
	oss << meter.GetBeforeDateString() << L"',to_date('";
	oss << NowTimeToString(TRUE, L"-", L"-") << L"','yyyy-mm-dd hh24-mi-ss'),";
	oss << meter.GetMeterStatus() << L",";
	oss << meter.GetTotalFlow() << L",";
	oss << meter.GetMinFlow(true) << L",";
	oss << meter.GetMinFlow(false) << L",";
	oss << L"to_date('";
	oss << meter.GetMinFlowTimeString();
	oss << L"','yyyy-mm-dd hh24-mi-ss'),";
	oss << meter.GetMaxFlow(true) << L",";
	oss << meter.GetMaxFlow(false) << L",";
	oss << "to_date('";
	oss << meter.GetMaxFlowTimeString();
	oss << "','yyyy-mm-dd hh24-mi-ss'),";
	oss << meter.GetPressValue() << ",";
	oss << meter.GetPressStatus() << ",";
	oss << "'" << meter.GetAllPressure() << "',";
	oss << "' ',";
	oss << "'" << meter.GetAllTemperature() << "',";
	oss << "' ',";
	oss << "' ')";
	strSQL = oss.str();

	return ExecuteSQL(strSQL.c_str());
}

//更新一条记录数据
BOOL CMyDB::UpdateMeterRecord(CGPRSBigMeter meter)
{
	cout << "更新大表：" << meter.GetDeviceID() << endl;
	wstring strSQL;
	wstring strDevID = ANSIToUnicode(meter.GetDeviceID());
	wostringstream oss;
	oss << "update MeterRecord set ";
	oss << "UploadTime = to_date('" << NowTimeToString(TRUE, L"-", L"-") << "','yyyy-mm-dd hh24-mi-ss'),";
	oss << "MeterStatus = " << meter.GetMeterStatus() << ",";
	oss << "TotalFlow = " << meter.GetTotalFlow() << ",";
	oss << "MinFlow = " << meter.GetMinFlow(true) << ",";
	oss << "RealMinFlow = " << meter.GetMinFlow(false) << ",";
	oss << "MinFlowTime = to_date('";
	oss << meter.GetMinFlowTimeString() << "','yyyy-mm-dd hh24-mi-ss'),";
	oss << "MaxFlow = " << meter.GetMaxFlow(true) << ",";
	oss << "RealMaxFlow = " << meter.GetMaxFlow(false) << ",";
	oss << "MaxFlowTime = to_date('";
	oss << meter.GetMaxFlowTimeString();
	oss << "','yyyy-mm-dd hh24-mi-ss'),";
	oss << "PressValue = " << meter.GetPressValue() << ",";
	oss << "PressStatus = " << meter.GetPressStatus() << ",";
	oss << "PressRecord = '" << meter.GetAllPressure() << "' ,";
	oss << "RealPressRecord = ' ',";
	oss << "TempRecord = '" << meter.GetAllTemperature() << "',";
	oss << "RealTempRecord = ' ',";
	oss << "CollecterCode = ' ' ";
	oss << "where MeterCode = '" << strDevID << "' ";
	oss << "and to_date(CopyDate,'yyyy-mm-dd') = to_date('" << meter.GetBeforeDateString() << "','yyyy-mm-dd')";

	strSQL = oss.str();

	return ExecuteSQL(strSQL.c_str());
}

//更新集中器临时表
BOOL CMyDB::UpdateConSMTempTable(vector<DevConcentrator> vecCon)
{
	BOOL bResult = TRUE;
	vector<DevConcentrator>::iterator iteCon = vecCon.begin();
	wstring strSQL = L"delete roommeterdata_temp";
	if (!ExecuteSQL(strSQL.c_str()))
	{
		return FALSE;
	}
	strSQL = L"";
	
	for (; iteCon != vecCon.end(); iteCon++)
	{
		wostringstream oss;
		oss << L"insert into roommeterdata_temp ";
		//小表
		auto iteSM = iteCon->m_vecSmallMeters.begin();
		for (; iteSM != iteCon->m_vecSmallMeters.end(); iteSM++)
		{
			oss << L"select '";
			oss << iteSM->GetDeviceIDStr();
			oss << L"',";
			oss << iteSM->GetMeterStatus();
			oss << L",";
			oss << iteSM->GetMinSpeed();
			oss << L",";
			oss << iteSM->GetTotalFlow();
			oss << L",'";
			oss << iteCon->GetDateStr();
			oss << L"', to_date('";
			oss << NowTimeToString(TRUE, L"-", L"-");
			oss << L"', 'yyyy-mm-dd hh24-mi-ss'),";
			oss << L"0 from dual";
			if ((iteSM + 1) != iteCon->m_vecSmallMeters.end())
			{
				//不是最后一个
				oss << L" union all ";
			}
			else{
				//oss << L";";
			}
		}
		strSQL = oss.str();
		if (!ExecuteSQL(strSQL.c_str()))
		{
			bResult = FALSE;
			wcout << L"执行SQL错误：" << endl << strSQL.c_str() << endl;
			//出错了，跳出
			break;
		}
	}
	return bResult;
}

//执行存储小表存储过程
BOOL CMyDB::ExecSaveSmallMeter()
{
	_bstr_t bstrSQL(L"saveroommeterrecord");
	_CommandPtr pCMD = NULL;

	try{
		pCMD.CreateInstance(__uuidof(Command));
		pCMD->ActiveConnection = m_pConnection;
		pCMD->CommandText = bstrSQL;
		pCMD->CommandType = adCmdStoredProc;

		//pCMD->Parameters->Refresh();
		pCMD->Execute(NULL, NULL, adCmdStoredProc);

	}
	catch (_com_error &e)
	{
		pCMD.Release();
		PrintComError(e);
		return FALSE;
	}

	return TRUE;
}