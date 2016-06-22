#pragma once
class DevWaterMeter;
class DevPressureAlarm;
class DevMultipleParam;

class DevPipeNet
{
private:
	SYSTEMTIME m_UploadTime;			//数据到达时间
public:
	wstring m_DeviceID;					//设备ID

	BYTE m_AlarmState;					//当前告警或状态标志
	double m_TotalFlow;					//累计流量
	double m_QuantityOfHeat;			//热量
	WORD m_PAValue;						//实时压力告警 压力值
	BYTE m_PAState;						//实时压力告警 状态
	
	vector<WORD> m_ArrTemperature;		//前一天所有温度 0.1
	vector<WORD> m_ArrPressure;			//前一天所有压力数据
	float m_Flow;						//当前瞬时流量
	float m_Pressure;					//当前压力
	float m_Temperature;				//当前温度
	SYSTEMTIME m_Date;					//前一天的日期
	BYTE m_SucessRate;					//当前通讯成功率
	SYSTEMTIME m_CurrentTime;			//当前日期
	BYTE m_State;						//水表状态
	BYTE m_SmallState;					//小无线状态
	vector<DevWaterMeter*> m_ArrMeter;	//多表数据
	DWORD m_Verify[2];					//校正系数
	vector<DevPressureAlarm*> m_ArrPressureAlarm;	//实时压力告警（变长）
	vector<DevMultipleParam*> m_ArrMultipleParam;	//实时多参数数据
	//测量模块软件版本号
	DWORD m_VerMeasurementID;
	BYTE m_VerMeasurementNum[6];
	//通讯模块版本硬件，软件版本号
	WORD m_VerCommunicationHNum;
	WORD m_VerCommunicationSNum;
public:
	DevPipeNet();
	~DevPipeNet();

	void SetUploadTime(SYSTEMTIME* t);
	SYSTEMTIME GetUploadTime();
	void operator=(DevPipeNet &dev);
};

