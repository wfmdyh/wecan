#pragma once
class DevPipeNet;

class DeviceConnect
{
private:
	char* m_pRecDataBuf;			//���յ������ݻ������
	int m_iDataRear;				//���ݵĽ���λ��
	const int DATA_BUFFER_SIZE;
	
public:
	//TCPModel *m_Parent;
	DevPipeNet *m_Device;			//������ϵ�����
	DWORD m_DeviceID;				//��ǰ�������ݵ��豸ID
public:
	DeviceConnect();
	~DeviceConnect();
	
	//���ͻ����յ������ݿ��������ݶ����У�����ֵ��1֡����������2֡���������������գ�3�����֡
	int CopyDataToBuf(char *pData, DWORD dwDataLen);
	void TryAnalysis();
	wstring GetDeviceIDStr();
private:
	//��������
	void RemoveFrontData(int iNum);
	void AnalyzeData(BYTE *szBuf, WORD wPackageLen);

	//2�ֽ�ת��
	WORD ConvertTwoBytes(BYTE *pData);
	//4�ֽ�ת��
	DWORD ConvertFourBytes(BYTE *pData);
	//8�ֽ�תdouble
	double ConvertEightByteToDouble(BYTE *pby);

	//��֤֡�Ƿ���ȷ
	int CheckPackage();
};

