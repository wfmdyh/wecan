#include "stdafx.h"
#include "IOServerManager.h"
#include "DataManager.h"
#include "MyTools.h"
#include "IOServerLib.h"
#include "TransmitPoint.h"
#include "DataDef.h"

//�����ӿڶ���
//�ɼ�ͨ��
//�ص�����
typedef void(*fnDataArrive)(CHANNEL_INFO *pChannel);
typedef BOOL(*PDeviceOpen)(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData);
//�µ������ӿ�
typedef void(*fnShowMessage)(const wchar_t* wcsMsg, ...);
typedef BOOL (*fnIOS_DeviceOpenEx)(fnShowMessage lpfnShowMessage, DataManager* pMgr);

typedef void(*PDeviceClose)();
typedef BOOL(*PDeviceWrite)(wstring strChannel, wstring strDevice, wstring strPoint, wstring strValue);
//ת��ͨ��
//�ص����� ������pTraManager����ֵ��ϵ�ת������
//typedef bool(*fnUpdataTransmit)(TransmitManager *pTraManager);
//typedef BOOL (*PTransmitStart)(TransmitManager *pTraManager, fnUpdataTransmit UpdataTransmit);
//typedef BOOL(*PTransmitStop)();


IOServerManager::IOServerManager(string strProjectPath)
{
	m_hDBLib = NULL;
	m_strProjectPath = strProjectPath;
	m_Channels = new vector<CHANNEL_INFO*>();
	//m_hThreadSave = INVALID_HANDLE_VALUE;

	//��ʼ��
	InitializeCriticalSection(&m_csChannels);
	//ת��ͨ��
	m_TransmitMgr = new TransmitManager;

	m_DataMgr = new DataManager;

}


IOServerManager::~IOServerManager()
{
	//�ͷ�ͨ����Դ
	if (m_Channels != NULL)
	{
		for (unsigned i = 0; i < m_Channels->size(); i++)
		{
			CHANNEL_INFO *pChannel = m_Channels->at(i);
			delete pChannel;
			pChannel = NULL;
		}
		m_Channels->clear();
		delete m_Channels;
		m_Channels = NULL;
	}
	//�ͷ�������Դ
	/*if (m_DriverList != NULL)
	{
		for (unsigned i = 0; i < m_DriverList->size(); i++)
		{
			CIOSDriver *pDriver = m_DriverList->at(i);
			delete pDriver;
			pDriver = NULL;
		}
		m_DriverList->clear();
		delete m_DriverList;
		m_DriverList = NULL;
	}*/

	DeleteCriticalSection(&m_csChannels);
	//�ͷ�ת��ͨ��
	if (m_TransmitMgr != NULL)
	{
		delete m_TransmitMgr;
		m_TransmitMgr = NULL;
	}

	delete m_DataMgr;
}

//���һ���������б���
bool IOServerManager::AddDriver(CIOSDriver driver)
{

	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		if (driver.GetName() == m_DriverList.at(i).GetName())
		{
			//�Ѿ���ӹ��ˣ�����Ҫ�������
			return false;
		}
	}

	m_DriverList.push_back(driver);
	return true;
}

bool IOServerManager::ReadOneTransmit(TransmitChannel *pTraChannel)
{
	
	TiXmlDocument myDocument(pTraChannel->m_strConfigFileFullPath.c_str());
	if (!myDocument.LoadFile())
	{
		ShowMessage(L"�޷���ȡת�������ļ�");
		return false;
	}

	TiXmlElement *elRoot = myDocument.RootElement();
	if (!elRoot)
	{
		return false;
	}
	TiXmlElement *elChannel = elRoot->FirstChildElement("Channel");
	if (elChannel)
	{
		TiXmlElement *elParam = elChannel->FirstChildElement("Param");
		//ͨ������
		while (elParam != NULL)
		{
			const char *tmpKey = NULL, *tmpValue = NULL;
			//POINT_PARAM paramChannel;
			tmpKey = elParam->Attribute("Name");
			tmpValue = elParam->Attribute("Value");
			if (tmpKey != NULL && tmpValue != NULL)
			{
				pTraChannel->AddChannelParam(tmpKey, tmpValue);
			}
			elParam = elParam->NextSiblingElement("Param");
		}
		//��
		TiXmlElement *elPoint = elChannel->FirstChildElement("Point");
		while (elPoint)
		{
			TransmitPoint *pTraPoint = new TransmitPoint;
			pTraChannel->AddPoint(pTraPoint);
			pTraPoint->m_strName = elPoint->Attribute("Name");
			//����
			TiXmlElement *elMap = elPoint->FirstChildElement();
			while (elMap != NULL && strcmp(elMap->Value(), "Param") == 0)
			{
				const char *tmpKey = NULL, *tmpValue = NULL;
				tmpKey = elMap->Attribute("Name");
				tmpValue = elMap->Attribute("Value");
				
				if (tmpKey != NULL && tmpValue != NULL)
				{
					pTraPoint->AddPointParam(tmpKey, tmpValue);
				}
				elMap = elMap->NextSiblingElement();
			}
			
			elPoint = elPoint->NextSiblingElement("Point");
		}
	}
	return true;
}

