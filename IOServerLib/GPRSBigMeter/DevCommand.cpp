#include "stdafx.h"
#include "DevCommand.h"


DevCommand::DevCommand()
{
	for (int i = 0; i < 4; i++)
	{
		m_DeviceID[i] = 0;
	}
	//����Ĭ��ֵ
	m_PressureUpper = 80;
	m_PressureFloor = 10;
	m_MeasureCycle = 10;
	m_PressureRange = 2;

	m_UploadRate = 1;
	m_UploadTime1[0] = 1;				//GPRS�ϴ�ʱ�� ʱ����2���ֽڵ�������ʾ
	m_UploadTime1[1] = 0;
	m_UploadTime2[0] = 13;
	m_UploadTime2[1] = 0;
	for (int i = 0; i < 11; i++)
	{
		m_TelephoneNumber[i] = 0;			//ѹ�����ϱ��õ绰���� 11����Ч��ֵ
	}
	for (int i = 0; i < 6; i++)
	{
		m_Time[i] = 0;						//ʱ��У׼��Ϣ �ꡢ�¡��ա�ʱ���֡���
	}
}


DevCommand::~DevCommand()
{
}

BOOL DevCommand::SetPressureUpper(float fMPa)
{
	//0-1.6
	int nValue = (int)(fMPa * 100);
	//����С������
	if (nValue <= m_PressureFloor || nValue > 160 || nValue < 0)
	{
		return FALSE;
	}
	m_PressureUpper = (BYTE)nValue;
	return TRUE;
}

BOOL DevCommand::SetPressureFloor(float fMPa)
{
	//0-1.6
	int nValue = (int)(fMPa * 100);
	//���ܴ�������
	if (nValue <= m_PressureFloor || nValue > 160 || nValue < 0)
	{
		return FALSE;
	}
	m_PressureFloor = (BYTE)nValue;
	return TRUE;

}

