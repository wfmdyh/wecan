/*
�����ṹ��
ʱ�䣺2015-12-08
*/
#pragma once
class CIOSDriver
{
public:
	enum IOSDTYPE
	{
		TYPE_COL,		//�ɼ�ͨ��
		TYPE_TRA		//ת��ͨ��
	};
private:
	string m_strName;			//�������ƣ���̬��ȫ·��
	HMODULE m_hMod;				//�������
	int m_nType;				//��������
public:
	CIOSDriver();
	~CIOSDriver();

	string GetName();
	HMODULE GetHandle();
	void SetName(string strName);
	void SetHandle(HMODULE hMod);
	void SetType(IOSDTYPE type);
	int GetType();
};

