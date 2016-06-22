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

	//为设备对象分配内存空间
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
将客户端收到的数据拷贝到数据队列中
*/
int DeviceConnect::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;
	//每收到一个数据包，就尝试解析一次
	
	return CheckPackage();
}

//验证帧是否正确
int DeviceConnect::CheckPackage()
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
	return 1;
}

//尝试解析
void DeviceConnect::TryAnalysis()
{
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
}

//舍弃数据
void DeviceConnect::RemoveFrontData(int iNum)
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
BOOL DeviceConnect::AnalyzeData(BYTE *szBuf, WORD wPackageLen)
{	
	DWORD dwIndex = 7;		//定位在ID
	//ID
	m_DeviceID = ConvertFourBytes(szBuf + dwIndex);
	dwIndex += 4;
	string strID = str_format("%u", m_DeviceID);
	m_Device->SetDeviceID(strID);

	//压力数据长度
	WORD wPressureLen = 0;
	//判断TAG值
	BYTE tag = *(szBuf + dwIndex);
	while (dwIndex < (wPackageLen-2))
	{
		//指向数据位
		dwIndex++;
		switch (tag)
		{
		case 0x00:
			printf("设备：%s，上电测试\n", strID.c_str());
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
			//压力告警
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
			dwIndex += 2;		//定位到数据块

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
	//时间戳
	m_Device->SetLastTime(time(NULL));
	m_Parent->m_pProtocolMgr->UpdateBigMeter(*m_Device);
	//调用回调函数
	//m_Parent->GetOneDevice(m_Device);
	
	return TRUE;
}

//2字节转换
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

//4字节转换
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

//8字节转double
double DeviceConnect::ConvertEightByteToDouble(BYTE *pby)
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