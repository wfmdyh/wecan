#pragma once
class COL_POINT
{
public:
	string astrName;
	//点参数集合
	map<string,string> mapParam;
	VARIANT *varData;

	//值是否变化
	bool m_IsChange;
	//运算符重载
	void operator=(COL_POINT &cp);
public:
	COL_POINT();
	~COL_POINT();
};

