/*
WFM
2016-3-14
���ݿ������
*/
#pragma once

//��̬�⵼������ԭ��
//typedef BOOL(*fnSaveDataToDB)(vector<CGPRSBigMeter>* listGprsBigMeter);

class CDataBaseManager
{
private:
	vector<CGPRSBigMeter> m_listMeters;				//���е�ˮ��
	vector<DevConcentrator> m_listConcentrators;	//������
	HMODULE m_hDBLib;
	CMyDB m_dbOracle;
	//���ݿ�����
	wstring m_IP;
	wstring m_PORT;
	wstring m_DBNAME;
	wstring m_DBUSER;
	wstring m_DBPWD;
public:
	BOOL m_FlagShutdown;							//ֹͣ��־λ
public:
	CDataBaseManager();
	~CDataBaseManager();

	//���������ļ�
	BOOL LoadConfig();
	//��ʼ
	BOOL Start();
	//ֹͣ
	BOOL Stop();
	BOOL UpdateOne(CGPRSBigMeter meter);
	//������Ҫ�洢������
	BOOL UpdateMeters(CProtocolManager* pProtocolMgr);
	//�洢�������
	BOOL UpdateBigMeterData(vector<CGPRSBigMeter> vecBigMeters);
	//�洢����������
	BOOL UpdateConcentratorData(vector<DevConcentrator> vecCon);

	//���¼�������ʷ��¼
	vector<DevConcentrator>::iterator ConHistoryExist(DevConcentrator concentrator);
};