BOOL DevCommand::SetMeasureCycle(int nCycle)
{
	switch (nCycle)
	{
	case 5:
	case 10:
	case 15:
		m_MeasureCycle = nCycle;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL DevCommand::SetPressureRange(int nRange)
{
	if (nRange >= 1 && nRange <= 3)
	{
		m_PressureRange = nRange;
		return TRUE;
	}
	return FALSE;
}

BOOL DevCommand::SetUploadRate(int nRate)
{
	switch (nRate)
	{
	case 1:
	case 2:
		m_UploadRate = nRate;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL DevCommand::SetUploadTime1(WORD wHour, WORD wMinute)
{
	if (wHour <= 23 && wHour >= 0)
	{
		m_UploadTime1[0] = (BYTE)wHour;
		if (wMinute <= 59 && wMinute >= 0)
		{
			m_UploadTime1[1] = (BYTE)wMinute;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DevCommand::SetUploadTime2(WORD wHour, WORD wMinute)
{
	if (wHour <= 23 && wHour >= 0)
	{
		m_UploadTime2[0] = (BYTE)wHour;
		if (wMinute <= 59 && wMinute >= 0)
		{
			m_UploadTime2[1] = (BYTE)wMinute;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DevCommand::SetTelephoneNumber(DWORD dwPhoneNumber)
{
	//11λ��
	if (dwPhoneNumber < 10000000000)
	{
		return FALSE;
	}
	BYTE *pNumber = (BYTE*)&dwPhoneNumber;
	for (int i = 0; i < 11; i++)
	{
		m_TelephoneNumber[i] = pNumber[10 - i];
	}
	return TRUE;
}

void DevCommand::SetDeviceID(DWORD dwID)
{
	BYTE *byID = (BYTE*)&dwID;
	m_DeviceID[3] = byID[0];
	m_DeviceID[2] = byID[1];
	m_DeviceID[1] = byID[2];
	m_DeviceID[0] = byID[3];
}

void DevCommand::SetTime()
{
	//��ǰʱ��
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_Time[0] = (BYTE)(st.wYear - 2000);
	m_Time[1] = (BYTE)st.wMonth;
	m_Time[2] = (BYTE)st.wDay;
	m_Time[3] = (BYTE)st.wHour;
	m_Time[4] = (BYTE)st.wMinute;
	m_Time[5] = (BYTE)st.wSecond;
}



//�����·�ָ��
BOOL DevCommand::GetCommand(CommandType cType, SendCommand::DEVICE_TYPE devType)
{
	if (devType == SendCommand::DEVICE_TYPE::TYPE_BM)
	{
		//���ָ��
		GenerateBMCommand(cType);
	}
	else{
		//�������·�ָ��
		GenerateConCommand(cType);
	}
	return TRUE;
}

void DevCommand::GenerateConCommand(CommandType cType)
{
	m_vecCommand.clear();

	SetTime();

	WORD wPackageLen = 0;
	BYTE *byPL = (BYTE*)&wPackageLen;

	m_vecCommand.push_back(0x68);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x68);
	//Э��汾
	m_vecCommand.push_back(0x07);
	m_vecCommand.push_back(0x01);
	//AFN
	if (cType == NO_PARAM)
	{
		//��������
		m_vecCommand.push_back(0x81);
	}
	else{
		//������
		
	}
	//��������
	m_vecCommand.push_back(0xC0);
	m_vecCommand.push_back(0x00);
	//ID
	for (unsigned i = 0; i < 4; i++)
	{
		m_vecCommand.push_back(m_DeviceID[i]);
	}

	//ʱ��У��
	m_vecCommand.push_back(0xFF);
	for (unsigned i = 0; i < 6; i++)
	{
		m_vecCommand.push_back(m_Time[i]);
	}
	//��������
	wPackageLen = m_vecCommand.size() + 2;
	m_vecCommand.at(1) = byPL[1];
	m_vecCommand.at(2) = byPL[0];
	//У���
	BYTE byCS = 0;
	DWORD dwSum = 0;
	for (unsigned i = 0; i < m_vecCommand.size(); i++)
	{
		dwSum += m_vecCommand.at(i);
	}
	byCS = dwSum % 256;
	m_vecCommand.push_back(byCS);
	//��β��ʶ
	m_vecCommand.push_back(0x16);
}

void DevCommand::GenerateBMCommand(CommandType cType)
{
	m_vecCommand.clear();
	//vecCommand.resize(7);
	SetTime();

	WORD wPackageLen = 0;
	BYTE *byPL = (BYTE*)&wPackageLen;

	m_vecCommand.push_back(0x68);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x00);
	m_vecCommand.push_back(0x68);
	//Э��汾
	m_vecCommand.push_back(0x06);
	m_vecCommand.push_back(0x01);
	if (cType == NO_PARAM)
	{
		//��������
		m_vecCommand.push_back(0x81);
		//ID
		for (unsigned i = 0; i < 4; i++)
		{
			m_vecCommand.push_back(m_DeviceID[i]);
		}
	}
	else{
		//������
		m_vecCommand.push_back(0xC1);
		//ID
		for (unsigned i = 0; i < 4; i++)
		{
			m_vecCommand.push_back(m_DeviceID[i]);
		}
		//TAG
		m_vecCommand.push_back(0x01);
		m_vecCommand.push_back(m_PressureUpper);
		m_vecCommand.push_back(0x02);
		m_vecCommand.push_back(m_PressureFloor);
		m_vecCommand.push_back(0x03);
		m_vecCommand.push_back(m_MeasureCycle);
		m_vecCommand.push_back(0x04);
		m_vecCommand.push_back(m_PressureRange);
		m_vecCommand.push_back(0x05);
		m_vecCommand.push_back(m_UploadRate);
		m_vecCommand.push_back(0x06);
		m_vecCommand.push_back(m_UploadTime1[0]);
		m_vecCommand.push_back(m_UploadTime1[1]);
		m_vecCommand.push_back(0x07);
		m_vecCommand.push_back(m_UploadTime2[0]);
		m_vecCommand.push_back(m_UploadTime2[1]);
		m_vecCommand.push_back(0x08);
		for (unsigned i = 0; i < 11; i++)
		{
			m_vecCommand.push_back(m_TelephoneNumber[i]);
		}
	}
	//ʱ��У��
	m_vecCommand.push_back(0xFF);
	for (unsigned i = 0; i < 6; i++)
	{
		m_vecCommand.push_back(m_Time[i]);
	}
	//��������
	wPackageLen = m_vecCommand.size() + 2;
	m_vecCommand.at(1) = byPL[1];
	m_vecCommand.at(2) = byPL[0];
	//У���
	BYTE byCS = 0;
	DWORD dwSum = 0;
	for (unsigned i = 0; i < m_vecCommand.size(); i++)
	{
		dwSum += m_vecCommand.at(i);
	}
	byCS = dwSum % 256;
	m_vecCommand.push_back(byCS);
	//��β��ʶ
	m_vecCommand.push_back(0x16);
}

vector<BYTE> DevCommand::GetHexCommand()
{
	return m_vecCommand;
}

void DevCommand::operator = (DevCommand &cmd)
{
	memcpy(m_DeviceID, cmd.m_DeviceID, 4);					//�豸ID
	m_PressureUpper = cmd.m_PressureUpper;					//ѹ������
	m_PressureFloor = cmd.m_PressureFloor;					//ѹ������
	m_MeasureCycle = cmd.m_MeasureCycle;					//ѹ����������5, 10, 15
	m_PressureRange = cmd.m_PressureRange;					//ѹ������1 2 3
	m_UploadRate = cmd.m_UploadRate;						//GPRS�ϴ�Ƶ��1 2
	memcpy(m_UploadTime1, cmd.m_UploadTime1, 2);			//GPRS�ϴ�ʱ�� ʱ����2���ֽڵ�������ʾ
	memcpy(m_UploadTime2, cmd.m_UploadTime2, 2);
	memcpy(m_TelephoneNumber, cmd.m_TelephoneNumber, 11);	//ѹ�����ϱ��õ绰���� 11����Ч��ֵ
	memcpy(m_Time, cmd.m_Time, 6);							//ʱ��У׼��Ϣ �ꡢ�¡��ա�ʱ���֡���
}

