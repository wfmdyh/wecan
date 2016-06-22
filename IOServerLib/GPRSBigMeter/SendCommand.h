/*
指令下发类
*/
#pragma once

class DevCommand;
class SendCommand
{
private:
	map<DWORD, DevCommand> m_CommandOrder;				//指令队列
	CRITICAL_SECTION m_csCommand;					//线程互斥
public:
	enum DEVICE_TYPE
	{
		TYPE_BM,		//大表
		TYPE_CON		//集中器
	};
public:
	SendCommand();
	~SendCommand();
	//创建指令队列
	BOOL CreateOrder();
	//获取对应ID的设备指令，找到返回TRUE，指令保存在cmd中。如果没有找到下发指令，默认返回不带参数的指令
	BOOL GetOrderById(DWORD dwID, DevCommand &cmd, DEVICE_TYPE devType);
};