bool IOServerManager::ReadOneChannel(CHANNEL_INFO* pChannel)
{
	TiXmlDocument myDocument(pChannel->astrConfigFileFullPath.c_str());
	if (!myDocument.LoadFile())
	{
		ShowMessage(L"�޷���ȡ�����ļ�");
		return false;
	}

	TiXmlElement *elRoot = myDocument.RootElement();
	if (!elRoot)
	{
		return false;
	}
	TiXmlElement *elChannel = elRoot->FirstChildElement("Channel");
	if (elChannel)
	{
		TiXmlElement *elParam = elChannel->FirstChildElement("Param");
		while (elParam != NULL)
		{
			//ͨ������
			const char *tmpKey = NULL, *tmpValue = NULL;
			//POINT_PARAM paramChannel;
			tmpKey = elParam->Attribute("Name");
			tmpValue = elParam->Attribute("Value");
			if (tmpKey != NULL && tmpValue != NULL)
			{
				pChannel->mapParam[tmpKey] = tmpValue;
			}
			elParam = elParam->NextSiblingElement("Param");
		}

		TiXmlElement *elDevice = elChannel->FirstChildElement("Device");
		//�豸
		while (elDevice)
		{
			DEVICE_INFO *pDevice = new DEVICE_INFO();
			pDevice->astrDeviceName = elDevice->Attribute("Name");
			//����
			elParam = elDevice->FirstChildElement("Param");
			while (elParam != NULL)
			{
				//�豸����
				const char *tmpKey = NULL, *tmpValue = NULL;
				tmpKey = elParam->Attribute("Name");
				tmpValue = elParam->Attribute("Value");
				if (tmpKey != NULL && tmpValue != NULL)
				{
					pDevice->mapParam[tmpKey] = tmpValue;
				}
				elParam = elParam->NextSiblingElement("Param");
			}
			//��
			TiXmlElement *elPoint = elDevice->FirstChildElement("Point");
			while (elPoint)
			{
				COL_POINT *pPont = new COL_POINT();
				pPont->astrName = elPoint->Attribute("Name");
				//pPont->varData = NULL;
				//����
				TiXmlElement *elMap = elPoint->FirstChildElement("Map");
				while (elMap != NULL)
				{
					map<string,string> piParam;
					const char *tmpKey = NULL, *tmpValue = NULL;
					tmpKey = elMap->Attribute("Code");
					tmpValue = elMap->Attribute("Value");
					if (tmpKey != NULL && tmpValue != NULL)
					{
						pPont->mapParam[tmpKey] = tmpValue;
					}
					elMap = elMap->NextSiblingElement("Map");
				}

				//ֵ�Ƿ����仯����ʼֵΪfalse
				pPont->IsChange = false;

				pDevice->vecPoint.push_back(pPont);
				elPoint = elPoint->NextSiblingElement("Point");
			}
			pChannel->vecDevice.push_back(pDevice);
			elDevice = elDevice->NextSiblingElement();
		}
	}
	return true;
}


