/*
���ݹ�����
wfm
2016-4-14
*/
#pragma once
class DataManager
{
private:
	vector<CHANNEL_INFO> m_Channels;
	CRITICAL_SECTION m_LockChannel;
public:
	DataManager();
	~DataManager();
	//���һ��ͨ������
	BOOL AddChannel(CHANNEL_INFO channel);
	//��ȡͨ���ļ����˿�
	int GetChannelPort(string strName);

	void Lock();
	void Unlock();
};

