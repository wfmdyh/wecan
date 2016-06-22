#include "stdafx.h"
#include "TCPModel.h"
#include "DevPipeNet.h"
#include "DeviceConnect.h"
#include "DataDef.h"
#include "channel.h"
#include "DevWaterMeter.h"
#include "DevPressureAlarm.h"
#include "DevMultipleParam.h"

DeviceConnect::DeviceConnect()
	:DATA_BUFFER_SIZE(1024*1024)
{
	m_DeviceID = 0;
	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1, DATA_BUFFER_SIZE);

	//为设备对象分配内存空间
	m_Device = new(nothrow)DevPipeNet();
	//m_Parent = NULL;
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
		ShowMessage(L"错误的帧，不是以0x68开头，舍弃全部...");
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
		ShowMessage(L"不是完整的帧，继续接收...");
		return 2;
	}
	//检查包尾
	BYTE byEnd = *(dataTmp + (wPackageLen - 1));
	if (byEnd != 0x16)
	{
		ShowMessage(L"错误的帧，不是以0x16结束，舍弃全部...");
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
		ShowMessage(L"错误的帧，校验和错误");
		return 3;
	}
	return 1;
}

//尝试解析
void DeviceConnect::TryAnalysis()
{
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
		memset(m_pRecDataBuf, 0, DATA_BUFFER_SIZE);
	}
	else{
		memcpy(m_pRecDataBuf, m_pRecDataBuf + iNum, iRemainLen);
		memset(m_pRecDataBuf + iRemainLen, 0, DATA_BUFFER_SIZE - iRemainLen);
		m_iDataRear -= iNum;
	}
}

