#pragma once
class COL_POINT
{
public:
	string astrName;
	//���������
	map<string,string> mapParam;
	VARIANT *varData;

	//ֵ�Ƿ�仯
	bool m_IsChange;
	struct tm m_CollectTime;				//���ݲɼ���ʱ��
	//���������
	void operator=(COL_POINT &cp);
public:
	COL_POINT();
	~COL_POINT();
	//��ȡֵ���ַ�����ʽ
	wstring GetValue();
};

