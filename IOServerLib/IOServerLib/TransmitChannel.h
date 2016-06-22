#pragma once
#include "TransmitPoint.h"
class TransmitChannel
{
public:
	string m_strName;
	string m_strDllFile;
	string m_strConfigFileFullPath;
	string m_strType;					//��·�Ĵ�������opc com tcp
private:
	vector<TransmitPoint*> m_vecPoint;
	//ͨ������
	map<string, string> m_ParamChannel;
public:
	TransmitChannel();
	~TransmitChannel();

	void AddPoint(TransmitPoint* pPoint);
	void AddChannelParam(string strName, string strValue);
	vector<TransmitPoint*> GetVecPoint();
	//���������
	void operator=(TransmitChannel &channel);
};

