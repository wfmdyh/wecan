#include "stdafx.h"
#include "IOServerManager.h"
#include "DataManager.h"
#include "MyTools.h"
#include "IOServerLib.h"
#include "TransmitPoint.h"
#include "DataDef.h"

//公共接口定义
//采集通道
//回调函数
typedef void(*fnDataArrive)(CHANNEL_INFO *pChannel);
typedef BOOL(*PDeviceOpen)(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData);
//新的驱动接口
typedef void(*fnShowMessage)(const wchar_t* wcsMsg, ...);
typedef BOOL (*fnIOS_DeviceOpenEx)(fnShowMessage lpfnShowMessage, DataManager* pMgr);

typedef void(*PDeviceClose)();
typedef BOOL(*PDeviceWrite)(wstring strChannel, wstring strDevice, wstring strPoint, wstring strValue);
//转发通道
//回调函数 参数：pTraManager，赋值完毕的转发数据
//typedef bool(*fnUpdataTransmit)(TransmitManager *pTraManager);
//typedef BOOL (*PTransmitStart)(TransmitManager *pTraManager, fnUpdataTransmit UpdataTransmit);
//typedef BOOL(*PTransmitStop)();


IOServerManager::IOServerManager(string strProjectPath)
{
	m_hDBLib = NULL;
	m_strProjectPath = strProjectPath;
	m_Channels = new vector<CHANNEL_INFO*>();
	//m_hThreadSave = INVALID_HANDLE_VALUE;

	//初始化
	InitializeCriticalSection(&m_csChannels);
	//转发通道
	m_TransmitMgr = new TransmitManager;

	m_DataMgr = new DataManager;

}


IOServerManager::~IOServerManager()
{
	//释放通道资源
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
	//释放驱动资源
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
	//释放转发通道
	if (m_TransmitMgr != NULL)
	{
		delete m_TransmitMgr;
		m_TransmitMgr = NULL;
	}

	delete m_DataMgr;
}

//添加一个驱动到列表中
bool IOServerManager::AddDriver(CIOSDriver driver)
{

	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		if (driver.GetName() == m_DriverList.at(i).GetName())
		{
			//已经添加过了，不需要重新添加
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
		ShowMessage(L"无法读取转发配置文件");
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
		//通道参数
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
		//点
		TiXmlElement *elPoint = elChannel->FirstChildElement("Point");
		while (elPoint)
		{
			TransmitPoint *pTraPoint = new TransmitPoint;
			pTraChannel->AddPoint(pTraPoint);
			pTraPoint->m_strName = elPoint->Attribute("Name");
			//参数
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
		ShowMessage(L"无法读取配置文件");
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
			//通道参数
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
		//设备
		while (elDevice)
		{
			DEVICE_INFO *pDevice = new DEVICE_INFO();
			pDevice->astrDeviceName = elDevice->Attribute("Name");
			//参数
			elParam = elDevice->FirstChildElement("Param");
			while (elParam != NULL)
			{
				//设备参数
				const char *tmpKey = NULL, *tmpValue = NULL;
				tmpKey = elParam->Attribute("Name");
				tmpValue = elParam->Attribute("Value");
				if (tmpKey != NULL && tmpValue != NULL)
				{
					pDevice->mapParam[tmpKey] = tmpValue;
				}
				elParam = elParam->NextSiblingElement("Param");
			}
			//点
			TiXmlElement *elPoint = elDevice->FirstChildElement("Point");
			while (elPoint)
			{
				COL_POINT *pPont = new COL_POINT();
				pPont->astrName = elPoint->Attribute("Name");
				//pPont->varData = NULL;
				//参数
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

				//值是否发生变化，初始值为false
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

	//采集
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
		//配置文件全路径
		string strChannelConfig = elChannel->Attribute("ConfigFile");
		strChannelConfig = m_strProjectPath + strChannelConfig;
		//配置文件路径
		pChannel->astrConfigFileFullPath = strChannelConfig;
		//名称
		pChannel->astrName = elChannel->Attribute("Name");
		//读取单个通道的配置
		ReadOneChannel(pChannel);
		m_Channels->push_back(pChannel);
		m_DataMgr->AddChannel(*pChannel);
		elChannel = elChannel->NextSiblingElement("Channel");
	}
	//转发
	TiXmlElement *elTraChannel = elRoot->FirstChildElement("TraChannel");
	//配置了转发通道
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

			////配置文件全路径
			string strChannelConfig = elChannel->Attribute("ConfigFile");
			strChannelConfig = m_strProjectPath + strChannelConfig;
			
			pTraChannel->m_strConfigFileFullPath = strChannelConfig;
			pTraChannel->m_strName = elChannel->Attribute("Name");
			////读取单个通道的配置
			ReadOneTransmit(pTraChannel);
			
			m_TransmitMgr->AddTransmitChannel(pTraChannel);
			elChannel = elChannel->NextSiblingElement();
		}
		//关联采集通道
		m_TransmitMgr->RelevanceColIndex(*m_Channels);
	}
	return true;
}

bool IOServerManager::InvokeDllFun()
{
	//加载动态库
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver driver = m_DriverList.at(i);
		if (driver.GetType() == CIOSDriver::IOSDTYPE::TYPE_TRA)
		{
			//转发通道不加载
			continue;
		}
		wstring strPath = ANSIToUnicode(driver.GetName());
		HMODULE hDriver = LoadLibrary(strPath.c_str());
		if (hDriver == NULL)
		{
			ShowMessage(L"驱动库加载失败：%s", strPath.c_str());
			//继续
		}
		else{
			m_DriverList.at(i).SetHandle(hDriver);
		}
	}
	//执行函数
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver iosDriver = m_DriverList.at(i);
		HMODULE hDll = iosDriver.GetHandle();
		//判断是否为采集通道
		if (hDll != NULL && iosDriver.GetType() == CIOSDriver::IOSDTYPE::TYPE_COL)
		{
			//获取函数地址
			//PDeviceOpen DeviceOpen = (PDeviceOpen)GetProcAddress(hDll, "IOS_DeviceOpen");
			fnIOS_DeviceOpenEx DeviceOpen = (fnIOS_DeviceOpenEx)GetProcAddress(hDll, "IOS_DeviceOpenEx");
			if (!DeviceOpen)
			{
				FreeLibrary(hDll);
				m_DriverList.at(i).SetHandle(NULL);
				ShowMessage(L"调用动态库%s函数失败", ANSIToUnicode(iosDriver.GetName()).c_str());
				continue;
			}
			//调用函数
			//if (!(*DeviceOpen)(DataArrive, m_Channels))
			if (!(*DeviceOpen)(ShowMessage, m_DataMgr))
			{
				FreeLibrary(hDll);
				m_DriverList.at(i).SetHandle(NULL);
				ShowMessage(L"协议函数调用失败");
				return false;
			}
		}
	}
	return true;
}