bool IOServerManager::LoadProjectFile()
{
	string astrIOConFileFullPath = m_strProjectPath + "ioserver.xml";
	TiXmlDocument myDocument(astrIOConFileFullPath.c_str());
	if (!myDocument.LoadFile())
	{
		return false;
	}
	TiXmlElement *elRoot = myDocument.RootElement();
	if (elRoot == NULL)
	{
		return false;
	}

	//�ɼ�
	TiXmlElement *elColChannel = elRoot->FirstChildElement("ColChannel");
	if (elColChannel == NULL)
	{
		return false;
	}
	TiXmlElement *elChannel = elColChannel->FirstChildElement("Channel");
	while (elChannel != NULL)
	{
		CIOSDriver driver;// = new CIOSDriver();
		string strDriverPath = elChannel->Attribute("DllFile");
		driver.SetName(strDriverPath);
		driver.SetType(CIOSDriver::IOSDTYPE::TYPE_COL);
		AddDriver(driver);

		CHANNEL_INFO *pChannel = new CHANNEL_INFO();
		pChannel->astrDllFile = strDriverPath;
		//�����ļ�ȫ·��
		string strChannelConfig = elChannel->Attribute("ConfigFile");
		strChannelConfig = m_strProjectPath + strChannelConfig;
		//�����ļ�·��
		pChannel->astrConfigFileFullPath = strChannelConfig;
		//����
		pChannel->astrName = elChannel->Attribute("Name");
		//��ȡ����ͨ��������
		ReadOneChannel(pChannel);
		m_Channels->push_back(pChannel);
		m_DataMgr->AddChannel(*pChannel);
		elChannel = elChannel->NextSiblingElement("Channel");
	}
	//ת��
	TiXmlElement *elTraChannel = elRoot->FirstChildElement("TraChannel");
	//������ת��ͨ��
	if (elTraChannel != NULL)
	{
		elChannel = NULL;
		elChannel = elTraChannel->FirstChildElement("Channel");
		while (elChannel)
		{
			CIOSDriver driver;// = new CIOSDriver;
			string strDriverPath = elChannel->Attribute("DllFile");
			driver.SetName(strDriverPath);
			driver.SetType(CIOSDriver::IOSDTYPE::TYPE_TRA);
			AddDriver(driver);

			TransmitChannel *pTraChannel = new TransmitChannel;
			
			pTraChannel->m_strDllFile = strDriverPath;

			////�����ļ�ȫ·��
			string strChannelConfig = elChannel->Attribute("ConfigFile");
			strChannelConfig = m_strProjectPath + strChannelConfig;
			
			pTraChannel->m_strConfigFileFullPath = strChannelConfig;
			pTraChannel->m_strName = elChannel->Attribute("Name");
			////��ȡ����ͨ��������
			ReadOneTransmit(pTraChannel);
			
			m_TransmitMgr->AddTransmitChannel(pTraChannel);
			elChannel = elChannel->NextSiblingElement();
		}
		//�����ɼ�ͨ��
		m_TransmitMgr->RelevanceColIndex(*m_Channels);
	}
	return true;
}

bool IOServerManager::InvokeDllFun()
{
	//���ض�̬��
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver driver = m_DriverList.at(i);
		if (driver.GetType() == CIOSDriver::IOSDTYPE::TYPE_TRA)
		{
			//ת��ͨ��������
			continue;
		}
		wstring strPath = ANSIToUnicode(driver.GetName());
		HMODULE hDriver = LoadLibrary(strPath.c_str());
		if (hDriver == NULL)
		{
			ShowMessage(L"���������ʧ�ܣ�%s", strPath.c_str());
			//����
		}
		else{
			m_DriverList.at(i).SetHandle(hDriver);
		}
	}
	//ִ�к���
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver iosDriver = m_DriverList.at(i);
		HMODULE hDll = iosDriver.GetHandle();
		//�ж��Ƿ�Ϊ�ɼ�ͨ��
		if (hDll != NULL && iosDriver.GetType() == CIOSDriver::IOSDTYPE::TYPE_COL)
		{
			//��ȡ������ַ
			//PDeviceOpen DeviceOpen = (PDeviceOpen)GetProcAddress(hDll, "IOS_DeviceOpen");
			fnIOS_DeviceOpenEx DeviceOpen = (fnIOS_DeviceOpenEx)GetProcAddress(hDll, "IOS_DeviceOpenEx");
			if (!DeviceOpen)
			{
				FreeLibrary(hDll);
				m_DriverList.at(i).SetHandle(NULL);
				ShowMessage(L"���ö�̬��%s����ʧ��", ANSIToUnicode(iosDriver.GetName()).c_str());
				continue;
			}
			//���ú���
			//if (!(*DeviceOpen)(DataArrive, m_Channels))
			if (!(*DeviceOpen)(ShowMessage, m_DataMgr))
			{
				FreeLibrary(hDll);
				m_DriverList.at(i).SetHandle(NULL);
				ShowMessage(L"Э�麯������ʧ��");
				return false;
			}
		}
	}
	return true;
}

