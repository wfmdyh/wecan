
// IOSUIDlg.cpp : 实现文件
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


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CIOSUIDlg 对话框



CIOSUIDlg::CIOSUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIOSUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//初始化函数指针
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


// CIOSUIDlg 消息处理程序

BOOL CIOSUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	//获得原有风格
	DWORD dwStyle = ::GetWindowLong(m_MessageList.m_hWnd, GWL_STYLE);
	dwStyle &= ~(LVS_TYPEMASK);
	dwStyle &= ~(LVS_EDITLABELS);
	//设置新风格
	SetWindowLong(m_MessageList.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS);
	//设置扩展风格
	DWORD styles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyleEx(m_MessageList.m_hWnd, styles, styles);

	m_MessageList.InsertColumn(0, L"时间");
	m_MessageList.InsertColumn(1, L"消息");
	m_MessageList.SetColumnWidth(0, 150);//设置列宽  
	m_MessageList.SetColumnWidth(1, 350);
	g_pDlg = (CIOSUIDlg*)AfxGetMainWnd();
	//初始化退出标志
	m_Exit = FALSE;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void __stdcall ShowLog(LPCTSTR pstrFormat)
{
	g_pDlg->ShowMessage(pstrFormat);
}

//初始化动态库里面的内容，函数指针
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
	//注册回调函数
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIOSUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CIOSUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIOSUIDlg::OnBnClickedStart()
{
	//稍作延时
	Sleep(100);
	if (!g_bStop)
	{
		ShowMessage(L"程序已经运行");
		return;
	}
	
	// TODO:  在此添加控件通知处理程序代码
	if (!ProjectFileExist(m_strProjectPath))
	{
		return;
	}
	wstring strPath = m_strProjectPath + L"\\";
	ShowMessage(L"正在启动，请稍等...");
	if (!(*IOSStart)((wchar_t*)strPath.c_str()))
	{
		ShowMessage(L"无法启动IOServer");
		return;
	}
	g_bStop = FALSE;
	ShowMessage(L"成功启动");
}

//选择项目路径
void CIOSUIDlg::OnBnClickedSelectPro()
{
	WCHAR wszPath[MAX_PATH + 1] = { 0 };
	LPITEMIDLIST lpItemList = NULL;
	BROWSEINFO bBinfo;
	memset(&bBinfo, 0, sizeof(BROWSEINFO));
	bBinfo.hwndOwner = m_hWnd;
	bBinfo.lpszTitle = _TEXT("请选择路径:");
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

//查找是否存在ioserver.xml文件
BOOL CIOSUIDlg::ProjectFileExist(wstring strPath)
{
	BOOL bResult = TRUE;
	wstring strFullPath = strPath;
	strFullPath += L"\\ioserver.xml";
	wifstream projectFile(strFullPath);
	if (!projectFile.is_open())
	{
		ShowMessage(L"没有找到项目文件，请重新选择");
		bResult = FALSE;
	}
	else{
		projectFile.close();
	}
	return bResult;
}



//在界面上打印消息
void CIOSUIDlg::ShowMessage(LPCTSTR pstrFormat, ...)
{
	CString str, strTime;
	// format and write the data you were given
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);
	va_end(args);
	//时间
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
	// TODO:  在此添加控件通知处理程序代码
	ShowMessage(L"正在停止程序，请稍等...");
	if (g_bStop && !m_Exit)
	{
		ShowMessage(L"程序没有运行");
		return;
	}
	//创建退出线程
	m_hClose = CreateThread(NULL, 0, ThreadClose, this, 0, NULL);
	
}


void CIOSUIDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (IDYES == AfxMessageBox(L"是否要退出程序？", MB_YESNO))
	{
		m_Exit = TRUE;
		OnBnClickedStop();
		
	}
}

LRESULT CIOSUIDlg::OnStoped(WPARAM wParam, LPARAM lParam)
{
	ShowMessage(L"停止消息到达：%d", wParam);
	Sleep(100);
	//CDialogEx::OnClose();
	::PostQuitMessage(0);

	return 0;
}

void CIOSUIDlg::OnWatch()
{
	// TODO:  在此添加命令处理程序代码
	

}
