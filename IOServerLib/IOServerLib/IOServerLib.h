#ifndef _IOSERVER_LIB_H
#define _IOSERVER_LIB_H
#include "channel.h"
#include "TransmitManager.h"
//�ص�����������dv��ֵ���ַ�����ʽ������
typedef void(__stdcall *PReadColData)(wchar_t **dv,int iCount);
//-----------------------------------��������--------------------

/*
 ����ͨ�������豸��������������ֵ��ָ��
 ������val�����ظ�������
 ����ֵ���ҵ�����TRUE��û�ҵ�����FALSE
*/
BOOL IOS_GetPointValue(string strChannel,string strDevice,string strPoint,VARIANT *val);

//��ͨ��������ʱ�Ļص����� pChannels���ɼ���ɵ�ͨ�����ݣ����߳�
void DataArrive(CHANNEL_INFO *pChannel);
//�ص����� ������pTraManager����ֵ��ϵ�ת������
bool UpdataTransmit(TransmitManager *pTraManager);

#endif
