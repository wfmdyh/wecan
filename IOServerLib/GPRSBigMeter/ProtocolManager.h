/*
协议管理类

*/
#pragma once


class CProtocolManager
{
private:
	const string NAME_BIG;								//大表协议名称
	const string NAME_CONCENTRATOR;						//集中器名称
	CRITICAL_SECTION m_csVecBigMeters;					//大表数据互斥
	CRITICAL_SECTION m_csVecConcentrator;
	HANDLE m_hDatabase;									//数据库线程句柄
	vector<CGPRSBigMeter> m_vecBigMeters;				//大表数据
	vector<DevConcentrator> m_vecConcentrator;			//集中器数据
	int m_nBigMeterPort;								//大表监听端口
	int m_nConcentratorPort;							//集中器监听端口
public:
	BOOL m_FlagShutdown;
	CDataBaseManager m_DBMgr;							//数据库管理
	CIOCPModel* m_pTCPModuleBigMeter;					//大表TCP模型
	TCPModelConcentrator* m_pTCPModuleCon;				//集中器TCP模型
	SendCommand m_CmdOrder;								//下发指令队列
public:
	CProtocolManager();
	~CProtocolManager();
	//更新或者插入一条大表数据
	void UpdateBigMeter(CGPRSBigMeter meter);
	//取回大表的数据
	vector<CGPRSBigMeter> GetBigMeters();
	//集中器
	void UpdateConcentrator(DevConcentrator con);
	vector<DevConcentrator> GetConcentrators();
	//设置大表监听端口
	bool SetBigMeterPort(int nPort);
	//获取大表监听端口
	int GetBigMeterPort();
	//设置集中器监听端口
	bool SetConcentratorPort(int nPort);
	//获取集中器监听端口
	int GetConcentratorPort();

	void Init(fnDataArrive fnDA, vector<CHANNEL_INFO*> *pVecColData);
	void UnInit();
	//启动大表
	bool StartBigMeter(vector<CHANNEL_INFO*> *pVecColData);
	//启动集中器
	bool StartConcentrator(vector<CHANNEL_INFO*> *pVecColData);
	//加载winsock库
	BOOL LoadSocketLib();
	void UnloadSocketLib() { WSACleanup(); }
};

