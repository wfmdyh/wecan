#pragma once
class COL_POINT
{
public:
	string astrName;
	//点参数集合
	POINT_PARAM mapParam;
	VARIANT *varData;

	//值是否变化
	bool IsChange;
	//运算符重载
	void operator=(COL_POINT &cp);
public:
	COL_POINT();
};

