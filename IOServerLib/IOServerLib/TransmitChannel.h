#pragma once
#include "TransmitPoint.h"
class TransmitChannel
{
public:
	string m_strName;
	string m_strDllFile;
	string m_strConfigFileFullPath;
	string m_strType;					//链路的传输类型opc com tcp
private:
	vector<TransmitPoint*> m_vecPoint;
	//通道参数
	map<string, string> m_ParamChannel;
public:
	TransmitChannel();
	~TransmitChannel();

	void AddPoint(TransmitPoint* pPoint);
	void AddChannelParam(string strName, string strValue);
	vector<TransmitPoint*> GetVecPoint();
	//运算符重载
	void operator=(TransmitChannel &channel);
};

