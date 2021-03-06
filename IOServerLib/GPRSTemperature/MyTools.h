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

void ErrMsg(wchar_t *strMsg,...);

BOOL MapVariantToString(VARIANT vtVal,wchar_t *strValue);

//调试输出
void DebugPrintf(char *szMsg,...);