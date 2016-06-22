#pragma once
class TransmitPoint
{
public:
	string m_strName;
	string m_strDataType;
private:
	VARIANT m_varValue;
	//采集通道的下标
	int m_colChannel;
	int m_colDevice;
	int m_colPoint;
	//点参数
	map<string, string> m_ParamPoint;
public:
	TransmitPoint();
	~TransmitPoint();
	void AddPointParam(string strName, string strValue);
	VARIANT GetVarValue();
	string GetValue();
	void SetValue(VARIANT var);
	//设置采集通道下标
	void SetColIndex(int nChannel, int nDevice, int nPoint);
	int GetColChannel();
	int GetColDevice();
	int GetColPoint();
	//获取参数
	map<string, string> GetParams();

	//将一个VARIANT映射成字符串的值
	BOOL MapVariantToStringVal(VARIANT vtVal, wstring &strValue);
	

};

