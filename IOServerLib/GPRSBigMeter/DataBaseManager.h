/*
WFM
2016-3-14
数据库管理类
*/
#pragma once

//动态库导出函数原型
//typedef BOOL(*fnSaveDataToDB)(vector<CGPRSBigMeter>* listGprsBigMeter);

class CDataBaseManager
{
private:
	vector<CGPRSBigMeter> m_listMeters;				//所有的水表
	vector<DevConcentrator> m_listConcentrators;	//集中器
	HMODULE m_hDBLib;
	CMyDB m_dbOracle;
	//数据库配置
	wstring m_IP;
	wstring m_PORT;
	wstring m_DBNAME;
	wstring m_DBUSER;
	wstring m_DBPWD;
public:
	BOOL m_FlagShutdown;							//停止标志位
public:
	CDataBaseManager();
	~CDataBaseManager();

	//加载配置文件
	BOOL LoadConfig();
	//开始
	BOOL Start();
	//停止
	BOOL Stop();
	BOOL UpdateOne(CGPRSBigMeter meter);
	//更新想要存储的数据
	BOOL UpdateMeters(CProtocolManager* pProtocolMgr);
	//存储大表数据
	BOOL UpdateBigMeterData(vector<CGPRSBigMeter> vecBigMeters);
	//存储集中器数据
	BOOL UpdateConcentratorData(vector<DevConcentrator> vecCon);

	//更新集中器历史记录
	vector<DevConcentrator>::iterator ConHistoryExist(DevConcentrator concentrator);
};

