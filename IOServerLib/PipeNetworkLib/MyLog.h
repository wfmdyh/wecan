#pragma once
class MyLog
{
private:
	//�ļ�·��
	wstring m_strPath;
	//ǰ׺
	wstring m_strPrefix;
public:
	MyLog();
	~MyLog();
	//������־�ļ���ǰ׺����
	void SetFileName(wstring strPrefix);
	//д��־ ���1024���ַ�
	bool LogToFile(char *pData, DWORD dwLen);
	//bool LogMsgToFile(const wchar_t* wszMsg, ...);
	//��ȡwindows�Ĵ����ı�������free()�����ͷ���Դ
	wchar_t* GetWinErr(DWORD dwError);
};

