
// IOSUIDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"

//////////////////////////////////动态库函数定义
typedef int(*fnIOS_Start)(wchar_t *wszProjectPath);
typedef void(*fnIOS_Stop)();
//OPC Server注册
typedef BOOL(*fnRegOPCServer)(LPCTSTR wszAppExePath);
typedef BOOL(*fnUnRegOPCServer)();

typedef void(*fndbtest)();

//获取数据
typedef BOOL(*fnIOS_GetDataArrByChannel)(wchar_t *wszChannelName);

typedef void(__stdcall *PReadColData)(wchar_t **dv, int iCount);
typedef void(*fnIOS_RegColReadDataFun)(PReadColData fnReadColData);
//日志
typedef void(__stdcall *fnShowLog)(const wchar_t* pstrFormat);
typedef void(*fnRegShowMessage)(fnShowLog lpfnShowLog);
/////////////////////////////////

// CIOSUIDlg 对话框
class CIOSUIDlg : public CDialogEx
{
// 构造
public:
	CIOSUIDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IOSUI_DIALOG };
public:
	CListCtrl m_MessageList;
	wstring m_strProjectPath;					//项目路径
	HANDLE m_hClose;							//停止运行的线程句柄
	BOOL m_Exit;								//是否退出程序标志

	fnIOS_Start IOSStart;
	//退出函数
	fnIOS_Stop IOSStop;

	fnRegOPCServer OPCReg;
	fnUnRegOPCServer OPCUnReg;

	fnIOS_GetDataArrByChannel IOS_GetDataArrByChannel;
	fnIOS_RegColReadDataFun IOS_RegColReadDataFun;
	fnRegShowMessage RegShowMessage;
public:
	//在界面上打印消息
	void ShowMessage(LPCTSTR pstrFormat, ...);
	//初始化动态库里面的内容，函数指针
	BOOL InitIOServerLib(HMODULE hMod);
	//查找是否存在ioserver.xml文件
	BOOL ProjectFileExist(wstring strPath);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedSelectPro();
	afx_msg void OnBnClickedStop();
	afx_msg void OnClose();
	afx_msg LRESULT OnStoped(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWatch();
};
