#include "stdafx.h"
#include "DeviceConnectCon.h"


DeviceConnectCon::DeviceConnectCon()
{
	m_DeviceID = 0;
	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1, DATA_BUF_SIZE);

	//Ϊ�豸��������ڴ�ռ�
	m_Device = new(nothrow)DevConcentrator();
	m_Parent = NULL;
}


DeviceConnectCon::~DeviceConnectCon()
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
int DeviceConnectCon::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;

	return CheckPackage();
}

//��֤֡�Ƿ���ȷ
int DeviceConnectCon::CheckPackage()
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
	//�Ƿ�Ϊ���һ֡
	BYTE *bpNum = (BYTE*)(m_pRecDataBuf + 7);
	BYTE bFIN = bpNum[0];
	if ((bFIN&0x40) == 0)
	{
		cout << "�м�֡" << endl;
		return 2;
	}
	cout << "����֡" << endl;
	return 1;
}

//���Խ���
void DeviceConnectCon::TryAnalysis()
{
	do{
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
	} while (m_iDataRear > 1);
	
}

//��������
void DeviceConnectCon::RemoveFrontData(int iNum)
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
BOOL DeviceConnectCon::AnalyzeData(BYTE *szBuf, WORD wPackageLen)
{
	DWORD dwIndex = 9;		//��λ��ID
	//ID
	m_DeviceID = ConvertFourBytes(szBuf + dwIndex);
	m_Device->m_DeviceID = m_DeviceID;
	dwIndex += 4;

	//�ж�TAGֵ
	BYTE tag = *(szBuf + dwIndex);
	while (dwIndex < (wPackageLen - 2))
	{
		//ָ������λ
		dwIndex++;
		switch (tag)
		{
		case 0x00:
			//printf("�豸��%s���ϵ����\n", strID.c_str());
			cout << "�豸��" << m_DeviceID << "�ϵ����" << endl;
			++dwIndex;
			break;
		case 0x01:
			m_Device->m_SuccessRate = *(szBuf + dwIndex);
			++dwIndex;
			break;
		case 0x02:
		{
			//ˮ��(С��)����
			//����
			WORD wSize = ConvertTwoBytes(szBuf + dwIndex);
			wSize = wSize / 14;		//����ˮ��ռ�õ��ֽ���
			dwIndex += 2;
			for (unsigned i = 0; i < wSize; i++)
			{
				SmallMeter sm;
				sm.m_LastTime = time(NULL);
				sm.m_DeviceID = *((DWORD*)(szBuf + dwIndex));
				dwIndex += 4;
				sm.SetMeterStatus(*(szBuf + dwIndex));
				++dwIndex;
				sm.SetMinSpeed(*(szBuf + dwIndex));
				++dwIndex;
				sm.SetTotalFlow(ConvertEightByteToDouble(szBuf + dwIndex));
				dwIndex += 8;
				m_Device->m_vecSmallMeters.push_back(sm);
			}
			break;
		}
		
		}

		tag = *(szBuf + dwIndex);
	}
	//ʱ���
	m_Device->SetLastTime(time(NULL));
	m_Parent->m_pProtocolMgr->UpdateConcentrator(*m_Device);
	//���ûص�����
	//m_Parent->GetOneDevice(m_Device);

	return TRUE;
}

//2�ֽ�ת��
WORD DeviceConnectCon::ConvertTwoBytes(BYTE *pData)
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
DWORD DeviceConnectCon::ConvertFourBytes(BYTE *pData)
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
double DeviceConnectCon::ConvertEightByteToDouble(BYTE *pby)
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