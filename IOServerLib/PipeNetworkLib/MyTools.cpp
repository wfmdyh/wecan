#include "stdafx.h"
#include "MyTools.h"



MyTools::MyTools():
	BUFFER_SIZE(1024)
{

}

MyTools::~MyTools()
{

}

//string MyTools::str_format(const char *fmt, ...)
//{
//	string strResult = "";
//	if (NULL != fmt)
//	{
//		va_list maker = NULL;
//		va_start(maker, fmt);						//初始化变量参数
//		int iLength = _vscprintf(fmt, maker)+1;		//格式化以后的字符串长度
//		//动态数组
//		vector<char> vecWsz(iLength, L'\0');
//		int iWritten = _vsnprintf_s(&(vecWsz[0]), iLength, BUFFER_SIZE, fmt, maker);
//		if (iWritten > 0)
//		{
//			strResult = &(vecWsz[0]);
//		}
//		va_end(maker);								//重置变量参数
//	}
//	return strResult;
//}

//wstring MyTools::wstr_format(const wchar_t *fmt, ...)
//{
//	
//	wstring strResult = L"";
//	if (NULL != fmt)
//	{
//		va_list maker = NULL;
//		va_start(maker,fmt);						//初始化变量参数
//		int iLength = _vscwprintf(fmt,maker)+1;		//格式化以后的字符串长度
//		//动态数组
//		vector<wchar_t> vecWsz(iLength, L'\0');
//		int iWritten = _vsnwprintf_s(&(vecWsz[0]), iLength, BUFFER_SIZE, fmt, maker);
//		if (iWritten > 0)
//		{
//			strResult = &(vecWsz[0]);
//		}
//		va_end(maker);								//重置变量参数
//	}
//
//
//	return strResult;
//}

string MyTools::UnicodeToANSI(const wstring& str)
{
	int	iTextLen = WideCharToMultiByte( CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL );
	vector<char> vecText(iTextLen, '\0');
	::WideCharToMultiByte( CP_ACP,
		0,
		str.c_str(),
		-1,
		&(vecText[0]),
		iTextLen,
		NULL,
		NULL );
	string strText = &(vecText[0]);

	return strText;
}


wstring MyTools::ANSIToUnicode(const string& str)
{
	int  len = 0;
	len = str.length();
	int  unicodeLen = ::MultiByteToWideChar( CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0 );  
	wchar_t *  pUnicode;  
	pUnicode = new  wchar_t[unicodeLen+1];  
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
	::MultiByteToWideChar( CP_ACP,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen );  
	wstring  rt;  
	rt = ( wchar_t* )pUnicode;
	delete  pUnicode; 

	return  rt;  
}

//
void MyTools::chUnicodeToANSI(const wchar_t* wsz, char *sz)
{
	char*     pElementText;
	int    iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte( CP_ACP,
		0,
		wsz,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset(pElementText,0,sizeof(char) * (iTextLen+1));
	::WideCharToMultiByte( CP_ACP,
		0,
		wsz,
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL );

	strcpy_s(sz,(iTextLen+1),pElementText);
	delete pElementText;
}


void MyTools::chANSIToUnicode(const char *str, wchar_t *wstr)
{
	int unicodeLen = ::MultiByteToWideChar( CP_ACP,
		0,
		str,
		-1,
		NULL,
		0 );  
	wchar_t * pUnicode;  
	pUnicode = new  wchar_t[unicodeLen+1];  
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
	::MultiByteToWideChar(CP_ACP,
		0,
		str,
		-1,
		(LPWSTR)pUnicode,
		unicodeLen );  
	wcscpy_s(wstr, unicodeLen, pUnicode);
	delete  pUnicode;
}

BOOL MyTools::MapVariantToString(VARIANT vtVal, wchar_t *strValue)
{
	if (strValue == NULL)
	{
		return FALSE;
	}
	// Assume success until a problem arises:
	BOOL bSuccess = TRUE;
	wchar_t wszVal[20] = {0};
	wstring strVal = L"";
	// Cast string to proper value:
	switch (vtVal.vt)
	{
	case VT_BOOL:
		swprintf_s(wszVal,L"%hd",vtVal.boolVal);
		break;
	case VT_UI1:
		swprintf_s(wszVal, L"%uc", vtVal.bVal);
		break;
	case VT_I1:
		swprintf_s(wszVal, L"%c", vtVal.cVal);
		break;
	case VT_UI2:
		swprintf_s(wszVal, L"%hu", vtVal.uiVal);
		break;
	case VT_I2:
	case VT_INT:
		swprintf_s(wszVal, L"%d", vtVal.iVal);
		break;
	case VT_UI4:
		swprintf_s(wszVal, L"%lu", vtVal.ulVal);
		break;
	case VT_I4:
		swprintf_s(wszVal, L"%ld", vtVal.lVal);
		break;
	case VT_R4:
		swprintf_s(wszVal, L"%hf", vtVal.fltVal);
		break;
	case VT_R8:
		swprintf_s(wszVal, L"%f", vtVal.dblVal);
		break;
	case VT_BSTR:
		strVal = vtVal.bstrVal;
		//swprintf(wszVal,L"%s",vtVal.bstrVal);
		break;
		// We don't expect array types.  If so, return false:
	case VT_UI1	| VT_ARRAY:
	case VT_I1	| VT_ARRAY:
	case VT_UI2	| VT_ARRAY:
	case VT_I2	| VT_ARRAY:
	case VT_UI4	| VT_ARRAY:
	case VT_I4	| VT_ARRAY:
	case VT_R4	| VT_ARRAY:
	case VT_R8	| VT_ARRAY:
		bSuccess = FALSE;
		break;

	default:
		// Unexpected variant type.  Return false:
		bSuccess = FALSE;
		break;
	}
	if (vtVal.vt == VT_BSTR)
	{
		wcscpy_s(strValue, strVal.size(), strVal.c_str());
	}
	else{
		wcscpy_s(strValue, 20, wszVal);
	}
	
	// Return success:
	return (bSuccess);
}

string MyTools::ByteToHex(unsigned char n)
{
	string s = "";
	if (n == 0)
	{
		s = "00";
		return s;
	}
	while (n != 0)
	{
		if (n % 16 > 9)
		{
			s += n % 16 - 10 + 'A';
		}
		else{
			s += n % 16 + '0';
		}
		n = n / 16;
	}
	//补0
	if (s.size() == 1)
	{
		s += '0';
	}
	//反转
	reverse(s.begin(), s.end());
	return s;
}

string MyTools::ToHexStr(char *pData, unsigned nLen)
{
	string strResult = "";
	for (unsigned i = 0; i < nLen; i++)
	{
		strResult += ByteToHex(*(pData + i));
		strResult += " ";
	}
	return strResult;
}

//当前时间的字符串形式
wstring MyTools::NowTimeToString(BOOL bTime, wstring dateFlag, wstring timeFlag)
{
	wostringstream oss;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	oss << st.wYear << dateFlag << st.wMonth << dateFlag << st.wDay;
	
	if (bTime)
	{
		oss << " " << st.wHour << timeFlag << st.wMinute << timeFlag << st.wSecond;
	}
	strTime = oss.str();
	return strTime;
}