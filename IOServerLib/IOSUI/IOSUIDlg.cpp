
// IOSUIDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "IOSUI.h"
#include "IOSUIDlg.h"
#include "afxdialogex.h"

CIOSUIDlg* g_pDlg = NULL;
BOOL g_bStop = TRUE;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIOSUIDlg �Ի���



CIOSUIDlg::CIOSUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIOSUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//��ʼ������ָ��
	IOSStart = NULL;
	IOSStop = NULL;
	OPCReg = NULL;
	OPCUnReg = NULL;
	IOS_GetDataArrByChannel = NULL;
	IOS_RegColReadDataFun = NULL;
	RegShowMessage = NULL;

	
}

void CIOSUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGES, m_MessageList);
}

BEGIN_MESSAGE_MAP(CIOSUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CIOSUIDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_SELECT_PRO, &CIOSUIDlg::OnBnClickedSelectPro)
	ON_BN_CLICKED(IDC_STOP, &CIOSUIDlg::OnBnClickedStop)
	ON_WM_CLOSE()
	ON_MESSAGE(UM_STOP, &CIOSUIDlg::OnStoped)
	ON_COMMAND(IDM_WATCH, &CIOSUIDlg::OnWatch)
END_MESSAGE_MAP()


// CIOSUIDlg ��Ϣ�������

BOOL CIOSUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	//���ԭ�з��
	DWORD dwStyle = ::GetWindowLong(m_MessageList.m_hWnd, GWL_STYLE);
	dwStyle &= ~(LVS_TYPEMASK);
	dwStyle &= ~(LVS_EDITLABELS);
	//�����·��
	SetWindowLong(m_MessageList.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS);
	//������չ���
	DWORD styles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyleEx(m_MessageList.m_hWnd, styles, styles);

	m_MessageList.InsertColumn(0, L"ʱ��");
	m_MessageList.InsertColumn(1, L"��Ϣ");
	m_MessageList.SetColumnWidth(0, 150);//�����п�  
	m_MessageList.SetColumnWidth(1, 350);
	g_pDlg = (CIOSUIDlg*)AfxGetMainWnd();
	//��ʼ���˳���־
	m_Exit = FALSE;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void __stdcall ShowLog(LPCTSTR pstrFormat)
{
	g_pDlg->ShowMessage(pstrFormat);
}

//��ʼ����̬����������ݣ�����ָ��
BOOL CIOSUIDlg::InitIOServerLib(HMODULE hMod)
{
	if (hMod == NULL)
	{
		return FALSE;
	}
	IOSStart = (fnIOS_Start)GetProcAddress(hMod, "IOS_Start");
	if (!IOSStart)
	{
		return FALSE;
	}
	IOSStop = (fnIOS_Stop)GetProcAddress(hMod, "IOS_Stop");
	if (!IOSStop)
	{
		return FALSE;
	}
	OPCReg = (fnRegOPCServer)GetProcAddress(hMod, "RegOPCServer");
	if (!OPCReg)
	{
		return FALSE;
	}
	OPCUnReg = (fnUnRegOPCServer)GetProcAddress(hMod, "UnRegOPCServer");
	if (!OPCUnReg)
	{
		return FALSE;
	}
	IOS_GetDataArrByChannel = (fnIOS_GetDataArrByChannel)GetProcAddress(hMod, "IOS_GetDataArrByChannel");
	if (!IOS_GetDataArrByChannel)
	{
		return FALSE;
	}
	IOS_RegColReadDataFun = (fnIOS_RegColReadDataFun)GetProcAddress(hMod, "IOS_RegColReadDataFun");
	if (!IOS_GetDataArrByChannel)
	{
		return FALSE;
	}
	RegShowMessage = (fnRegShowMessage)GetProcAddress(hMod, "RegShowMessage");
	if (!RegShowMessage)
	{
		return FALSE;
	}
	(*RegShowMessage)(ShowLog);
	//ע��ص�����
	//(*IOS_RegColReadDataFun)(ReadColData);

	return TRUE;
}

void CIOSUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CIOSUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CIOSUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIOSUIDlg::OnBnClickedStart()
{
	//������ʱ
	Sleep(100);
	if (!g_bStop)
	{
		ShowMessage(L"�����Ѿ�����");
		return;
	}
	
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (!ProjectFileExist(m_strProjectPath))
	{
		return;
	}
	wstring strPath = m_strProjectPath + L"\\";
	ShowMessage(L"�������������Ե�...");
	if (!(*IOSStart)((wchar_t*)strPath.c_str()))
	{
		ShowMessage(L"�޷�����IOServer");
		return;
	}
	g_bStop = FALSE;
	ShowMessage(L"�ɹ�����");
}

//ѡ����Ŀ·��
void CIOSUIDlg::OnBnClickedSelectPro()
{
	WCHAR wszPath[MAX_PATH + 1] = { 0 };
	LPITEMIDLIST lpItemList = NULL;
	BROWSEINFO bBinfo;
	memset(&bBinfo, 0, sizeof(BROWSEINFO));
	bBinfo.hwndOwner = m_hWnd;
	bBinfo.lpszTitle = _TEXT("��ѡ��·��:");
	bBinfo.ulFlags = BIF_RETURNONLYFSDIRS;

	lpItemList = SHBrowseForFolder(&bBinfo);
	if (lpItemList != NULL)
	{
		SHGetPathFromIDList(lpItemList, wszPath);
	}
	if (ProjectFileExist(wszPath))
	{
		m_strProjectPath = wszPath;
		SetDlgItemText(IDC_PRO_PATH, wszPath);
	}
}

//�����Ƿ����ioserver.xml�ļ�
BOOL CIOSUIDlg::ProjectFileExist(wstring strPath)
{
	BOOL bResult = TRUE;
	wstring strFullPath = strPath;
	strFullPath += L"\\ioserver.xml";
	wifstream projectFile(strFullPath);
	if (!projectFile.is_open())
	{
		ShowMessage(L"û���ҵ���Ŀ�ļ���������ѡ��");
		bResult = FALSE;
	}
	else{
		projectFile.close();
	}
	return bResult;
}



//�ڽ����ϴ�ӡ��Ϣ
void CIOSUIDlg::ShowMessage(LPCTSTR pstrFormat, ...)
{
	CString str, strTime;
	// format and write the data you were given
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);
	va_end(args);
	//ʱ��
	SYSTEMTIME st;
	GetLocalTime(&st);
	strTime.Format(L"%d-%d-%d %d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	int itemIndex = m_MessageList.InsertItem(0, strTime);
	m_MessageList.SetItemText(itemIndex, 1, str);
}



DWORD WINAPI ThreadClose(LPVOID lpThreadParameter)
{
	CIOSUIDlg* pDlg = (CIOSUIDlg*)lpThreadParameter;
	(*(pDlg->IOSStop))();
	g_bStop = TRUE;
	if (pDlg->m_Exit)
	{
		::PostMessage(pDlg->GetSafeHwnd(), UM_STOP, 0, 0);
	}
	return 0;
}

void CIOSUIDlg::OnBnClickedStop()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	ShowMessage(L"����ֹͣ�������Ե�...");
	if (g_bStop && !m_Exit)
	{
		ShowMessage(L"����û������");
		return;
	}
	//�����˳��߳�
	m_hClose = CreateThread(NULL, 0, ThreadClose, this, 0, NULL);
	
}


void CIOSUIDlg::OnClose()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (IDYES == AfxMessageBox(L"�Ƿ�Ҫ�˳�����", MB_YESNO))
	{
		m_Exit = TRUE;
		OnBnClickedStop();
		
	}
}

LRESULT CIOSUIDlg::OnStoped(WPARAM wParam, LPARAM lParam)
{
	ShowMessage(L"ֹͣ��Ϣ���%d", wParam);
	Sleep(100);
	//CDialogEx::OnClose();
	::PostQuitMessage(0);

	return 0;
}

void CIOSUIDlg::OnWatch()
{
	// TODO:  �ڴ���������������
	

}
