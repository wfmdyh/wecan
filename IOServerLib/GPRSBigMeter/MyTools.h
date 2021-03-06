/************************************************************************
 作者：wfm
 日期：2015.1.12
 描述：常用工具类
************************************************************************/
#pragma once

string UnicodeToANSI(const wstring& str );
wstring ANSIToUnicode( const string& str );

void chUnicodeToANSI(const wchar_t* wsz,char *sz);
void chANSIToUnicode(const char *str,wchar_t *wstr);

//格式化字符串到wstring中
wstring wstr_format(const wchar_t *fmt,...);
string str_format(const char *fmt,...);

BOOL MapVariantToString(VARIANT vtVal,wchar_t *strValue);

//将一个整数转换成16进制字符
string ByteToHex(unsigned char n);

//将数据块转换成16进制字符串
string ToHexStr(char *pData, unsigned nLen);

//当前时间的字符串形式 
wstring NowTimeToString(BOOL bTime, wstring dateFlag, wstring timeFlag);