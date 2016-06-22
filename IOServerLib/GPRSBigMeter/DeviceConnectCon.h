/*
���������Ӷ���
*/
#pragma once
class TCPModelConcentrator;

class DeviceConnectCon
{
private:
	char* m_pRecDataBuf;			//���յ������ݻ������
	int m_iDataRear;				//���ݵĽ���λ��
	DevConcentrator *m_Device;		//������ϵ�����
	map<int, BYTE*> m_FrameSet;		//֡����
public:
	TCPModelConcentrator *m_Parent;						//������
	DWORD m_DeviceID;									//��ǰ�������ݵ��豸ID
public:
	DeviceConnectCon();
	~DeviceConnectCon();
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

