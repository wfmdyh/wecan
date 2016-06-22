#pragma once
class MyLog
{
private:
	//文件路径
	wstring m_strPath;
	//前缀
	wstring m_strPrefix;
public:
	MyLog();
	~MyLog();
	//设置日志文件的前缀名称
	void SetFileName(wstring strPrefix);
	//写日志 最大1024个字符
	//bool LogToFile(char *pData, DWORD dwLen);
	bool LogMsgToFile(const wchar_t* wszMsg, ...);
	//获取windows的错误文本，调用free()函数释放资源
	//wchar_t* GetWinErr(DWORD dwError);
};