//���ö�̬����ͷ���Դ����
bool IOServerManager::InvokeDllCloseFun()
{
	//��ж��ת��ͨ��
	
	//�ɼ�ͨ��
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver iosDriver = m_DriverList.at(i);
		HMODULE hDll = iosDriver.GetHandle();
		if (hDll != NULL && iosDriver.GetType() == CIOSDriver::IOSDTYPE::TYPE_COL)
		{
			//��ȡ������ַ
			PDeviceClose DevClose = (PDeviceClose)GetProcAddress(hDll, "IOS_DeviceClose");
			if (DevClose != NULL)
			{
				(*DevClose)();
			}
			//ж�ض�̬��
			FreeLibrary(hDll);
			m_DriverList.at(i).SetHandle(NULL);
		}
	}
	return true;
}

//���ݿ����
//bool IOServerManager::LoadDataBase()
//{
//	//�������ݿ⺯��
//	m_hDBLib = LoadLibrary(L"DataSourceLib.dll");
//	if (m_hDBLib == NULL)
//	{
//		m_log.LogMsgToFile(L"�������ݿ����ӿ�ʧ��");
//		return false;
//	}
//	m_SaveDataToDB = (pSaveDataToDB)GetProcAddress(m_hDBLib, "SaveDataToDB");
//	if (!m_SaveDataToDB)
//	{
//		m_log.LogMsgToFile(L"�������ݿ�ӿں���ʧ��");
//		return false;
//	}
//	return true;
//}

//bool IOServerManager::SaveDataToDB()
//{
//	bool bRet = false;
//	if (m_SaveDataToDB != NULL)
//	{
//		EnterCriticalSection(&m_csChannels);
//		bRet = (*m_SaveDataToDB)(m_Channels);
//		LeaveCriticalSection(&m_csChannels);
//	}
//	else{
//		m_log.LogMsgToFile(L"�洢ʧ�ܣ�δ�������ݿ⶯̬��");
//	}
//	return bRet;
//}
//
//void IOServerManager::UnloadDataBase()
//{
//	if (m_hDBLib != NULL)
//	{
//		FreeLibrary(m_hDBLib);
//		m_hDBLib = NULL;
//	}
//}
//
////�洢�߳�
//void IOServerManager::ThreadSaveData(LPVOID lpParam)
//{
//	IOServerManager *iosMgr = (IOServerManager *)lpParam;
//	while (WAIT_OBJECT_0 != WaitForSingleObject(iosMgr->m_hShutdownEvent, 0))
//	{
//		iosMgr->SaveDataToDB();
//		Sleep(5000);
//	}
//	OutputDebugString(L"���ݿ�洢�߳��˳�\n");
//	_endthread();
//}
//
////��ʼ�洢����
//bool IOServerManager::BeginSaveData()
//{
//	m_hThreadSave = (HANDLE)_beginthread(ThreadSaveData, 0, this);
//	if (m_hThreadSave == INVALID_HANDLE_VALUE)
//	{
//		m_log.LogMsgToFile(L"�����洢�߳�ʧ��");
//		return false;
//	}
//	return true;
//}


//��ʼ����
bool IOServerManager::Start()
{
	//����ϵͳ�˳��¼�
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!LoadProjectFile())
	{
		return false;
	}

	if (!InvokeDllFun())
	{
		return false;
	}

	//���ݿ�
	//if (!LoadDataBase())
	//{
	//	UnloadDataBase();
	//	return false;
	//}
	////��ʼ�洢
	//if (!BeginSaveData())
	//{
	//	return false;
	//}

	//����ת��ͨ��
	/*if (!m_TransmitMgr->Start())
	{
		return false;
	}*/

	return true;
}

bool IOServerManager::Stop()
{
	//ֹͣת��ͨ��
	//m_TransmitMgr->Stop();
	//ֹͣ
	SetEvent(m_hShutdownEvent);
	//�洢�߳�
	//WaitForSingleObject(m_hThreadSave, INFINITE);
	////ж�����ݿ�
	//UnloadDataBase();
	//ֹͣ��̬��
	InvokeDllCloseFun();

	return true;
}

