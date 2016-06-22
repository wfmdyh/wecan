/*
Э�������

*/
#pragma once


class CProtocolManager
{
private:
	const string NAME_BIG;								//���Э������
	const string NAME_CONCENTRATOR;						//����������
	CRITICAL_SECTION m_csVecBigMeters;					//������ݻ���
	CRITICAL_SECTION m_csVecConcentrator;
	HANDLE m_hDatabase;									//���ݿ��߳̾��
	vector<CGPRSBigMeter> m_vecBigMeters;				//�������
	vector<DevConcentrator> m_vecConcentrator;			//����������
	int m_nBigMeterPort;								//�������˿�
	int m_nConcentratorPort;							//�����������˿�
public:
	BOOL m_FlagShutdown;
	CDataBaseManager m_DBMgr;							//���ݿ����
	CIOCPModel* m_pTCPModuleBigMeter;					//���TCPģ��
	TCPModelConcentrator* m_pTCPModuleCon;				//������TCPģ��
	SendCommand m_CmdOrder;								//�·�ָ�����
public:
	CProtocolManager();
	~CProtocolManager();
	//���»��߲���һ���������
	void UpdateBigMeter(CGPRSBigMeter meter);
	//ȡ�ش�������
	vector<CGPRSBigMeter> GetBigMeters();
	//������
	void UpdateConcentrator(DevConcentrator con);
	vector<DevConcentrator> GetConcentrators();
	//���ô������˿�
	bool SetBigMeterPort(int nPort);
	//��ȡ�������˿�
	int GetBigMeterPort();
	//���ü����������˿�
	bool SetConcentratorPort(int nPort);
	//��ȡ�����������˿�
	int GetConcentratorPort();

	void Init(fnDataArrive fnDA, vector<CHANNEL_INFO*> *pVecColData);
	void UnInit();
	//�������
	bool StartBigMeter(vector<CHANNEL_INFO*> *pVecColData);
	//����������
	bool StartConcentrator(vector<CHANNEL_INFO*> *pVecColData);
	//����winsock��
	BOOL LoadSocketLib();
	void UnloadSocketLib() { WSACleanup(); }
};

