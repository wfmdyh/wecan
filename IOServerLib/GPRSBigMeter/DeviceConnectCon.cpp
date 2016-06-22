#include "stdafx.h"
#include "DeviceConnectCon.h"


DeviceConnectCon::DeviceConnectCon()
{
	m_DeviceID = 0;
	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1, DATA_BUF_SIZE);

	//为设备对象分配内存空间
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
将客户端收到的数据拷贝到数据队列中
*/
int DeviceConnectCon::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;

	return CheckPackage();
}

//验证帧是否正确
int DeviceConnectCon::CheckPackage()
{
	int iIndex = 0;
	//整包长度
	WORD wPackageLen = 0;
	char *dataTmp = m_pRecDataBuf;

	//检查包头
	if ((BYTE)(*dataTmp) != 0x68)
	{
		cout << "错误的帧，不是以0x68开头，舍弃全部...\n";
		//错误的帧，舍弃全部
		RemoveFrontData(m_iDataRear);
		return 3;
	}
	++iIndex;
	wPackageLen = ConvertTwoBytes((BYTE*)(dataTmp + iIndex));
	//iIndex += 2;
	//是否接收完一帧
	if (wPackageLen > m_iDataRear)
	{
		cout << "不是完整的帧，继续接收...\n";
		return 2;
	}
	//检查包尾
	BYTE byEnd = *(dataTmp + (wPackageLen - 1));
	if (byEnd != 0x16)
	{
		cout << "错误的帧，不是以0x16结束，舍弃全部...\n";
		//错误的帧，舍弃全部
		RemoveFrontData(m_iDataRear);
		return 3;
	}
	//校验和
	unsigned int uiSum = 0;
	BYTE byCS = *(dataTmp + (wPackageLen - 2));
	//计算校验和
	for (int i = 0; i < wPackageLen - 2; i++)
	{
		uiSum += *(dataTmp + i);
	}
	uiSum %= 256;
	if (byCS != uiSum)
	{
		cout << "错误的帧，校验和错误\n";
		return 3;
	}
	//是否为最后一帧
	BYTE *bpNum = (BYTE*)(m_pRecDataBuf + 7);
	BYTE bFIN = bpNum[0];
	if ((bFIN&0x40) == 0)
	{
		cout << "中间帧" << endl;
		return 2;
	}
	cout << "结束帧" << endl;
	return 1;
}

//尝试解析
void DeviceConnectCon::TryAnalysis()
{
	do{
		OutputDebugString(L"尝试数据解析\n");
		WORD sPackageLen = ConvertTwoBytes((BYTE*)(m_pRecDataBuf + 1));
		//拷贝数据，进行深度解析
		//解析数据
		BYTE *oneData = (BYTE*)calloc(1, sPackageLen);
		memcpy(oneData, m_pRecDataBuf, sPackageLen);
		RemoveFrontData(sPackageLen);

		AnalyzeData(oneData, sPackageLen);
		free(oneData);
		oneData = NULL;
	} while (m_iDataRear > 1);
	
}

//舍弃数据
void DeviceConnectCon::RemoveFrontData(int iNum)
{
	//错误的信息，舍弃
	int iRemainLen = m_iDataRear - iNum;
	if (iRemainLen < 0)
	{
		//超过最大长度就全部舍弃
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

//如果数据报不正确，返回FALSE
BOOL DeviceConnectCon::AnalyzeData(BYTE *szBuf, WORD wPackageLen)
{
	DWORD dwIndex = 9;		//定位在ID
	//ID
	m_DeviceID = ConvertFourBytes(szBuf + dwIndex);
	m_Device->m_DeviceID = m_DeviceID;
	dwIndex += 4;

	//判断TAG值
	BYTE tag = *(szBuf + dwIndex);
	while (dwIndex < (wPackageLen - 2))
	{
		//指向数据位
		dwIndex++;
		switch (tag)
		{
		case 0x00:
			//printf("设备：%s，上电测试\n", strID.c_str());
			cout << "设备：" << m_DeviceID << "上电测试" << endl;
			++dwIndex;
			break;
		case 0x01:
			m_Device->m_SuccessRate = *(szBuf + dwIndex);
			++dwIndex;
			break;
		case 0x02:
		{
			//水表(小表)数据
			//长度
			WORD wSize = ConvertTwoBytes(szBuf + dwIndex);
			wSize = wSize / 14;		//除以水表占用的字节数
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
	//时间戳
	m_Device->SetLastTime(time(NULL));
	m_Parent->m_pProtocolMgr->UpdateConcentrator(*m_Device);
	//调用回调函数
	//m_Parent->GetOneDevice(m_Device);

	return TRUE;
}

//2字节转换
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

//4字节转换
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

//8字节转double
double DeviceConnectCon::ConvertEightByteToDouble(BYTE *pby)
{
	int index = 0;
	double dbDecimals = 0.0, dbPositive = 0.0;
	//整数部分
	DWORD dwInt = ConvertFourBytes(pby);
	dbPositive += dwInt;
	index += 4;
	//小数部分
	unsigned long long ull = 1;
	dwInt = ConvertFourBytes(pby + index);
	dbDecimals = ((double)dwInt) / (double)(ull << 32);
	dbPositive += dbDecimals;
	return dbPositive;
}