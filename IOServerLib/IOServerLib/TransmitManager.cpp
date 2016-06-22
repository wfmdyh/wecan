#include "stdafx.h"
#include "TransmitManager.h"
#include "OPCServer.h"
#include "IOServerLib.h"

TransmitManager::TransmitManager()
{
	m_hOPC = NULL;
}


TransmitManager::~TransmitManager()
{
	//�ͷ���Դ
	for (unsigned i = 0; i < m_vecChannel.size(); i++)
	{
		delete m_vecChannel.at(i);
		m_vecChannel.at(i) = NULL;
	}
	m_vecChannel.clear();
}

void TransmitManager::AddTransmitChannel(TransmitChannel *pChannel)
{
	m_vecChannel.push_back(pChannel);
}

vector<TransmitChannel*> TransmitManager::GetVecChannel()
{
	return m_vecChannel;
}

TransmitManager* TransmitManager::GetProtocolChannel(string strProtocolName)
{
	TransmitManager * traMgr = new TransmitManager;
	for (unsigned i = 0; i < m_vecChannel.size(); i++)
	{
		TransmitChannel *sourceChannel = m_vecChannel.at(i);
		
		string strDll = sourceChannel->m_strDllFile;
		if (string::npos != strDll.rfind(strProtocolName))
		{
			TransmitChannel *destChannel = new TransmitChannel;
			//��ָ��Э���ͨ����ӽ���
			*destChannel = *sourceChannel;
			traMgr->AddTransmitChannel(destChannel);
		}
	}
	return traMgr;
}

//��ӡ����
void TransmitManager::ShowData()
{
	for (unsigned c = 0; c < m_vecChannel.size(); c++)
	{
		TransmitChannel *pChannel = m_vecChannel.at(c);
		vector<TransmitPoint*> vecPoint = pChannel->GetVecPoint();
		OutputDebugString(L"ͨ�����ƣ�");
		OutputDebugStringA(pChannel->m_strName.c_str());
		OutputDebugString(L"\n");
		for (unsigned p = 0; p < vecPoint.size(); p++)
		{
			TransmitPoint* pPoint = vecPoint.at(p);
			OutputDebugString(L"�����ƣ�");
			OutputDebugStringA(pPoint->m_strName.c_str());
			OutputDebugString(L"\t��ֵ��");
			OutputDebugStringA(pPoint->GetValue().c_str());
			OutputDebugString(L"\n");
		}
	}
	
}

void TransmitManager::RelevanceColIndex(vector<CHANNEL_INFO*> vecChannel)
{
	vector<TransmitChannel*>::iterator iteTraChannel = m_vecChannel.begin();
	for (; iteTraChannel != m_vecChannel.end(); iteTraChannel++)
	{
		TransmitChannel *pTraChannel = *iteTraChannel;
		vector<TransmitPoint*> vecTraPoint = pTraChannel->GetVecPoint();
		vector<TransmitPoint*>::iterator iteTraPoint = vecTraPoint.begin();
		for (; iteTraPoint != vecTraPoint.end(); iteTraPoint++)
		{
			int nC = -1;
			int nD = -1;
			int nP = -1;
			TransmitPoint *pTraPoint = (*iteTraPoint);
			//����
			map<string,string> mapTraParam = pTraPoint->GetParams();
			//���±�
			for (unsigned int c = 0; c < vecChannel.size(); c++)
			{
				CHANNEL_INFO *pChannel = vecChannel.at(c);
				if (pChannel->astrName.compare(mapTraParam["ColChannel"]) == 0)
				{
					nC = c;
					//���豸
					for (unsigned int d = 0; d < pChannel->vecDevice.size(); d++)
					{
						DEVICE_INFO *pDev = pChannel->vecDevice.at(d);
						if (pDev->astrDeviceName.compare(mapTraParam["ColDevice"]) == 0)
						{
							nD = d;
							//�ҵ�
							for (unsigned int p = 0; p < pDev->vecPoint.size(); p++)
							{
								COL_POINT *cp = pDev->vecPoint.at(p);
								if (cp->astrName.compare(mapTraParam["ColPoint"]) == 0)
								{
									nP = p;
									//�����ɼ���
									pTraPoint->SetColIndex(nC, nD, nP);
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
			/////////////////////////////////////////////
		}
	}
}

BOOL g_bRunning = FALSE;
//COPCServer *g_OPCServer = NULL;
//�̺߳���
unsigned __stdcall ThreadChannel(void *pParam)
{
	TransmitManager* pOPCMgr = (TransmitManager*)pParam;
	//OPC Serverֻ������һ��
	COPCServer *OPCServer = new COPCServer;
	OPCServer->SetTransmit(pOPCMgr);
	if (!OPCServer->Start())
	{
		cout << "OPC Server����ʧ��\n";
		return -1;
	}
	//ȡ��ת��Ƶ��  �����˳��¼�
	while (1)
	{
		if (!g_bRunning)
		{
			//�˳�
			break;
		}
		//����ͨ�������ݴ����ص�����
		if (pOPCMgr != NULL)
		{
			//�ص�������IOServer����ã����Ҹ�ֵ
			UpdataTransmit(pOPCMgr);
		}
		//����ת����ȥ
		OPCServer->TransmitData();
		//����
		//g_ProtocolMgr->ShowData();
		Sleep(1000);
	}
	//�ͷ���Դ
	if (pOPCMgr != NULL)
	{
		delete pOPCMgr;
	}
	OPCServer->Stop();
	delete OPCServer;
	OPCServer = NULL;
	return 0;
}

bool TransmitManager::Start()
{
	g_bRunning = TRUE;
	//ȡ��opc serverЭ���ͨ��
	TransmitManager* pOPCMgr = GetProtocolChannel("OPCServer.dll");
	m_hOPC = (HANDLE)_beginthreadex(NULL, 0, ThreadChannel, pOPCMgr, 0, NULL);
	if (m_hOPC == NULL)
	{
		m_log.LogMsgToFile(L"����ת���߳�ʧ��");
		return false;
	}
	
	return true;
}

bool TransmitManager::Stop()
{
	g_bRunning = FALSE;
	WaitForSingleObject(m_hOPC, INFINITE);
	CloseHandle(m_hOPC);

	return true;
}