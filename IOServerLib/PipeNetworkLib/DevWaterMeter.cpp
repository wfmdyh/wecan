#include "stdafx.h"
#include "DevWaterMeter.h"


DevWaterMeter::DevWaterMeter()
{
	m_State = 0;				//ˮ��״̬
	m_TotalFlow = 0;			//�ۼ�����
	m_HeatQuantity = 0;			//����
	m_Flow = 0;					//˲ʱ����
	m_Pressure = 0;				//ѹ��
	m_Temperature = 0;			//�¶�
	m_RFState = 0;				//RF״̬
}


DevWaterMeter::~DevWaterMeter()
{
}
