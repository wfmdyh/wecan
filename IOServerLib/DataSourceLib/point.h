#pragma once
class COL_POINT
{
public:
	string astrName;
	//���������
	POINT_PARAM mapParam;
	VARIANT *varData;

	//ֵ�Ƿ�仯
	bool IsChange;
	//���������
	void operator=(COL_POINT &cp);
public:
	COL_POINT();
};

