#pragma once
class OPCTag
{
public:
	string m_strName;
	VARIANT m_varValue;
	WORD m_wQuality;
	BOOL m_bIsWritable;
	HANDLE m_hTag;			//��ǩ���
public:
	OPCTag();
	~OPCTag();
};

