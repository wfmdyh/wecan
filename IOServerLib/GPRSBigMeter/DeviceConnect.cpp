#include "stdafx.h"
#include "DeviceConnect.h"
#include "ProtocolManager.h"

//static const int DATA_BUF_SIZE = 1024;

DeviceConnect::DeviceConnect()
{
	m_DeviceID = 0;
	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1, DATA_BUF_SIZE);

	//Ϊ�豸��������ڴ�ռ�
	m_Device = new(nothrow) CGPRSBigMeter();
	m_Parent = NULL;
}


DeviceConnect::~DeviceConnect()
{
	if (m_pRecDataBuf != NULL)
	{
		free(m_pRecDataBuf);
		m_pRecDataBuf = NULL;
		m_iDataRear = 0;
	}
	if (m_Device != NULL)
	{
		delete m_Device;
		m_Device = NULL;
	}
}

/*
���ͻ����յ������ݿ��������ݶ�����
*/
int DeviceConnect::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;
	//ÿ�յ�һ�����ݰ����ͳ��Խ���һ��
	
	return CheckPackage();
}

//��֤֡�Ƿ���ȷ
int DeviceConnect::CheckPackage()
{
	int iIndex = 0;
	//��������
	WORD wPackageLen = 0;
	char *dataTmp = m_pRecDataBuf;

	//����ͷ
	if ((BYTE)(*dataTmp) != 0x68)
	{
		cout << "�����֡��������0x68��ͷ������ȫ��...\n";
		//�����֡������ȫ��
		RemoveFrontData(m_iDataRear);
		return 3;
	}
	++iIndex;
	wPackageLen = ConvertTwoBytes((BYTE*)(dataTmp + iIndex));
	//iIndex += 2;
	//�Ƿ������һ֡
	if (wPackageLen > m_iDataRear)
	{
		cout << "����������֡����������...\n";
		return 2;
	}
	//����β
	BYTE byEnd = *(dataTmp + (wPackageLen - 1));
	if (byEnd != 0x16)
	{
		cout << "�����֡��������0x16����������ȫ��...\n";
		//�����֡������ȫ��
		RemoveFrontData(m_iDataRear);
		return 3;
	}
	//У���
	unsigned int uiSum = 0;
	BYTE byCS = *(dataTmp + (wPackageLen - 2));
	//����У���
	for (int i = 0; i < wPackageLen - 2; i++)
	{
		uiSum += *(dataTmp + i);
	}
	uiSum %= 256;
	if (byCS != uiSum)
	{
		cout << "�����֡��У��ʹ���\n";
		return 3;
	}
	return 1;
}

//���Խ���
void DeviceConnect::TryAnalysis()
{
	OutputDebugString(L"�������ݽ���\n");
	WORD sPackageLen = ConvertTwoBytes((BYTE*)(m_pRecDataBuf + 1));
	//�������ݣ�������Ƚ���
	//��������
	BYTE *oneData = (BYTE*)calloc(1, sPackageLen);
	memcpy(oneData, m_pRecDataBuf, sPackageLen);
	RemoveFrontData(sPackageLen);

	AnalyzeData(oneData, sPackageLen);
	free(oneData);
	oneData = NULL;
}

//��������
void DeviceConnect::RemoveFrontData(int iNum)
{
	//�������Ϣ������
	int iRemainLen = m_iDataRear - iNum;
	if (iRemainLen < 0)
	{
		//������󳤶Ⱦ�ȫ������
		iRemainLen = 0;
		m_iDataRear = 0;
		memset(m_pRecDataBuf, 0, DATA_BUF_SIZE);
	}
	else{
		memcpy(m_pRecDataBuf, m_pRecDataBuf + iNum, iRemainLen);
		memset(m_pRecDataBuf + iRemainLen, 0, DATA_BUF_SIZE - iRemainLen);
		m_iDataRear -= iNum;
	}
}

