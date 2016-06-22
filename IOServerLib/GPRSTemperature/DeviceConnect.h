/*
设备连接对象
*/
#pragma once
#include "DataDef.h"
#include "IOCPModel.h"

class DeviceConnect
{
private:
	char* m_pRecDataBuf;			//接收到的数据缓存队列
	int m_iDataRear;				//数据的结束位置
	CGPRSTemperature *m_Device;		//解析完毕的数据
public:
	//fnGetOneDevice m_GetOneDevice;			//数据采集完毕时调用的回调函数
	CIOCPModel *m_Parent;					//通道
public:
	DeviceConnect();
	~DeviceConnect();
	/*
	将客户端收到的数据拷贝到数据队列中
	*/
	void CopyDataToBuf(char *pData, DWORD dwDataLen);
private:
	//尝试解析
	void TryAnalysis();
	//舍弃数据
	void RemoveFrontData(int iNum);
	BOOL AnalyzeData(BYTE *szBuf);

	//错误日志文件
	void WriteErrToFile(char *pData, DWORD dwLen);

	string TempID;

};

