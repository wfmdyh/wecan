#include "stdafx.h"
#include "DeviceConnect.h"

static const int DATA_BUF_SIZE = 1024;

DeviceConnect::DeviceConnect()
{
    TempID = "";

	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1024, 1024);

	//为设备对象分配内存空间
	m_Device = new CGPRSTemperature();
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
void DeviceConnect::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;
	//每收到一个数据包，就尝试解析一次
	TryAnalysis();
}

//尝试解析
void DeviceConnect::TryAnalysis()
{
	OutputDebugString(L"尝试数据解析\n");

	while (m_iDataRear > 1)
	{
		//解析数据
		//查找< 如果发现 > 就取出这段，如果再次发现 < 就舍弃
		BYTE *oneData = NULL;
		WORD wLen = 0;
		char *dataTmp = m_pRecDataBuf;
		//找到开头标记<，舍弃没用的数据
		while (((*dataTmp != '<') || (*dataTmp == 0)) && m_iDataRear > 0)
		{
			//由于这种几率很第，所以每次都舍弃，提高外循环的效率
			//舍弃前面的数据
			RemoveFrontData(1);
			//dataTmp++;
		}
		//判断是不是有数据，如果为0说明没数据
		if (*dataTmp == '<')
		{
			dataTmp++;
			wLen++;
			while (*dataTmp != '>' && *dataTmp != '<')
			{
				wLen++;
				if (wLen == m_iDataRear)
				{
					//到达结尾
					break;
				}
				dataTmp++;
			}

			if (wLen == m_iDataRear || *dataTmp == '<')
			{
				//错误的信息，舍弃
				RemoveFrontData(wLen);
			}
			else{
				wLen++;
				//判断数据长度
				if (m_iDataRear >= wLen)
				{
					//拷贝数据，由于是字符串，所以要+1
					oneData = (BYTE*)malloc(wLen + 1);
					memset(oneData, 0, wLen + 1);
					memcpy(oneData, m_pRecDataBuf, wLen);
					//移动数据
					RemoveFrontData(wLen);
				}
			}
		}
		if (oneData != NULL)
		{
			//解析数据
			AnalyzeData(oneData);
			//释放内存块
			free(oneData);
			oneData = NULL;
		}
	}
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
	//DebugPrintf("舍弃数据：%d\n",iNum);
}