//������ݱ�����ȷ������FALSE
BOOL DeviceConnect::AnalyzeData(BYTE *szBuf, WORD wPackageLen)
{	
	DWORD dwIndex = 7;		//��λ��ID
	//ID
	m_DeviceID = ConvertFourBytes(szBuf + dwIndex);
	dwIndex += 4;
	string strID = str_format("%u", m_DeviceID);
	m_Device->SetDeviceID(strID);

	//ѹ�����ݳ���
	WORD wPressureLen = 0;
	//�ж�TAGֵ
	BYTE tag = *(szBuf + dwIndex);
	while (dwIndex < (wPackageLen-2))
	{
		//ָ������λ
		dwIndex++;
		switch (tag)
		{
		case 0x00:
			printf("�豸��%s���ϵ����\n", strID.c_str());
			++dwIndex;
			break;
		case 0x01:
			m_Device->SetMeterStatus(*(szBuf + dwIndex));
			++dwIndex;
			break;
		case 0x02:
			m_Device->SetTotalFlow((float)ConvertEightByteToDouble(szBuf + dwIndex));
			dwIndex += 8;
			break;
		case 0x03:
			m_Device->SetPressValue((BYTE)(*(szBuf + dwIndex)));
			++dwIndex;
			break;
		case 0x04:
		{
			//ѹ���澯
			m_Device->SetPressValue(*(szBuf + dwIndex));
			++dwIndex;
			m_Device->SetPressStatus(*(szBuf + dwIndex));
			++dwIndex;
			break;
		}
		case 0x05:
		{
			SYSTEMTIME tmMax = { 0 };
			float fValue = 0.0f;
			tmMax.wHour = *(szBuf + dwIndex);
			++dwIndex;
			tmMax.wMinute = *(szBuf + dwIndex);
			m_Device->SetMaxFlowTime(tmMax);
			++dwIndex;
			memcpy(&fValue, szBuf + dwIndex, 4);
			m_Device->SetMaxFlow(fValue);
			dwIndex += 4;
			break;
		}
		case 0x06:
		{
			float fValue = 0.0f;
			SYSTEMTIME tmMin = { 0 };
			tmMin.wHour = *(szBuf + dwIndex);
			++dwIndex;
			tmMin.wMinute = *(szBuf + dwIndex);
			m_Device->SetMinFlowTime(tmMin);
			++dwIndex;
			memcpy(&fValue, szBuf + dwIndex, 4);
			m_Device->SetMinFlow(fValue);
			dwIndex += 4;
			break;
		}
		case 0x07:
		{
			wstringstream ss(L"");
			wstring strTemp;
			wPressureLen = ConvertTwoBytes(szBuf + dwIndex);
			dwIndex += 2;		//��λ�����ݿ�

			ss << (int)(*(szBuf + dwIndex));
			for (int i = 0; i < wPressureLen-1; i++)
			{
				dwIndex++;
				ss << ",";
				ss << (int)(*(szBuf + dwIndex));
			}
			ss >> strTemp;
			m_Device->SetAllPressure(strTemp);
			++dwIndex;
			break;
		}
		case 0x08:
		{
			SYSTEMTIME st = {0};
			wstringstream wss;
			wstring strDate;
			st.wYear = (int)(*(szBuf + dwIndex)) + 2000;
			++dwIndex;
			st.wMonth = (int)(*(szBuf + dwIndex));
			++dwIndex;
			st.wDay = (int)(*(szBuf + dwIndex));
			m_Device->SetBeforeDate(st);
			++dwIndex;
			break;
		}
		case 0x0A:
			m_Device->SetSuccessRate(*(szBuf + dwIndex));
			++dwIndex;
			break;
		case 0x0b:
		{
			wstringstream wss;
			wstring strValue;
			wss << (int)(*(szBuf + dwIndex));
			for (int i = 0; i < 96-1; i++)
			{
				dwIndex++;
				wss << ",";
				wss << (int)(*(szBuf + dwIndex));
			}
			wss >> strValue;
			m_Device->SetAllTemperature(strValue);
			++dwIndex;
			break;
		}
		}

		tag = *(szBuf + dwIndex);
	}
	//ʱ���
	m_Device->SetLastTime(time(NULL));
	m_Parent->m_pProtocolMgr->UpdateBigMeter(*m_Device);
	//���ûص�����
	//m_Parent->GetOneDevice(m_Device);
	
	return TRUE;
}

//2�ֽ�ת��
WORD DeviceConnect::ConvertTwoBytes(BYTE *pData)
{
	int iIndex = 0;
	WORD wResult = 0;
	BYTE *byDest = (BYTE*)&wResult;
	
	byDest[1] = *(pData + iIndex);
	++iIndex;
	byDest[0] = *(pData + iIndex);

	return wResult;
}

//4�ֽ�ת��
DWORD DeviceConnect::ConvertFourBytes(BYTE *pData)
{
	DWORD dwResult = 0;

	int iIndex = 0;
	BYTE *byDest = (BYTE*)&dwResult;

	byDest[3] = *(pData + iIndex);
	++iIndex;
	byDest[2] = *(pData + iIndex);
	++iIndex;
	byDest[1] = *(pData + iIndex);
	++iIndex;
	byDest[0] = *(pData + iIndex);

	return dwResult;
}

//8�ֽ�תdouble
double DeviceConnect::ConvertEightByteToDouble(BYTE *pby)
{
	int index = 0;
	double dbDecimals = 0.0, dbPositive = 0.0;
	//��������
	DWORD dwInt = ConvertFourBytes(pby);
	dbPositive += dwInt;
	index += 4;
	//С������
	unsigned long long ull = 1;
	dwInt = ConvertFourBytes(pby + index);
	dbDecimals = ((double)dwInt) / (double)(ull << 32);
	dbPositive += dbDecimals;
	return dbPositive;
}