#pragma once
class TransmitPoint
{
public:
	string m_strName;
	string m_strDataType;
private:
	VARIANT m_varValue;
	//�ɼ�ͨ�����±�
	int m_colChannel;
	int m_colDevice;
	int m_colPoint;
	//�����
	map<string, string> m_ParamPoint;
public:
	TransmitPoint();
	~TransmitPoint();
	void AddPointParam(string strName, string strValue);
	VARIANT GetVarValue();
	string GetValue();
	void SetValue(VARIANT var);
	//���òɼ�ͨ���±�
	void SetColIndex(int nChannel, int nDevice, int nPoint);
	int GetColChannel();
	int GetColDevice();
	int GetColPoint();
	//��ȡ����
	map<string, string> GetParams();

	//��һ��VARIANTӳ����ַ�����ֵ
	BOOL MapVariantToStringVal(VARIANT vtVal, wstring &strValue);
	

};

