#pragma once
#include "DataDef.h"
#include "channel.h"
#include "MyLog.h"

// winsock 2 的头文件和库
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")

// 缓冲区长度 (1024*8)
// 之所以为什么设置8K，也是一个江湖上的经验值
// 如果确实客户端发来的每组数据都比较少，那么就设置得小一些，省内存
#define MAX_BUFFER_LEN        8192  


// CIOCPModel类
class CIOCPModel
{
private:
	string m_strIP;						// 服务器端的IP地址
	int m_nPort;						// 服务器端的监听端口
	MyLog m_log;						//日志
	CHANNEL_INFO* m_pChannel;			//通道副本
public:
	fnDataArrive m_DataArrive;			//数据到达时调用的回调函数
	SOCKET m_ListenSocket;
	HANDLE m_hShutdownEvent;			// 用来通知线程系统退出的事件，为了能够更好的退出线程
	
	CRITICAL_SECTION m_csDataFile;		//将数据包写入到文件的互斥
	CRITICAL_SECTION m_csChannel;		//连接将设备存储到通道的互斥

	HANDLE m_hThrdAccept;
public:
	CIOCPModel(void);
	~CIOCPModel(void);
	// 启动服务器
	BOOL Start(CHANNEL_INFO* pChannel);
	//	停止服务器
	void Stop();
	// 加载Socket库
	bool LoadSocketLib();
	void UnloadSocketLib() { WSACleanup(); }

	void SetPort( const int& nPort ) { m_nPort=nPort; }
	int GetPort(){ return m_nPort; }

	//将数据写入文件
	void WriteDataToFile(char *pData, DWORD dwLen);

	//等待接收请求
	static void ThreadAccept(LPVOID lpParam);
	//客户端线程
	static void ThreadClient(LPVOID lpThreadParameter);

	// 判断客户端Socket是否已经断开
	bool _IsSocketAlive(SOCKET s);

	//如果设定时间内没有连接，将温度初始为-50
	void Init50(time_t NowTime);
	//将时间转为字符串
	int API_TimeToString(string &strDateStr,time_t &timeData);

	//由连接对象调用，pDevice储存的是解析完的设备对象
	void GetOneDevice(CGPRSTemperature *pDevice);
};

