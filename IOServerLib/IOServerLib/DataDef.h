/************************************************************************
 ���ݽṹ����                                                                     
************************************************************************/
#ifndef _DATADEF_H
#define _DATADEF_H




//ͨ������ 1�ɼ� or 2ת��
#define DRIVER_TYPE_COL 1
#define DRIVER_TYPE_TRA 2

//ͨ�������ݴ�������
#define CTYPE_OPCCLIENT "OPCClient"
#define CTYPE_OPCSERVER "OPCServer"
#define CTYPE_TCP "TCP"
#define CTYPE_COM "COM"

//�����־
typedef void(__stdcall *fnShowLog)(const wchar_t* pstrFormat);
extern fnShowLog g_lpfnShowLog;
void ShowMessage(const wchar_t* wcsMsg, ...);

#endif
