
// IOSUIDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"

//////////////////////////////////��̬�⺯������
typedef int(*fnIOS_Start)(wchar_t *wszProjectPath);
typedef void(*fnIOS_Stop)();
//OPC Serverע��
typedef BOOL(*fnRegOPCServer)(LPCTSTR wszAppExePath);
typedef BOOL(*fnUnRegOPCServer)();

typedef void(*fndbtest)();

//��ȡ����
typedef BOOL(*fnIOS_GetDataArrByChannel)(wchar_t *wszChannelName);

typedef void(__stdcall *PReadColData)(wchar_t **dv, int iCount);
typedef void(*fnIOS_RegColReadDataFun)(PReadColData fnReadColData);
//��־
typedef void(__stdcall *fnShowLog)(const wchar_t* pstrFormat);
typedef void(*fnRegShowMessage)(fnShowLog lpfnShowLog);
/////////////////////////////////

// CIOSUIDlg �Ի���
class CIOSUIDlg : public CDialogEx
{
// ����
public:
	CIOSUIDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_IOSUI_DIALOG };
public:
	CListCtrl m_MessageList;
	wstring m_strProjectPath;					//��Ŀ·��
	HANDLE m_hClose;							//ֹͣ���е��߳̾��
	BOOL m_Exit;								//�Ƿ��˳������־

	fnIOS_Start IOSStart;
	//�˳�����
	fnIOS_Stop IOSStop;

	fnRegOPCServer OPCReg;
	fnUnRegOPCServer OPCUnReg;

	fnIOS_GetDataArrByChannel IOS_GetDataArrByChannel;
	fnIOS_RegColReadDataFun IOS_RegColReadDataFun;
	fnRegShowMessage RegShowMessage;
public:
	//�ڽ����ϴ�ӡ��Ϣ
	void ShowMessage(LPCTSTR pstrFormat, ...);
	//��ʼ����̬����������ݣ�����ָ��
	BOOL InitIOServerLib(HMODULE hMod);
	//�����Ƿ����ioserver.xml�ļ�
	BOOL ProjectFileExist(wstring strPath);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
