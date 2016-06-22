#include "stdafx.h"
#include "TransmitChannel.h"


TransmitChannel::TransmitChannel()
{
	//Ĭ�ϵ�����
	m_strName = "δ����";
}


TransmitChannel::~TransmitChannel()
{
	//�ͷ���Դ
	for (unsigned i = 0; i < m_vecPoint.size(); i++)
	{
		delete m_vecPoint.at(i);
		m_vecPoint.at(i) = NULL;
	}
	m_vecPoint.clear();
}

void TransmitChannel::AddPoint(TransmitPoint* pPoint)
{
	m_vecPoint.push_back(pPoint);
}

void TransmitChannel::AddChannelParam(string strName, string strValue)
{
	m_ParamChannel[strName] = strValue;
}

void TransmitChannel::operator = (TransmitChannel &channel)
{
	this->m_ParamChannel = channel.m_ParamChannel;
	this->m_strConfigFileFullPath = channel.m_strConfigFileFullPath;
	this->m_strDllFile = channel.m_strDllFile;
	this->m_strName = channel.m_strName;
	this->m_strType = channel.m_strType;
	//��ɾ�����еĵ�
	for (unsigned i = 0; i < m_vecPoint.size(); i++)
	{
		TransmitPoint *pPoint = m_vecPoint.at(i);
		delete pPoint;
	}
	m_vecPoint.clear();
	//�������еĵ�
	for (unsigned i = 0; i < channel.m_vecPoint.size(); i++)
	{
		TransmitPoint *sourcePoint = channel.m_vecPoint.at(i);
		TransmitPoint *destPoint = new TransmitPoint;
		*destPoint = *sourcePoint;

		this->m_vecPoint.push_back(destPoint);
	}
}

vector<TransmitPoint*> TransmitChannel::GetVecPoint()
{
	return m_vecPoint;
}