//调用动态库的释放资源函数
bool IOServerManager::InvokeDllCloseFun()
{
	//先卸载转发通道
	
	//采集通道
	for (unsigned i = 0; i < m_DriverList.size(); i++)
	{
		CIOSDriver iosDriver = m_DriverList.at(i);
		HMODULE hDll = iosDriver.GetHandle();
		if (hDll != NULL && iosDriver.GetType() == CIOSDriver::IOSDTYPE::TYPE_COL)
		{
			//获取函数地址
			PDeviceClose DevClose = (PDeviceClose)GetProcAddress(hDll, "IOS_DeviceClose");
			if (DevClose != NULL)
			{
				(*DevClose)();
			}
			//卸载动态库
			FreeLibrary(hDll);
			m_DriverList.at(i).SetHandle(NULL);
		}
	}
	return true;
}

//数据库操作
//bool IOServerManager::LoadDataBase()
//{
//	//加载数据库函数
//	m_hDBLib = LoadLibrary(L"DataSourceLib.dll");
//	if (m_hDBLib == NULL)
//	{
//		m_log.LogMsgToFile(L"加载数据库链接库失败");
//		return false;
//	}
//	m_SaveDataToDB = (pSaveDataToDB)GetProcAddress(m_hDBLib, "SaveDataToDB");
//	if (!m_SaveDataToDB)
//	{
//		m_log.LogMsgToFile(L"调用数据库接口函数失败");
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
//		m_log.LogMsgToFile(L"存储失败，未加载数据库动态库");
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
////存储线程
//void IOServerManager::ThreadSaveData(LPVOID lpParam)
//{
//	IOServerManager *iosMgr = (IOServerManager *)lpParam;
//	while (WAIT_OBJECT_0 != WaitForSingleObject(iosMgr->m_hShutdownEvent, 0))
//	{
//		iosMgr->SaveDataToDB();
//		Sleep(5000);
//	}
//	OutputDebugString(L"数据库存储线程退出\n");
//	_endthread();
//}
//
////开始存储数据
//bool IOServerManager::BeginSaveData()
//{
//	m_hThreadSave = (HANDLE)_beginthread(ThreadSaveData, 0, this);
//	if (m_hThreadSave == INVALID_HANDLE_VALUE)
//	{
//		m_log.LogMsgToFile(L"创建存储线程失败");
//		return false;
//	}
//	return true;
//}


//开始运行
bool IOServerManager::Start()
{
	//建立系统退出事件
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!LoadProjectFile())
	{
		return false;
	}

	if (!InvokeDllFun())
	{
		return false;
	}

	//数据库
	//if (!LoadDataBase())
	//{
	//	UnloadDataBase();
	//	return false;
	//}
	////开始存储
	//if (!BeginSaveData())
	//{
	//	return false;
	//}

	//启动转发通道
	/*if (!m_TransmitMgr->Start())
	{
		return false;
	}*/

	return true;
}

bool IOServerManager::Stop()
{
	//停止转发通道
	//m_TransmitMgr->Stop();
	//停止
	SetEvent(m_hShutdownEvent);
	//存储线程
	//WaitForSingleObject(m_hThreadSave, INFINITE);
	////卸载数据库
	//UnloadDataBase();
	//停止动态库
	InvokeDllCloseFun();

	return true;
}

