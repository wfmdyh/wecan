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

//���ݿ��߳�
DWORD WINAPI ProtocolMgr::ThreadDatabase(_In_ LPVOID lpParameter)
{
	ProtocolMgr* pMgr = (ProtocolMgr*)lpParameter;
	//���ݿ�����
	DatabaseMgr^ dbMgr = gcnew DatabaseMgr();
	if (!dbMgr->OpenDB())
	{
		ShowMessage(L"�����ݿ�ʧ��");
		return 1;
	}
	ShowMessage(L"�����ݿ�����");
	while (WAIT_OBJECT_0 != WaitForSingleObject(pMgr->m_hShutdownEvent, 0))
	{
		list<DevPipeNet*> tmpArrDev = pMgr->GetAllDevice();
		//��ȡ�������豸�Ժ����
		pMgr->DeleteAllDevice();
		if (!dbMgr->SaveData(tmpArrDev))
		{
			ShowMessage(L"��������ʧ��");
			
		}
		/*else{
			ShowMessage(L"��������");
		}*/
		
		Sleep(5000);
	}
	if (dbMgr->CloseDB())
	{
		ShowMessage(L"�ر����ݿ�����");
	}
	return 0;
}

BOOL ProtocolMgr::Start(DataManager* pMgr)
{
	m_nBigMeterPort = pMgr->GetChannelPort(PROTOCOL_NAME);
	wstringstream wss;
	wss << L"�����˿ںţ�" << m_nBigMeterPort;
	ShowMessage(wss.str().c_str());
	if (m_nBigMeterPort < 1)
	{
		ShowMessage(L"����ʧ�ܣ��˿ں����ô���");
		return FALSE;
	}
	// ����ϵͳ�˳����¼�֪ͨ
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_TcpModel->m_nPort = m_nBigMeterPort;
	if (!m_TcpModel->Start(this))
	{
		return FALSE;
	}
	m_hDatabase = CreateThread(NULL, 0, ThreadDatabase, this, 0, NULL);
	if (m_hDatabase == NULL)
	{
		ShowMessage(L"���ݿ�����ʧ��");
		return FALSE;
	}
	return TRUE;
}

void ProtocolMgr::Stop()
{
	// ����ر���Ϣ֪ͨ
	SetEvent(m_hShutdownEvent);
	ShowMessage(L"�ȴ����ݿ�ֹͣ...");
	//ֹͣ���ݿ�
	WaitForSingleObject(m_hDatabase, INFINITE);
	CloseHandle(m_hDatabase);
	ShowMessage(L"���ݿ�ֹͣ���");
	m_TcpModel->Stop();
}

void ProtocolMgr::UpdateDevice(DevPipeNet* dev)
{
	EnterCriticalSection(&m_csDevice);
	bool bExist = false;
	DevPipeNet* pTempDev = NULL;

	//�ж��豸�Ƿ��Ѿ���ӹ�
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
		//�ж��Ƿ�Ϊ��Ч����
		if ((dev->m_TotalFlow != -1) || (dev->m_ArrMultipleParam.size() > 0))
		{
			//�ۼ�����Ϊ-1ʱ����Ϊ�ǳ�ʼֵ��Ҳ�����豸û�д�ֵ�����˵�
			//�ж��±������Ƿ���ֵ�����û�� ���������������Чֵ
			pTempDev = new DevPipeNet;
			*pTempDev = *dev;
			m_ArrDevice.push_back(pTempDev);
		}
		else{
			wstringstream wss;
			wss << L"�豸��" << dev->m_DeviceID << L"����Чֵ";
			ShowMessage(wss.str().c_str());
		}
	}
	LeaveCriticalSection(&m_csDevice);
}

//ɾ�������豸���ͷ���Դ
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
