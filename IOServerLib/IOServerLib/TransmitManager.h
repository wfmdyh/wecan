/*
�ɼ�������࣬��������֮����̵߳Ļ���
�ɸĽ�����ת��ͨ���Ͳɼ�ͨ���ֿ�������IOServer��������̣߳�������̬����ֻ���븱�������ٲ���
*/
#pragma once
#include "TransmitChannel.h"
#include "channel.h"
#include "MyLog.h"

class TransmitManager
{
private:
	vector<TransmitChannel*> m_vecChannel;				//תͨ��
	HANDLE m_hOPC;										//opc server���߳̾��
	MyLog m_log;										//��־
public:
	TransmitManager();
	~TransmitManager();
	void AddTransmitChannel(TransmitChannel *pChannel);
	vector<TransmitChannel*> GetVecChannel();
	//��ȡָ��Э�������ͨ����ע��Ҫ�ͷ���Դ
	TransmitManager* GetProtocolChannel(string strProtocolName);
	//ת���������һ���ɼ�ͨ������
	void RelevanceColIndex(vector<CHANNEL_INFO*> vecChannel);
	
	//��ʼ
	bool Start();
	//ֹͣ
	bool Stop();
	//��ӡ����
	void ShowData();
};

