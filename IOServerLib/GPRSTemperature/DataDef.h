#ifndef _DATADEF_H
#define _DATADEF_H
#include "channel.h"
#include <vector>
#include "GPRSTemperature.h"

//���ݵ���Ļص����� ColData�ɼ������������ݣ�ͨ���ص���������
typedef void(*fnDataArrive)(CHANNEL_INFO *ColData);
//�ɹ�������һ���豸�����ݵĻص�������pDevice������ϵ�һ���豸���ݣ�ͨ���ص���������
//typedef void(*fnGetOneDevice)(CIOCPModel *pParent, CGPRSTemperature *pDevice);

#endif