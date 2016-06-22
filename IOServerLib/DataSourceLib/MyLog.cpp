#include "stdafx.h"
#include "MyLog.h"


MyLog::MyLog()
{
	m_strPath = L"C:\\IOServer日志";
	m_strPrefix = L"log";
}


MyLog::~MyLog()
{
}

void MyLog::SetFileName(wstring strPrefix)
{
	m_strPrefix = strPrefix;
}

//写日志
bool MyLog::LogToFile(char *pData, DWORD dwLen)
{
	//最大长度
	const int MAXLEN = 1024;
	if (dwLen > MAXLEN)
	{
		return false;
	}
	wstring strFullPath = m_strPath;
	//自动创建目录
	_wmkdir(m_strPath.c_str());

	SYSTEMTIME st;
	GetLocalTime(&st);
	wchar_t wcsTime[MAX_PATH - 100] = { 0 };
	swprintf(wcsTime, L"%s%d_%02d_%02d.data", m_strPrefix.c_str(), st.wYear, st.wMonth, st.wDay);
	strFullPath += L"\\";
	strFullPath += wcsTime;
	//追加只写
	FILE *fe = _wfopen(strFullPath.c_str(), L"ab");
	if (fe == NULL)
	{
		//DebugPrintf("打开文件失败：%s\n", strFullPath.c_str());
		return false;
	}
	//写入时间
	char szTime[20] = { 0 };
	sprintf(szTime, "\r\n%02d:%02d:%02d:", st.wHour, st.wMinute, st.wSecond);
	fwrite(szTime, 1, strlen(szTime), fe);
	dwLen = strlen(pData);
	if (fwrite(pData, dwLen, 1, fe) < 1)
	{
		//DebugPrintf("写入文件失败：%s\n", strFilePath.c_str());
		fclose(fe);
		return false;
	}

	fclose(fe);

	return true;
}

wchar_t* MyLog::GetWinErr(DWORD dwError)
{
	wchar_t *lpBuf = NULL, *lpDestBuf = NULL;
	DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwError, systemLocale, (LPWSTR)&lpBuf, 0, NULL);
	PTSTR hLock = (PTSTR)LocalLock(lpBuf);
	//分配内存大小
	unsigned unSize = wcslen(lpBuf) * 2;
	unSize += 2;	//字符串结束符
	lpDestBuf = (wchar_t*)malloc(unSize);
	memset(lpDestBuf, 0, unSize);
	memcpy(lpDestBuf, hLock, unSize - 2);
	LocalFree(lpBuf);

	return lpDestBuf;
}

#pragma unmanaged
bool MyLog::LogMsgToFile(const wchar_t* wszMsg, ...)
{
	TCHAR szBuffer[512] = { 0 };

	va_list args;
	va_start(args, wszMsg);

	int nBuf = _vsnwprintf_s(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]), wszMsg, args);
	if (nBuf >= 0)
	{
		wstring strFilePath = m_strPath + L"//";
		//判断目录是否存在
		if (_waccess(strFilePath.c_str(), 0) == -1)
		{
			//目录不存在，创建目录
			if (_wmkdir(strFilePath.c_str()) == -1)
			{
				OutputDebugString(L"无法创建根目录\n");
				return false;
			}
		}

		//文件名
		FILE *fe = NULL;
		SYSTEMTIME st;
		GetLocalTime(&st);
		wchar_t wszTime[20] = { 0 };
		swprintf_s(wszTime, L"%s_%d_%02d_%02d.log", m_strPrefix.c_str(), st.wYear, st.wMonth, st.wDay);
		strFilePath += wszTime;

		//判断文件是否存在，如果第一次，则添加unicode标识
		if (_waccess(strFilePath.c_str(), 0) == -1)
		{
			_wfopen_s(&fe, strFilePath.c_str(), L"ab");
			if (fe == NULL)
			{
				OutputDebugString(L"无法创建日志文件\n");
				return false;
			}
			USHORT usUnicode = 0xfeff;
			fwrite(&usUnicode, 2, 1, fe);
			fclose(fe);
		}
		fe = NULL;
		//追加只写
		_wfopen_s(&fe, strFilePath.c_str(), L"ab");
		if (fe == NULL)
		{
			OutputDebugString(L"无法打开日志文件\n");
			return false;
		}
		//文件内容
		//写入时间
		wchar_t wszWriteData[1024] = { 0 };
		swprintf_s(wszWriteData, L"\r\n%02d:%02d:%02d:%s", st.wHour, st.wMinute, st.wSecond, szBuffer);
		if (fwrite(wszWriteData, sizeof(wszWriteData[0]), wcslen(wszWriteData), fe) < 1)
		{
			OutputDebugString(L"无法写入日志文件\n");
			//写入文件失败
			fclose(fe);
			return false;
		}

		fclose(fe);
	}

	va_end(args);
	return true;
}

#pragma managed