void DeviceConnect::AnalyzeData(BYTE *szBuf, WORD wPackageLen)
{
	DWORD dwIndex = 7;		//定位在ID
	//ID
	m_DeviceID = ConvertFourBytes(szBuf + dwIndex);
	dwIndex += 4;
	wstringstream wss;
	wss << m_DeviceID;
	wstring strID = wss.str();
	wss.clear();
	m_Device->m_DeviceID = strID;

	//压力数据长度
	WORD wPressureLen = 0;
	//判断TAG值
	BYTE tag = *(szBuf + dwIndex);
	while (dwIndex < (wPackageLen - (unsigned)2))
	{
		//指向数据位
		dwIndex++;
		switch (tag)
		{
		case 0x00:
			ShowMessage(L"设备上电测试");
			++dwIndex;
			break;
		case 0x01:
			m_Device->m_AlarmState = *(szBuf + dwIndex);
			++dwIndex;
			break;
		case 0x02:
			m_Device->m_TotalFlow = ConvertEightByteToDouble(szBuf + dwIndex);
			dwIndex += 8;
			break;
		case 0x03:
			m_Device->m_QuantityOfHeat = ConvertEightByteToDouble(szBuf + dwIndex);
			dwIndex += 8;
			break;
		case 0x04:
		{
			//压力告警
			m_Device->m_PAValue = ConvertTwoBytes(szBuf + dwIndex);
			dwIndex += 2;
			m_Device->m_PAState = *(szBuf + dwIndex);
			++dwIndex;
			break;
		}
		case 0x05:
		{
			WORD wLen = ConvertTwoBytes(szBuf + dwIndex);
			dwIndex += 2;
			for (DWORD i = 0; i < wLen; i++)
			{
				WORD* pwValue = (WORD*)(szBuf + dwIndex);
				m_Device->m_ArrTemperature.push_back(*pwValue);
				dwIndex += 2;
			}
			break;
		}
		case 0x06:
		{
			WORD wLen = ConvertTwoBytes(szBuf + dwIndex);
			dwIndex += 2;
			for (DWORD i = 0; i < wLen; i++)
			{
				WORD* pwValue = (WORD*)(szBuf + dwIndex);
				m_Device->m_ArrPressure.push_back(*pwValue);
				dwIndex += 2;
			}
			break;
		}
		case 0x07:
		{
			float val = 0;
			memcpy(&val, szBuf + dwIndex, 4);
			m_Device->m_Flow = val;
			dwIndex += 4;
			break;
		}
		case 0x08:
		{
			float val = 0;
			memcpy(&val, szBuf + dwIndex, 4);
			m_Device->m_Temperature = val;
			dwIndex += 4;
			break;
		}
		case 0x09:
		{
			float val = 0;
			memcpy(&val, szBuf + dwIndex, 4);
			m_Device->m_Pressure = val;
			dwIndex += 4;
			break;
		}
		case 0x0A:
		{
			//前一天时间
			BYTE by[3] = { 0 };
			memcpy(by, szBuf + dwIndex, 3);
			m_Device->m_Date.wYear = by[0] + 2000;
			m_Device->m_Date.wMonth = by[1];
			m_Device->m_Date.wDay = by[2];
			dwIndex += 3;
			break;
		}
		case 0x0b:
		{
			m_Device->m_SucessRate = *(szBuf + dwIndex);
			dwIndex++;
			break;
		}
		case 0x0c:
		{
			BYTE by[5] = { 0 };
			memcpy(by, szBuf + dwIndex, 5);
			m_Device->m_CurrentTime.wYear = by[0] + 2000;
			m_Device->m_CurrentTime.wMonth = by[1];
			m_Device->m_CurrentTime.wDay = by[2];

			m_Device->m_CurrentTime.wHour = by[3];
			m_Device->m_CurrentTime.wMinute = by[4];
			dwIndex += 5;
			break;
		}
		case 0x0d:
		{
			m_Device->m_State = *(szBuf + dwIndex);
			++dwIndex;
			break;
		}
		case 0x0e:
		{
			//小无线状态
			m_Device->m_SmallState = *(szBuf + dwIndex);
			++dwIndex;
			break;
		}
		case 0x0f:
		{
			WORD wLen = ConvertTwoBytes(szBuf + dwIndex);
			dwIndex += 2;
			//数据长度34
			wLen /= 34;
			for (DWORD i = 0; i < wLen; i++)
			{
				DevWaterMeter* pMeter = new DevWaterMeter;
				DWORD dwMeterID = ConvertFourBytes(szBuf + dwIndex);
				wss << dwMeterID;
				pMeter->m_DeviceID = wss.str();
				wss.clear();
				dwIndex += 4;

				pMeter->m_State = *(szBuf + dwIndex);
				++dwIndex;

				pMeter->m_TotalFlow = ConvertEightByteToDouble(szBuf + dwIndex);
				dwIndex += 8;

				pMeter->m_HeatQuantity = ConvertEightByteToDouble(szBuf + dwIndex);
				dwIndex += 8;

				memcpy(&(pMeter->m_Flow), szBuf + dwIndex, 4);
				dwIndex += 4;

				memcpy(&(pMeter->m_Pressure), szBuf + dwIndex, 4);
				dwIndex += 4;
				
				memcpy(&(pMeter->m_Temperature), szBuf + dwIndex, 4);
				dwIndex += 4;

				pMeter->m_RFState = *(szBuf + dwIndex);
				++dwIndex;

				m_Device->m_ArrMeter.push_back(pMeter);
			}
			break;
		}
		case 0x10:
		{
			//表校正系数
			DWORD dwValue = ConvertFourBytes(szBuf + dwIndex);
			//ID
			m_Device->m_Verify[0] = dwValue;
			dwIndex += 4;
			//数据
			memcpy(&(m_Device->m_Verify[1]), szBuf + dwIndex, 4);
			dwIndex += 4;
			break;
		}
		case 0x11:
		{
			//实时压力告警
			WORD wLen = ConvertTwoBytes(szBuf + dwIndex);
			//数据长度8
			wLen /= 8;
			dwIndex += 2;
			for (DWORD i = 0; i < wLen; i++)
			{
				DevPressureAlarm* pAlarm = new DevPressureAlarm;
				DWORD dwValue = ConvertFourBytes(szBuf + dwIndex);
				pAlarm->SetDeviceID(dwValue);
				dwIndex += 4;

				//时间
				BYTE byTime[2] = { 0 };
				memcpy(byTime, szBuf + dwIndex, 2);
				pAlarm->m_Time.wHour = byTime[0];
				pAlarm->m_Time.wMinute = byTime[1];
				dwIndex += 2;
				//压力
				pAlarm->m_Pressure = *(szBuf + dwIndex);
				++dwIndex;
				//告警状态
				pAlarm->m_State = *(szBuf + dwIndex);
				++dwIndex;
				m_Device->m_ArrPressureAlarm.push_back(pAlarm);
			}
			break;
		}
		case 0x12:
		{
			//多参数数据
			WORD wLen = ConvertTwoBytes(szBuf + dwIndex);
			//数据长度53
			wLen /= 53;
			dwIndex += 2;
			for (DWORD i = 0; i < wLen; i++)
			{
				DevMultipleParam* pParam = new DevMultipleParam;
				pParam->SetDeviceID(ConvertFourBytes(szBuf + dwIndex));
				dwIndex += 4;
				//状态
				pParam->m_State = *(szBuf + dwIndex);
				++dwIndex;
				//累计气泡数
				dwIndex += 4;
				//正累计流量
				pParam->m_PositiveTotalFlow = ConvertEightByteToDouble(szBuf + dwIndex);
				dwIndex += 8;
				//负累计流量
				pParam->m_NegativeTotalFlow = ConvertEightByteToDouble(szBuf + dwIndex);
				dwIndex += 8;
				//瞬时流量
				memcpy(&(pParam->m_Flow), szBuf + dwIndex, 4);
				dwIndex += 4;
				//最大瞬时流量
				BYTE byMaxTime[2] = { 0 };
				memcpy(byMaxTime, szBuf + dwIndex, 2);
				dwIndex += 2;
				pParam->m_MaxFlowTime.wHour = byMaxTime[0];
				pParam->m_MaxFlowTime.wMinute = byMaxTime[1];

				memcpy(&(pParam->m_MaxFlow), szBuf + dwIndex, 4);
				dwIndex += 4;
				//最小瞬时流量
				BYTE byMinTime[2] = { 0 };
				memcpy(byMinTime, szBuf + dwIndex, 2);
				dwIndex += 2;
				pParam->m_MinFlowTime.wHour = byMinTime[0];
				pParam->m_MinFlowTime.wMinute = byMinTime[1];

				memcpy(&(pParam->m_MinFlow), szBuf + dwIndex, 4);
				dwIndex += 4;
				//压力
				pParam->m_Pressure = *(szBuf + dwIndex);
				++dwIndex;
				//温度
				pParam->m_Temperature = ConvertTwoBytes(szBuf + dwIndex);
				dwIndex += 2;
				//时间
				BYTE byTime[8] = { 0 };
				memcpy(byTime, szBuf + dwIndex, 8);
				pParam->m_DateTime.wYear = byTime[0] + 2000;
				pParam->m_DateTime.wMonth = byTime[1];
				pParam->m_DateTime.wDay = byTime[2];
				pParam->m_DateTime.wHour = byTime[3];
				pParam->m_DateTime.wMinute = byTime[4];
				pParam->m_DateTime.wSecond = byTime[5];
				pParam->m_DateTime.wMilliseconds = ConvertTwoBytes(&byTime[6]);
				dwIndex += 8;
				//状态
				pParam->m_State = *(szBuf + dwIndex);
				++dwIndex;
				m_Device->m_ArrMultipleParam.push_back(pParam);
			}
			break;
		}
		case 0xEF:
		{
			m_Device->m_VerMeasurementID = ConvertFourBytes(szBuf + dwIndex);
			dwIndex += 4;
			memcpy(m_Device->m_VerMeasurementNum, szBuf + dwIndex, 6);
			dwIndex += 6;
			break;
		}
		case 0xF0:
		{
			m_Device->m_VerCommunicationHNum = *((WORD*)(szBuf + dwIndex));
			dwIndex += 2;
			m_Device->m_VerCommunicationSNum = *((WORD*)(szBuf + dwIndex));
			dwIndex += 2;
			break;
		}
		}

		tag = *(szBuf + dwIndex);
	}
	//时间戳
	m_Device->SetUploadTime(NULL);
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

wstring DeviceConnect::GetDeviceIDStr()
{
	wstringstream wss;
	wss << m_DeviceID;
	return wss.str();
}