//如果数据报不正确，返回FALSE
BOOL DeviceConnect::AnalyzeData(BYTE *szBuf)
{
	
	//<51000260:TEMP=28.8℃><00000005:CSQ=16>
	string strData((char*)szBuf);
	int index = strData.find(':');
	if (index <= 0)
	{
		//<ID=51001556><STAT=0,20,V1.3.1><TIME=1970-02-24,06-00-14><TEMP=16.6,0,1970-02-24,06-00-00> 协议
		string::size_type valBegin = strData.find('=') + 1;
		string temp = strData.substr(1, strData.find('=') - 1);
		if (temp.compare("ID") == 0)
		{
			//值的区间
			string::size_type end = strData.rfind('>');
			if (TempID.empty())
			{
				TempID = strData.substr(valBegin, end - valBegin);
			}
			//设置ID
			m_Device->SetDeviceID(TempID);
			return TRUE;
		}
		else if (temp.compare("STAT") == 0)
		{
			//值的区间
			string::size_type end = strData.find_first_of(',');
			string STAT = strData.substr(valBegin, end - valBegin);
			return TRUE;
		}
		else if (temp.compare("TIME") == 0)
		{
			//值的区间
			string::size_type end = strData.find_first_of(',');
			string TIME = strData.substr(valBegin, end - valBegin);
			return TRUE;
		}
		else if (temp.compare("TEMP") == 0)
		{
			//值的区间
			string::size_type end = strData.find_first_of(',');
			string TEMP = strData.substr(valBegin, end - valBegin);

			if (atof(TEMP.c_str())>100 || atof(TEMP.c_str())<-40)
			{
				string::size_type valBegin = strData.find('<');
				WriteErrToFile((char*)szBuf, end - valBegin + 1);
				return TRUE;
			}
			
			m_Device->m_values[RI_Temperature].vt = VT_R4;
			m_Device->m_values[RI_Temperature].fltVal = (float)atof(TEMP.c_str());
			//m_Device->IsChange = true;

			TempID = "";

			//调用回调函数
			m_Parent->GetOneDevice(m_Device);

			return TRUE;
		}
	}
	//获取ID
	string strID = strData.substr(1, strData.find(':') - 1);
	//值的区间
	string::size_type end = strData.find_first_of(',');
	if (end > strData.length())
	{
		end = strData.rfind('>');
	}
	string::size_type valBegin = strData.find('=') + 1;
	//设置ID
	m_Device->SetDeviceID(strID);

	string strVal = strData.substr(valBegin, end - valBegin);
	//温度
	if (strData.find("TEMP") != string::npos)
	{
		if (strData.find("℃") != string::npos)
		{
			strVal = strData.substr(valBegin, end - valBegin - 2);
		}
		if (atof(strVal.c_str())>100 || atof(strVal.c_str())<-40)
		{
			string::size_type valBegin = strData.find('<');
			WriteErrToFile((char*)szBuf, end - valBegin+1);
			return TRUE;
		}
		//赋值
		m_Device->m_values[RI_Temperature].vt = VT_R4;
		m_Device->m_values[RI_Temperature].fltVal = (float)atof(strVal.c_str());

		//调用回调函数
		m_Parent->GetOneDevice(m_Device);
		return TRUE;
	}
	//服务器地址
	//else if (strData.find("GCNT") != string::npos)
	//{
	//	m_Device->m_values[RI_SRV_ADD].vt = VT_BSTR;
	//	m_Device->m_values[RI_SRV_ADD].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////服务器端口
	//else if (strData.find("PORT") != string::npos)
	//{
	//	m_Device->m_values[RI_SRV_PORT].vt = VT_BSTR;
	//	m_Device->m_values[RI_SRV_PORT].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////连接间隔
	//else if (strData.find("TCNT") != string::npos)
	//{
	//	m_Device->m_values[RI_TCNT].vt = VT_BSTR;
	//	m_Device->m_values[RI_TCNT].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////通讯间隔
	//else if (strData.find("TCOM") != string::npos)
	//{
	//	m_Device->m_values[RI_TCOM].vt = VT_BSTR;
	//	m_Device->m_values[RI_TCOM].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////湿度
	//else if (strData.find("HUMI") != string::npos)
	//{
	//	m_Device->m_values[RI_HUMI].vt = VT_BSTR;
	//	m_Device->m_values[RI_HUMI].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////电池低电量
	//else if (strData.find("LOWPOWER") != string::npos)
	//{

	//	m_Device->m_values[RI_LOWPOWER].vt = VT_BSTR;
	//	m_Device->m_values[RI_LOWPOWER].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////CCID
	//else if (strData.find("CCID") != string::npos)
	//{
	//	m_Device->m_values[RI_CCID].vt = VT_BSTR;
	//	m_Device->m_values[RI_CCID].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////信号强度
	//else if (strData.find("CSQ") != string::npos)
	//{
	//	m_Device->m_values[RI_CSQ].vt = VT_BSTR;
	//	m_Device->m_values[RI_CSQ].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	return TRUE;
}



//将错误信息写入日志文件
void DeviceConnect::WriteErrToFile(char *pData, DWORD dwLen)
{
	string strFilePath = "C://IOServer日志//";
	//判断文件是否存在
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//目录不存在，创建目录
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			MessageBox(NULL, L"无法创建日志文件", L"错误", MB_OK | MB_ICONERROR);
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szTime[20] = { 0 };
	sprintf(szTime, "异常%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
	strFilePath += szTime;
	//追加只写
	FILE *fe = fopen(strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		return;
	}
	//写入时间
	memset(szTime, 0, 20);
	sprintf(szTime, "\r\n%02d:%02d:%02d:", st.wHour, st.wMinute, st.wSecond);
	fwrite(szTime, 1, strlen(szTime), fe);
	if (fwrite(pData, dwLen, 1, fe) < 1)
	{
		fclose(fe);
		return;
	}

	fclose(fe);
}