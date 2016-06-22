// **************************************************************************
// opctestclient.cpp
//
// Description:
//	Implements the OPC Quick Client application root class.
//
// DISCLAIMER:
//	This programming example is provided "AS IS".  As such Kepware, Inc.
//	makes no claims to the worthiness of the code and does not warranty
//	the code to be error free.  It is provided freely and can be used in
//	your own projects.  If you do find this code useful, place a little
//	marketing plug for Kepware in your code.  While we would love to help
//	every one who is trying to write a great OPC client application, the 
//	uniqueness of every project and the limited number of hours in a day 
//	simply prevents us from doing so.  If you really find yourself in a
//	bind, please contact Kepware's technical support.  We will not be able
//	to assist you with server related problems unless you are using KepServer
//	or KepServerEx.
// **************************************************************************


#include "stdafx.h"
#include "opctestclient.h"
#include "mainwnd.h"
#include "document.h"
#include "eventview.h"

static LPCTSTR lpszPaths = _T("Paths");
static LPCTSTR lpszOpenFile = _T("Open File");


/////////////////////////////////////////////////////////////////////////////
// CKApp
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKApp, CWinApp)
	//{{AFX_MSG_MAP(CKApp)
	ON_COMMAND (ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND (ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	ON_COMMAND (ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND_EX_RANGE (ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnFileOpenMRU)
END_MESSAGE_MAP ()


/////////////////////////////////////////////////////////////////////////////
// CKApp construction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKApp ()
//
// Description:
//	Constructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKApp::CKApp ()
{
}

// **************************************************************************
// ~CKApp ()
//
// Description:
//	Destructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKApp::~CKApp ()
{
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CKApp object
/////////////////////////////////////////////////////////////////////////////

CKApp cApp;


/////////////////////////////////////////////////////////////////////////////
// CKApp initialization
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// InitInstance ()
//
// Description:
//	Override this function to initialize each new instance of the application.
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKApp::InitInstance ()
{
	// Load version information:
	m_strProductName.LoadString (IDS_APP_PRODUCTNAME);
	m_strInternalName.LoadString (IDS_APP_INTERNALNAME);
	m_strCompanyName.LoadString (IDS_APP_COMPANYNAME);
	m_strDescription.LoadString (IDS_APP_DESCRIPTION);
	m_strComments.LoadString (IDS_APP_COMMENTS);
	m_strLegalCopyright.LoadString (IDS_APP_LEGALCOPYRIGHT);
	m_strDefConnectProgID.LoadString (IDS_APP_DEFCONNECTPROGID);

	// Set application name and profile name:
	if (m_pszAppName)
		free ((void *)m_pszAppName);

	m_pszAppName = _tcsdup (m_strProductName);

	// Root registry key we'll use to store information for the app instance:
	SetRegistryKey (m_strCompanyName);

	// Load MRU (Most Recently Used file) list:
	LoadStdProfileSettings ();

	TRACE (_T("Server profile \\%s\\%s\n"), m_strCompanyName, m_strProductName);

	// Initialize COM:
	HRESULT hr = 
		CoInitializeEx (NULL, COINIT_MULTITHREADED);

	m_bComInitialized = SUCCEEDED (hr);

	// If we failed to initialize COM, there isn't much we can do so 
	// return FALSE.  This will prevent application from starting.
	if (!m_bComInitialized)
		return (FALSE);

	// Make sure we get time change notifications when the user modifies
	// regional settings:
	CTimeStamp::GlobalInit ();

	// Register the application's document templates.  Document templates
	// serve as the connection between documents, frame windows and views.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate (
		IDR_MAINFRAME,
		RUNTIME_CLASS (CKDocument),
		RUNTIME_CLASS (CKMainWnd),      // main SDI frame window
		RUNTIME_CLASS (CKEventView));	// We will add views later in CKMainWnd::OnCreateClient()

	AddDocTemplate (pDocTemplate);

	// Parse command line for standard shell commands:
	CCommandLineInfo cmdInfo;
	ParseCommandLine (cmdInfo);

	// Dispatch commands specified on the command line:
	if (!ProcessShellCommand (cmdInfo))
		return (FALSE);

	// If we make it here, everything went OK.  Return TRUE to indicate
	// success:
	return (TRUE);
}

// **************************************************************************
// ExitInstance ()
//
// Description:
//	Override this function to clean up when your application terminates.
//
// Parameters:
//  none
//
// Returns:
//  int - 0 indicates no errors, and values greater than 0 indicate an error.
// **************************************************************************
int CKApp::ExitInstance () 
{
	// Uninitialize COM:
	if (m_bComInitialized)
		CoUninitialize ();

	// Perform default processing:
	return (CWinApp::ExitInstance ());
}

// **************************************************************************
// OnUpdateMRUFile ()
//
// Description:
//	Called to update the Most Recently Used (MRU) file list.
//
// Parameters:
//  CCmdUI		*pCmdUI		Encapsulates user interface data.		
//
// Returns:
//  void
// **************************************************************************
void CKApp::OnUpdateMRUFile (CCmdUI *pCmdUI)
{
	// If no MRU files, disable MRU menu items,
	if (m_pRecentFileList == NULL)
		pCmdUI->Enable (FALSE);

	// else update menu to show MRU's
	else
		m_pRecentFileList->UpdateMenu (pCmdUI);
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Define the CAboutDlg class
// **************************************************************************
class CAboutDlg : public CDialog
{
public:
	CAboutDlg ();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog ();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP ()
};


// **************************************************************************
BEGIN_MESSAGE_MAP (CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CAboutDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CAboutDlg::CAboutDlg () : CDialog (CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

// **************************************************************************
// DoDataExchange ()
//
// Description:
//	This method is called by the framework to exchange and validate dialog data.
//
// Parameters:
//  CDataExchange	*pDX	A pointer to a CDataExchange object.
//
// Returns:
//  void
// **************************************************************************
void CAboutDlg::DoDataExchange (CDataExchange *pDX)
{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

// **************************************************************************
// OnInitDialog ()
//
// Description:
//	Called immediately before the dialog box is displayed.  Use opportunity
//	to initialize controls.
//
// Parameters:
//  none
//
// Returns:
//	BOOL - Result of base class processing.
// **************************************************************************
BOOL CAboutDlg::OnInitDialog () 
{
	// Get the version information:
	CVersionInfo cVer (AfxGetInstanceHandle ());

	// Set the title with product name:
	CString strTitle;
	strTitle.LoadString (IDS_ABOUT);
	strTitle += _T(' ');
	strTitle += cApp.GetProductName ();
	SetWindowText (strTitle);

	// Display description:
	SetDlgItemText (IDC_DESCRIPTION, cApp.GetDescription ());

	// Display version:
	CString strVersion;
	cVer.Format (strVersion);
	SetDlgItemText (IDC_VERSION, strVersion);

	// Display copyright:
	SetDlgItemText (IDC_COPYRIGHT, cApp.GetLegalCopyright ());

	// Perform default processing and return result:
	return (CDialog::OnInitDialog ());  
}


/////////////////////////////////////////////////////////////////////////////
// CKApp commands
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnAppAbout ()
//
// Description:
//	About menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKApp::OnAppAbout ()
{
	// Create an "About" dialog:
	CAboutDlg aboutDlg;

	// Show as modal dialog.
	aboutDlg.DoModal ();
}

// **************************************************************************
// OnFileOpen ()
//
// Description:
//	File Open menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
afx_msg void CKApp::OnFileOpen ()
{
	CString strFilter;
	CString strFileName;
	CString strExt;
	CString strInitialPath;

	// Assign a set of file filters (*.otc):
	strFilter.LoadString (IDS_OPCCLIENTFILTER);

	// Default extension (otc):
	strExt.LoadString (IDS_DEFAULTEXT);

	// Create a common file open dialog:
	CFileDialog	cOpenDlg (TRUE, strExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, strFilter, AfxGetMainWnd ());

	// Set up the initial path:
	strInitialPath = AfxGetApp ()->GetProfileString (lpszPaths, lpszOpenFile, _T(""));
	cOpenDlg.m_ofn.lpstrInitialDir = strInitialPath;

	// Display the dialog:
	if (cOpenDlg.DoModal () != IDOK)
		return;

	// Save the selected path:
	strInitialPath = cOpenDlg.GetPathName ();
	strInitialPath = strInitialPath.Left (strInitialPath.ReverseFind (_T('\\')));
	AfxGetApp ()->WriteProfileString (lpszPaths, lpszOpenFile, strInitialPath);	

	// Close current project.  This will automatically ask the user to save
	// changes if the project is modified.  By doing this, we can re-open the
	// same project and have it re-initialize (MFC would otherwise not close
	// and re-open).
	CWinApp::OnFileNew ();

	// OK open the correct project:
	strFileName = cOpenDlg.GetPathName ();
	OpenDocumentFile (strFileName);

	// Add this file to the MRU list:
	m_pRecentFileList->Add (strFileName);
}

// **************************************************************************
// OnFileOpenMRU
//
// Description:
//	Open recently used file menu event handler.
//
// Parameters:
//  UINT		nID		Index of MRU file.
//
// Returns:
//  BOOL - Result of base class processing.
// **************************************************************************
BOOL CKApp::OnFileOpenMRU (UINT nID)
{
	// Close current project.  This will automatically ask the user to save
	// changes if the project is modified.  By doing this, we can re-open the
	// same project and have it re-initialize (MFC would otherwise not close
	// and re-open).
	CWinApp::OnFileNew ();

	// Perform default processing and return result:
	return (CWinApp::OnOpenRecentFile (nID));
}
