/*
�豸���Ӷ���
*/
#pragma once
#include "DataDef.h"
#include "IOCPModel.h"

class DeviceConnect
{
private:
	char* m_pRecDataBuf;			//���յ������ݻ������
	int m_iDataRear;				//���ݵĽ���λ��
	CGPRSTemperature *m_Device;		//������ϵ�����
public:
	//fnGetOneDevice m_GetOneDevice;			//���ݲɼ����ʱ���õĻص�����
	CIOCPModel *m_Parent;					//ͨ��
public:
	DeviceConnect();
	~DeviceConnect();
	/*
	���ͻ����յ������ݿ��������ݶ�����
	*/
	void CopyDataToBuf(char *pData, DWORD dwDataLen);
private:
	//���Խ���
	void TryAnalysis();
	//��������
	void RemoveFrontData(int iNum);
	BOOL AnalyzeData(BYTE *szBuf);

	//������־�ļ�
	void WriteErrToFile(char *pData, DWORD dwLen);

	string TempID;

};

