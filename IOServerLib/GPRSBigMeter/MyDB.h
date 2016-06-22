#pragma once

typedef map<wstring, _variant_t> SQL_DATA_MAP;
typedef vector<wstring> COLLECTS;

class DevConcentrator;

class CMyDB
{
private:
	_ConnectionPtr m_pConnection;		//����һ����¼����һ����ָ��,ͨ��ʹ����������һ���������ӻ�ִ��һ���������κν����SQL���
	DWORD m_TimeOut;
	//_RecordsetPtr m_pRecordset;
public:
	CMyDB();
	~CMyDB();
	//�������ݿ�
	BOOL ConnectSQLServer(wstring strIP, wstring strPort, wstring strServer, wstring strUID, wstring strPWD);
	//�ͷ�RecordsetPtr����
	void ReleaseRecordset(_RecordsetPtr &pRecordset);
	//�ر����ݿ�
	void CloseSQLServer();
	//��ӡ����
	void PrintComError(_com_error &e);

	//��ȡ����
	BOOL ExecuteSQL(wstring strSQL);
	BOOL GetData(_RecordsetPtr &pRecordset, wstring strSQL);
	//�������ʷ��¼�Ƿ���� strDate yyyy-mm-dd
	int QueryRecordExist(wstring strDevID, wstring strDate);
	//����һ��������
	BOOL InsertMeterRecord(CGPRSBigMeter meter);
	//����һ����¼����
	BOOL UpdateMeterRecord(CGPRSBigMeter meter);
	//����һ��ʵʱ����
	BOOL InsertRealMeterData(CGPRSBigMeter meter);

	//������
	//���¼�������ʱ��
	BOOL UpdateConSMTempTable(vector<DevConcentrator> vecCon);
	//ִ�д洢С��洢����
	BOOL ExecSaveSmallMeter();
	//����
	long BeginTrans();
	BOOL CommitTrans();
	BOOL RollbackTrans();
};

