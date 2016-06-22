#pragma once
class DevPipeNet;

class DeviceConnect
{
private:
	char* m_pRecDataBuf;			//接收到的数据缓存队列
	int m_iDataRear;				//数据的结束位置
	const int DATA_BUFFER_SIZE;
	
public:
	//TCPModel *m_Parent;
	DevPipeNet *m_Device;			//解析完毕的数据
	DWORD m_DeviceID;				//当前发送数据的设备ID
public:
	DeviceConnect();
	~DeviceConnect();
	
	//将客户端收到的数据拷贝到数据队列中，返回值，1帧接收完整；2帧不完整，继续接收；3错误的帧
	int CopyDataToBuf(char *pData, DWORD dwDataLen);
	void TryAnalysis();
	wstring GetDeviceIDStr();
private:
	//舍弃数据
	void RemoveFrontData(int iNum);
	void AnalyzeData(BYTE *szBuf, WORD wPackageLen);

	//2字节转换
	WORD ConvertTwoBytes(BYTE *pData);
	//4字节转换
	DWORD ConvertFourBytes(BYTE *pData);
	//8字节转double
	double ConvertEightByteToDouble(BYTE *pby);

	//验证帧是否正确
	int CheckPackage();
};

