
// IOSUI.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "IOSUI.h"
#include "IOSUIDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIOSUIApp

BEGIN_MESSAGE_MAP(CIOSUIApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CIOSUIApp ����

CIOSUIApp::CIOSUIApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	m_hIOServerMod = NULL;
}


// Ψһ��һ�� CIOSUIApp ����

CIOSUIApp theApp;


// CIOSUIApp ��ʼ��

BOOL CIOSUIApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	//��ʼ��COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr != S_OK)
	{
		MessageBox(NULL, L"��ʼ��CoInitializeExʧ��", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	//
	if (!LoadIOSLibrary())
	{
		return FALSE;
	}
	CIOSUIDlg dlg;
	if (!dlg.InitIOServerLib(m_hIOServerMod))
	{
		MessageBox(NULL, L"��ʼ��IOServer��̬��ʧ��", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}
	
	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	CoUninitialize();
	//ж��IOServer��̬��
	FreeLibrary(m_hIOServerMod);
	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	//_CrtDumpMemoryLeaks();
	return FALSE;
}

//����IOServer��̬��
BOOL CIOSUIApp::LoadIOSLibrary()
{
	m_hIOServerMod = LoadLibrary(L"IOServerLib.dll");
	if (m_hIOServerMod == NULL)
	{
		MessageBox(NULL, L"����IOServer��̬��ʧ��", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

