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
	//���������
	void operator=(COL_POINT &cp);
public:
	COL_POINT();
	~COL_POINT();
};

