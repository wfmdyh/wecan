#ifndef _DATADEF_H
#define _DATADEF_H

class CHANNEL_INFO;
typedef void(*fnDataArrive)(CHANNEL_INFO *ColData);
// ‰≥ˆ»’÷æ
typedef void(*fnShowMessage)(const wchar_t* wcsMsg);
void ShowMessage(const wchar_t* wcsMsg);

#endif
