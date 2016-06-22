#pragma once
#include <msclr\marshal.h>
using namespace DatabaseServer;
using namespace WeCanIOT;
using namespace System;
using namespace msclr::interop;
using namespace System::Runtime::InteropServices;

class DevPipeNet;

ref class DatabaseMgr
{
private:
	Session^ m_DBSession;
public:
	DatabaseMgr();
	bool SaveData(list<DevPipeNet*> arrDev);
	//保存报警信息
	void SaveAlarm(AlarmData^ alarm);
	//保存设备数据
	void SaveDevice(CollectData^ dev);

	//打开数据库连接
	bool OpenDB();
	//关闭数据库连接
	bool CloseDB();
};

