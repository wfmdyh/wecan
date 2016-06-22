/*
管理数据类
*/
#pragma once
#include "channel.h"
#include "IOSDriver.h"
#include "MyLog.h"
#include "TransmitManager.h"

//数据库接口
//typedef bool(*pSaveDataToDB)(vector<CHANNEL_INFO*> *vecChannels);
class DataManager;
class IOServerManager
{
private:
	string m_strProjectPath;
	
	vector<CIOSDriver> m_DriverList;			//配置文件中需要加载的驱动列表
	//MyLog m_log;								//日志

	HMODULE m_hDBLib;							//数据库
	//pSaveDataToDB m_SaveDataToDB;				//数据库接口函数
	//HANDLE m_hThreadSave;						//保存数据的线程句柄
	TransmitManager *m_TransmitMgr;				//管理转发通道
public:
	CRITICAL_SECTION m_csChannels;				//用于互斥通道数据
	HANDLE m_hShutdownEvent;					//退出事件
	vector<CHANNEL_INFO*> *m_Channels;			//通道
	DataManager* m_DataMgr;						//数据操作对象
public:
	IOServerManager(string strProjectPath);
	~IOServerManager();

	bool Start();
	bool Stop();
	
	//开始存储数据
	//bool BeginSaveData();
	//存储线程
	//static void ThreadSaveData(LPVOID lpParam);
	
private:
	//加载项目配置文件
	bool LoadProjectFile();
	//加载动态库并执行动态库中的函数
	bool InvokeDllFun();
	//调用动态库的释放资源函数
	bool InvokeDllCloseFun();

	//加载数据库函数
	//bool LoadDataBase();
	//保存到数据库
	//bool SaveDataToDB();
	//卸载数据库
	//void UnloadDataBase();

	//添加一个驱动到列表中
	bool AddDriver(CIOSDriver driver);
	//读取单个通道的配置信息
	bool ReadOneChannel(CHANNEL_INFO* pChannel);
	//读取单个转发通道
	bool ReadOneTransmit(TransmitChannel *pTraChannel);
};

