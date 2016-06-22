/************************************************************************
 数据结构定义                                                                     
************************************************************************/
#ifndef _DATADEF_H
#define _DATADEF_H




//通道类型 1采集 or 2转发
#define DRIVER_TYPE_COL 1
#define DRIVER_TYPE_TRA 2

//通道的数据传输类型
#define CTYPE_OPCCLIENT "OPCClient"
#define CTYPE_OPCSERVER "OPCServer"
#define CTYPE_TCP "TCP"
#define CTYPE_COM "COM"

//输出日志
typedef void(__stdcall *fnShowLog)(const wchar_t* pstrFormat);
extern fnShowLog g_lpfnShowLog;
void ShowMessage(const wchar_t* wcsMsg, ...);

#endif
