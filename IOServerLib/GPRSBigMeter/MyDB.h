#pragma once

typedef map<wstring, _variant_t> SQL_DATA_MAP;
typedef vector<wstring> COLLECTS;

class DevConcentrator;

class CMyDB
{
private:
	_ConnectionPtr m_pConnection;		//返回一个记录集或一个空指针,通常使用它来创建一个数据连接或执行一条不返回任何结果的SQL语句
	DWORD m_TimeOut;
	//_RecordsetPtr m_pRecordset;
public:
	CMyDB();
	~CMyDB();
	//连接数据库
	BOOL ConnectSQLServer(wstring strIP, wstring strPort, wstring strServer, wstring strUID, wstring strPWD);
	//释放RecordsetPtr对象
	void ReleaseRecordset(_RecordsetPtr &pRecordset);
	//关闭数据库
	void CloseSQLServer();
	//打印错误
	void PrintComError(_com_error &e);

	//获取数据
	BOOL ExecuteSQL(wstring strSQL);
	BOOL GetData(_RecordsetPtr &pRecordset, wstring strSQL);
	//当天的历史记录是否存在 strDate yyyy-mm-dd
	int QueryRecordExist(wstring strDevID, wstring strDate);
	//插入一条表数据
	BOOL InsertMeterRecord(CGPRSBigMeter meter);
	//更新一条记录数据
	BOOL UpdateMeterRecord(CGPRSBigMeter meter);
	//插入一条实时数据
	BOOL InsertRealMeterData(CGPRSBigMeter meter);

	//集中器
	//更新集中器临时表
	BOOL UpdateConSMTempTable(vector<DevConcentrator> vecCon);
	//执行存储小表存储过程
	BOOL ExecSaveSmallMeter();
	//事务
	long BeginTrans();
	BOOL CommitTrans();
	BOOL RollbackTrans();
};

