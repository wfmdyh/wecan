#include "stdafx.h"
#include "DeviceConnect.h"

static const int DATA_BUF_SIZE = 1024;

DeviceConnect::DeviceConnect()
{
    TempID = "";

	m_iDataRear = 0;
	//1M
	m_pRecDataBuf = (char*)calloc(1024, 1024);

	//Ϊ�豸��������ڴ�ռ�
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
���ͻ����յ������ݿ��������ݶ�����
*/
void DeviceConnect::CopyDataToBuf(char *pData, DWORD dwDataLen)
{
	memcpy(m_pRecDataBuf + m_iDataRear, pData, dwDataLen);
	m_iDataRear += dwDataLen;
	//ÿ�յ�һ�����ݰ����ͳ��Խ���һ��
	TryAnalysis();
}

//���Խ���
void DeviceConnect::TryAnalysis()
{
	OutputDebugString(L"�������ݽ���\n");

	while (m_iDataRear > 1)
	{
		//��������
		//����< ������� > ��ȡ����Σ�����ٴη��� < ������
		BYTE *oneData = NULL;
		WORD wLen = 0;
		char *dataTmp = m_pRecDataBuf;
		//�ҵ���ͷ���<������û�õ�����
		while (((*dataTmp != '<') || (*dataTmp == 0)) && m_iDataRear > 0)
		{
			//�������ּ��ʺܵڣ�����ÿ�ζ������������ѭ����Ч��
			//����ǰ�������
			RemoveFrontData(1);
			//dataTmp++;
		}
		//�ж��ǲ��������ݣ����Ϊ0˵��û����
		if (*dataTmp == '<')
		{
			dataTmp++;
			wLen++;
			while (*dataTmp != '>' && *dataTmp != '<')
			{
				wLen++;
				if (wLen == m_iDataRear)
				{
					//�����β
					break;
				}
				dataTmp++;
			}

			if (wLen == m_iDataRear || *dataTmp == '<')
			{
				//�������Ϣ������
				RemoveFrontData(wLen);
			}
			else{
				wLen++;
				//�ж����ݳ���
				if (m_iDataRear >= wLen)
				{
					//�������ݣ��������ַ���������Ҫ+1
					oneData = (BYTE*)malloc(wLen + 1);
					memset(oneData, 0, wLen + 1);
					memcpy(oneData, m_pRecDataBuf, wLen);
					//�ƶ�����
					RemoveFrontData(wLen);
				}
			}
		}
		if (oneData != NULL)
		{
			//��������
			AnalyzeData(oneData);
			//�ͷ��ڴ��
			free(oneData);
			oneData = NULL;
		}
	}
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
	//DebugPrintf("�������ݣ�%d\n",iNum);
}

//������ݱ�����ȷ������FALSE
BOOL DeviceConnect::AnalyzeData(BYTE *szBuf)
{
	
	//<51000260:TEMP=28.8��><00000005:CSQ=16>
	string strData((char*)szBuf);
	int index = strData.find(':');
	if (index <= 0)
	{
		//<ID=51001556><STAT=0,20,V1.3.1><TIME=1970-02-24,06-00-14><TEMP=16.6,0,1970-02-24,06-00-00> Э��
		string::size_type valBegin = strData.find('=') + 1;
		string temp = strData.substr(1, strData.find('=') - 1);
		if (temp.compare("ID") == 0)
		{
			//ֵ������
			string::size_type end = strData.rfind('>');
			if (TempID.empty())
			{
				TempID = strData.substr(valBegin, end - valBegin);
			}
			//����ID
			m_Device->SetDeviceID(TempID);
			return TRUE;
		}
		else if (temp.compare("STAT") == 0)
		{
			//ֵ������
			string::size_type end = strData.find_first_of(',');
			string STAT = strData.substr(valBegin, end - valBegin);
			return TRUE;
		}
		else if (temp.compare("TIME") == 0)
		{
			//ֵ������
			string::size_type end = strData.find_first_of(',');
			string TIME = strData.substr(valBegin, end - valBegin);
			return TRUE;
		}
		else if (temp.compare("TEMP") == 0)
		{
			//ֵ������
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

			//���ûص�����
			m_Parent->GetOneDevice(m_Device);

			return TRUE;
		}
	}
	//��ȡID
	string strID = strData.substr(1, strData.find(':') - 1);
	//ֵ������
	string::size_type end = strData.find_first_of(',');
	if (end > strData.length())
	{
		end = strData.rfind('>');
	}
	string::size_type valBegin = strData.find('=') + 1;
	//����ID
	m_Device->SetDeviceID(strID);

	string strVal = strData.substr(valBegin, end - valBegin);
	//�¶�
	if (strData.find("TEMP") != string::npos)
	{
		if (strData.find("��") != string::npos)
		{
			strVal = strData.substr(valBegin, end - valBegin - 2);
		}
		if (atof(strVal.c_str())>100 || atof(strVal.c_str())<-40)
		{
			string::size_type valBegin = strData.find('<');
			WriteErrToFile((char*)szBuf, end - valBegin+1);
			return TRUE;
		}
		//��ֵ
		m_Device->m_values[RI_Temperature].vt = VT_R4;
		m_Device->m_values[RI_Temperature].fltVal = (float)atof(strVal.c_str());

		//���ûص�����
		m_Parent->GetOneDevice(m_Device);
		return TRUE;
	}
	//��������ַ
	//else if (strData.find("GCNT") != string::npos)
	//{
	//	m_Device->m_values[RI_SRV_ADD].vt = VT_BSTR;
	//	m_Device->m_values[RI_SRV_ADD].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////�������˿�
	//else if (strData.find("PORT") != string::npos)
	//{
	//	m_Device->m_values[RI_SRV_PORT].vt = VT_BSTR;
	//	m_Device->m_values[RI_SRV_PORT].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////���Ӽ��
	//else if (strData.find("TCNT") != string::npos)
	//{
	//	m_Device->m_values[RI_TCNT].vt = VT_BSTR;
	//	m_Device->m_values[RI_TCNT].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////ͨѶ���
	//else if (strData.find("TCOM") != string::npos)
	//{
	//	m_Device->m_values[RI_TCOM].vt = VT_BSTR;
	//	m_Device->m_values[RI_TCOM].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////ʪ��
	//else if (strData.find("HUMI") != string::npos)
	//{
	//	m_Device->m_values[RI_HUMI].vt = VT_BSTR;
	//	m_Device->m_values[RI_HUMI].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	////��ص͵���
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
	////�ź�ǿ��
	//else if (strData.find("CSQ") != string::npos)
	//{
	//	m_Device->m_values[RI_CSQ].vt = VT_BSTR;
	//	m_Device->m_values[RI_CSQ].bstrVal =
	//	_com_util::ConvertStringToBSTR(strVal.c_str());
	//	return TRUE;
	//}
	return TRUE;
}



//��������Ϣд����־�ļ�
void DeviceConnect::WriteErrToFile(char *pData, DWORD dwLen)
{
	string strFilePath = "C://IOServer��־//";
	//�ж��ļ��Ƿ����
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//Ŀ¼�����ڣ�����Ŀ¼
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			MessageBox(NULL, L"�޷�������־�ļ�", L"����", MB_OK | MB_ICONERROR);
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szTime[20] = { 0 };
	sprintf(szTime, "�쳣%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
	strFilePath += szTime;
	//׷��ֻд
	FILE *fe = fopen(strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		return;
	}
	//д��ʱ��
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