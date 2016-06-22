/*
����������
*/
#pragma once
#include "channel.h"
#include "IOSDriver.h"
#include "MyLog.h"
#include "TransmitManager.h"

//���ݿ�ӿ�
//typedef bool(*pSaveDataToDB)(vector<CHANNEL_INFO*> *vecChannels);
class DataManager;
class IOServerManager
{
private:
	string m_strProjectPath;
	
	vector<CIOSDriver> m_DriverList;			//�����ļ�����Ҫ���ص������б�
	//MyLog m_log;								//��־

	HMODULE m_hDBLib;							//���ݿ�
	//pSaveDataToDB m_SaveDataToDB;				//���ݿ�ӿں���
	//HANDLE m_hThreadSave;						//�������ݵ��߳̾��
	TransmitManager *m_TransmitMgr;				//����ת��ͨ��
public:
	CRITICAL_SECTION m_csChannels;				//���ڻ���ͨ������
	HANDLE m_hShutdownEvent;					//�˳��¼�
	vector<CHANNEL_INFO*> *m_Channels;			//ͨ��
	DataManager* m_DataMgr;						//���ݲ�������
public:
	IOServerManager(string strProjectPath);
	~IOServerManager();

	bool Start();
	bool Stop();
	
	//��ʼ�洢����
	//bool BeginSaveData();
	//�洢�߳�
	//static void ThreadSaveData(LPVOID lpParam);
	
private:
	//������Ŀ�����ļ�
	bool LoadProjectFile();
	//���ض�̬�Ⲣִ�ж�̬���еĺ���
	bool InvokeDllFun();
	//���ö�̬����ͷ���Դ����
	bool InvokeDllCloseFun();

	//�������ݿ⺯��
	//bool LoadDataBase();
	//���浽���ݿ�
	//bool SaveDataToDB();
	//ж�����ݿ�
	//void UnloadDataBase();

	//���һ���������б���
	bool AddDriver(CIOSDriver driver);
	//��ȡ����ͨ����������Ϣ
	bool ReadOneChannel(CHANNEL_INFO* pChannel);
	//��ȡ����ת��ͨ��
	bool ReadOneTransmit(TransmitChannel *pTraChannel);
};

