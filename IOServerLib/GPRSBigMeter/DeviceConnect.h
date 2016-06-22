/*
�豸���Ӷ���
*/
#pragma once

class CIOCPModel;
class TCPModelConcentrator;

class DeviceConnect
{
private:
	char* m_pRecDataBuf;			//���յ������ݻ������
	int m_iDataRear;				//���ݵĽ���λ��
	CGPRSBigMeter *m_Device;		//������ϵ�����
public:
	CIOCPModel *m_Parent;						//���ͨ��
	DWORD m_DeviceID;							//��ǰ�������ݵ��豸ID
public:
	DeviceConnect();
	~DeviceConnect();
	/*
	���ͻ����յ������ݿ��������ݶ����У�����ֵ��1֡����������2֡���������������գ�3�����֡
	*/
	int CopyDataToBuf(char *pData, DWORD dwDataLen);
	//���Խ���
	void TryAnalysis();
private:
	//��������
	void RemoveFrontData(int iNum);
	BOOL AnalyzeData(BYTE *szBuf, WORD wPackageLen);

	//2�ֽ�ת��
	WORD ConvertTwoBytes(BYTE *pData);
	//4�ֽ�ת��
	DWORD ConvertFourBytes(BYTE *pData);
	//8�ֽ�תdouble
	double ConvertEightByteToDouble(BYTE *pby);

	//��֤֡�Ƿ���ȷ
	int CheckPackage();
};

