#include "stdafx.h"
#include "ProtocolMgr.h"
#include "channel.h"
#include "TCPModel.h"
#include "DataManager.h"
#include "DataDef.h"
#include "DevPipeNet.h"
#include "DatabaseMgr.h"


ProtocolMgr::ProtocolMgr():
	PROTOCOL_NAME("PipeNetworkLib")
{
	m_nBigMeterPort = 0;
	m_TcpModel = new TCPModel;
	InitializeCriticalSection(&m_csDevice);
	m_hShutdownEvent = NULL;
}


ProtocolMgr::~ProtocolMgr()
{
	DeleteAllDevice();

	DeleteCriticalSection(&m_csDevice);

	delete m_TcpModel;
	m_TcpModel = NULL;
}

//数据库线程
DWORD WINAPI ProtocolMgr::ThreadDatabase(_In_ LPVOID lpParameter)
{
	ProtocolMgr* pMgr = (ProtocolMgr*)lpParameter;
	//数据库启动
	DatabaseMgr^ dbMgr = gcnew DatabaseMgr();
	if (!dbMgr->OpenDB())
	{
		ShowMessage(L"打开数据库失败");
		return 1;
	}
	ShowMessage(L"打开数据库连接");
	while (WAIT_OBJECT_0 != WaitForSingleObject(pMgr->m_hShutdownEvent, 0))
	{
		list<DevPipeNet*> tmpArrDev = pMgr->GetAllDevice();
		//获取完所有设备以后，清空
		pMgr->DeleteAllDevice();
		if (!dbMgr->SaveData(tmpArrDev))
		{
			ShowMessage(L"保存数据失败");
			
		}
		/*else{
			ShowMessage(L"保存数据");
		}*/
		
		Sleep(5000);
	}
	if (dbMgr->CloseDB())
	{
		ShowMessage(L"关闭数据库连接");
	}
	return 0;
}

BOOL ProtocolMgr::Start(DataManager* pMgr)
{
	m_nBigMeterPort = pMgr->GetChannelPort(PROTOCOL_NAME);
	wstringstream wss;
	wss << L"监听端口号：" << m_nBigMeterPort;
	ShowMessage(wss.str().c_str());
	if (m_nBigMeterPort < 1)
	{
		ShowMessage(L"启动失败，端口号设置错误");
		return FALSE;
	}
	// 建立系统退出的事件通知
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_TcpModel->m_nPort = m_nBigMeterPort;
	if (!m_TcpModel->Start(this))
	{
		return FALSE;
	}
	m_hDatabase = CreateThread(NULL, 0, ThreadDatabase, this, 0, NULL);
	if (m_hDatabase == NULL)
	{
		ShowMessage(L"数据库启动失败");
		return FALSE;
	}
	return TRUE;
}

void ProtocolMgr::Stop()
{
	// 激活关闭消息通知
	SetEvent(m_hShutdownEvent);
	ShowMessage(L"等待数据库停止...");
	//停止数据库
	WaitForSingleObject(m_hDatabase, INFINITE);
	CloseHandle(m_hDatabase);
	ShowMessage(L"数据库停止完毕");
	m_TcpModel->Stop();
}

void ProtocolMgr::UpdateDevice(DevPipeNet* dev)
{
	EnterCriticalSection(&m_csDevice);
	bool bExist = false;
	DevPipeNet* pTempDev = NULL;

	//判断设备是否已经添加过
	auto ite = m_ArrDevice.begin();
	for (; ite != m_ArrDevice.end(); ite++)
	{
		pTempDev = *ite;
		if (pTempDev->m_DeviceID == dev->m_DeviceID)
		{
			*pTempDev = *dev;
			bExist = true;
			break;
		}
	}
	if (!bExist)
	{
		//判断是否为有效数据
		if ((dev->m_TotalFlow != -1) || (dev->m_ArrMultipleParam.size() > 0))
		{
			//累计流量为-1时，认为是初始值，也就是设备没有传值，过滤掉
			//判断新表数据是否有值，如果没有 则代表传上来的是无效值
			pTempDev = new DevPipeNet;
			*pTempDev = *dev;
			m_ArrDevice.push_back(pTempDev);
		}
		else{
			wstringstream wss;
			wss << L"设备：" << dev->m_DeviceID << L"，无效值";
			ShowMessage(wss.str().c_str());
		}
	}
	LeaveCriticalSection(&m_csDevice);
}

//删除所有设备，释放资源
void ProtocolMgr::DeleteAllDevice()
{
	EnterCriticalSection(&m_csDevice);
	list<DevPipeNet*>::iterator iteDev = m_ArrDevice.begin();
	for (; iteDev != m_ArrDevice.end(); iteDev++)
	{
		delete *iteDev;
	}
	m_ArrDevice.clear();
	LeaveCriticalSection(&m_csDevice);
}

list<DevPipeNet*> ProtocolMgr::GetAllDevice()
{
	list<DevPipeNet*> tmpArrDev;
	EnterCriticalSection(&m_csDevice);

	list<DevPipeNet*>::iterator iteDev = m_ArrDevice.begin();
	for (; iteDev != m_ArrDevice.end(); iteDev++)
	{
		DevPipeNet* pCopyDev = new DevPipeNet;
		*pCopyDev = *(*iteDev);
		tmpArrDev.push_back(pCopyDev);
	}

	LeaveCriticalSection(&m_csDevice);
	return tmpArrDev;
}
