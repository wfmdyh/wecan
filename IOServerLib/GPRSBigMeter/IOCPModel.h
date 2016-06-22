#pragma once
//#include "ProtocolManager.h"

// 缓冲区长度 (1024*8)
// 之所以为什么设置8K，也是一个江湖上的经验值
// 如果确实客户端发来的每组数据都比较少，那么就设置得小一些，省内存
#define MAX_BUFFER_LEN        8192  

class CProtocolManager;
// CIOCPModel类
class CIOCPModel
{
private:
	string m_strIP;						// 服务器端的IP地址
	int m_nPort;						// 服务器端的监听端口
	MyLog m_log;						//日志
	HANDLE m_hAccept;
	//vector<CGPRSBigMeter> m_vecMeters;	//设备数量
public:
	fnDataArrive m_DataArrive;			//数据到达时调用的回调函数
	SOCKET m_ListenSocket;
	HANDLE m_hShutdownEvent;			// 用来通知线程系统退出的事件，为了能够更好的退出线程
	
	CRITICAL_SECTION m_csDataFile;		//将数据包写入到文件的互斥
	//CRITICAL_SECTION m_csChannel;		//连接将设备存储到通道的互斥

	
	CProtocolManager *m_pProtocolMgr;	//协议管理类
public:
	CIOCPModel(void);
	~CIOCPModel(void);

	//void AddMeterToVec(CGPRSBigMeter meter);
	//vector<CGPRSBigMeter> GetMeters();

	// 启动服务器
	bool Start(CProtocolManager* pPM);
	//	停止服务器
	void Stop();

	void SetPort( const int& nPort ) { m_nPort=nPort; }
	int GetPort(){ return m_nPort; }
	//将数据写入文件
	void WriteDataToFile(char *pData, DWORD dwLen);
	//客户端线程
	static void ThreadClient(LPVOID lpThreadParameter);
	//接受请求线程
	static unsigned int CALLBACK ThreadAccept(void *lpParam);
	// 判断客户端Socket是否已经断开
	bool _IsSocketAlive(SOCKET s);
	//由连接对象调用，pDevice储存的是解析完的设备对象
	//void GetOneDevice(CGPRSBigMeter *pDevice);
};

//判断ID是否相等
class MeterCompare
{
private:
	string m_strID;
public:
	MeterCompare(string strID)
	{
		m_strID = strID;
	}
	bool operator()(vector<CGPRSBigMeter>::value_type value)
	{
		return (value.GetDeviceID().compare(m_strID) == 0);
	}
};

//class ConcentratorCompare
//{
//private:
//	DWORD m_ID;
//public:
//	ConcentratorCompare(DWORD dwID)
//	{
//		m_ID = dwID;
//	}
//	bool operator()(vector<DevConcentrator>::value_type value)
//	{
//		return (value.m_DeviceID == m_ID);
//	}
//};