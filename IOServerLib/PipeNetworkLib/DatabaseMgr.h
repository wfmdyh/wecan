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
	//���汨����Ϣ
	void SaveAlarm(AlarmData^ alarm);
	//�����豸����
	void SaveDevice(CollectData^ dev);

	//�����ݿ�����
	bool OpenDB();
	//�ر����ݿ�����
	bool CloseDB();
};

