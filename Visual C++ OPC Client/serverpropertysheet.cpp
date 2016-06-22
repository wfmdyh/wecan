// **************************************************************************
// serverpropertysheet.cpp
//
// Description:
//	Implements a property sheet class and associated property page classes for
//	OPC server properties.
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
#include "serverpropertysheet.h"
#include "server.h"

static LPCTSTR lpszDataAccessServers10 = _T("OPC Data Access Servers Version 1.0");
static LPCTSTR lpszDataAccessServers20 = _T("OPC Data Access Servers Version 2.0");
static LPCTSTR lpszOPCServers = _T("OPC General");

static const CATID CATID_OPCDAServer10 = 
{ 0x63d5f430, 0xcfe4, 0x11d1, { 0xb2, 0xc8, 0x0, 0x60, 0x8, 0x3b, 0xa1, 0xfb } };
// {63D5F430-CFE4-11d1-B2C8-0060083BA1FB}

static const CATID CATID_OPCDAServer20 = 
{ 0x63d5f432, 0xcfe4, 0x11d1, { 0xb2, 0xc8, 0x0, 0x60, 0x8, 0x3b, 0xa1, 0xfb } };
// {63D5F432-CFE4-11d1-B2C8-0060083BA1FB}

// image list indices
#define ILI_CATAGORY	0
#define ILI_COMPONENT	1
#define ILI_INTERFACE	2


/////////////////////////////////////////////////////////////////////////////
// CKServerGeneralPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKServerGeneralPage, CPropertyPage)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerGeneralPage, CDialog)
//{{AFX_MSG_MAP(CKServerGeneralPage)
ON_NOTIFY (TVN_SELCHANGED, IDC_SERVERLIST, OnSelChanged)
//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerGeneralPage ()
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
CKServerGeneralPage::CKServerGeneralPage ()
: CPropertyPage (CKServerGeneralPage::IDD)
{
	// Initialize member variables:
	m_strProgID = cApp.GetDefConnectProgID ();
	m_strRemoteMachine.Empty ();

	m_pServerList = NULL;
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
void CKServerGeneralPage::DoDataExchange (CDataExchange *pDX)
{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKServerGeneralPage)
	DDX_Text (pDX, IDC_MACHINENAME, m_strRemoteMachine);
	DDV_MaxChars (pDX, m_strRemoteMachine, 256);
	DDX_Text (pDX, IDC_PROGID, m_strProgID);
	DDV_MaxChars (pDX, m_strProgID, 256);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CKServerGeneralPage message handlers
/////////////////////////////////////////////////////////////////////////////

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
//	BOOL - TRUE.
// **************************************************************************
BOOL CKServerGeneralPage::OnInitDialog () 
{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;
	CDialog::OnInitDialog ();

	// Initialize the tree control:
	m_pServerList = (CTreeCtrl *) GetDlgItem (IDC_SERVERLIST);
	ASSERT (m_pServerList != NULL);

	// Initialize the image list for the interfaces. The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 16x16 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	// Image number		Use
	//	0				not used
	//	1				not used
	//	2				Interface
	m_cImageList.Create (IDB_COMPONENTS, 16, 4, RGB (255, 0, 255));
	m_cImageList.SetBkColor (CLR_NONE);
	m_pServerList->SetImageList (&m_cImageList, TVSIL_NORMAL);

	HTREEITEM hParent;

	// Insert data access 1.0 servers:
	hParent = m_pServerList->InsertItem (lpszDataAccessServers10, ILI_CATAGORY, ILI_CATAGORY);
	DisplayComponentCatList (hParent, CATID_OPCDAServer10);

	// Insert data access 2.0 servers:
	hParent = m_pServerList->InsertItem (lpszDataAccessServers20, ILI_CATAGORY, ILI_CATAGORY);
	DisplayComponentCatList (hParent, CATID_OPCDAServer20);

	// Insert general OPC registered servers:
	hParent = m_pServerList->InsertItem (lpszOPCServers, ILI_CATAGORY, ILI_CATAGORY);
	DisplayGeneralOPCServers (hParent);

	//更改父窗口的按钮名字
	GetParent()->SetDlgItemText(IDOK,STR_OK);
	GetParent()->SetDlgItemText(IDCANCEL,STR_CANCEL);
	return (TRUE);  
}

// **************************************************************************
// OnSelChanged ()
//
// Description:
//	Handles notification that a new item in tree control has been selected.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKServerGeneralPage::OnSelChanged (NMHDR *pNMHDR, LRESULT *pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// see if it was a server selection (i.e, has a parent item)

	// Update page is server was selected (non-NULL parent item):
	if (m_pServerList->GetParentItem (pNMTreeView->itemNew.hItem) != NULL)
	{
		// Transfer selection to ProgID edit control:
		m_strProgID = m_pServerList->GetItemText (pNMTreeView->itemNew.hItem);

		// Update controls:
		UpdateData (FALSE);

		// Make sure selected server is visible:
		m_pServerList->EnsureVisible (pNMTreeView->itemNew.hItem); 
	}

	*pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CKServerGeneralPage helpers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// DisplayComponentCatList ()
//
// Description:
//	Construct a list of installed 1.0 and 2.0 OPC servers and insert into 
//	tree control.
//
// Parameters:
//  HTREEITEM	hParent		Handle of parent tree control item.
//	CATID		catid		Catagory ID (CATID_OPCDAServer10 or
//							  CATID_OPCDAServer20).
//
// Returns:
//  void
// **************************************************************************
void CKServerGeneralPage::DisplayComponentCatList (HTREEITEM hParent, CATID catid)
{
	HRESULT hr;

	// Make sure COM is initialized:
	hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);

	if (SUCCEEDED (hr))
	{
		ICatInformation *pCat = NULL;

		// Get component category manager:
		hr = CoCreateInstance (CLSID_StdComponentCategoriesMgr, 
			NULL,
			CLSCTX_SERVER, 
			IID_ICatInformation,
			(void **)&pCat);

		// If succeeded, enumerate registered components:
		if (SUCCEEDED (hr))
		{
			IEnumCLSID *pEnum = NULL;

			CATID arrcatid [1];
			arrcatid [0] = catid;

			// Enumerate registered components based on clsid:
			hr = pCat->EnumClassesOfCategories (
				sizeof (arrcatid) / sizeof (CATID),	// number of catids in the array that follows
				arrcatid,							// catid array
				0, 
				NULL,
				&pEnum);							// clsid enumerator for registered components under this category

			// If succeeded, process results:
			if (SUCCEEDED (hr))
			{
				GUID guid;
				ULONG fetched;

				// Loop over enumerated components.  Call enemerator's next
				// member function to reference next component and get its
				// guid:
				while ((hr = pEnum->Next (1, &guid, &fetched)) == S_OK)
				{
					// Get the ProgID from the guid:
					WCHAR *wszProgID;
					hr = ProgIDFromCLSID (guid, &wszProgID);

					// If succeeded, add component to list:
					if (SUCCEEDED (hr))
					{
						// ProgID string will be in UNICODE format.  Convert to 
						// ANSI format if this is and ANSI build.  Insert component
						// into list:
					#ifdef _UNICODE
						m_pServerList->InsertItem (wszProgID, ILI_COMPONENT, ILI_COMPONENT, hParent);
					#else
						TCHAR szProgID [DEFBUFFSIZE];

						_wcstombsz (szProgID, wszProgID, sizeof (szProgID) / sizeof (TCHAR));
						m_pServerList->InsertItem (szProgID, ILI_COMPONENT, ILI_COMPONENT, hParent);
					#endif
						// It is up to us to free the Prog ID string memory:
						CoTaskMemFree (wszProgID);
					}
				}

				// Release our enumerator:
				pEnum->Release ();
			}
			// release our category mamager
			pCat->Release ();
		}
		// Uninitialize COM:
		CoUninitialize ();
	}
}

// **************************************************************************
// DisplayGeneralOPCServers ()
//
// Description:
//	Construct a list of installed general OPC servers and insert into tree
//	control.
//
// Parameters:
//  HTREEITEM	hParent		Handle of parent tree control item.
//
// Returns:
//  void
// **************************************************************************
void CKServerGeneralPage::DisplayGeneralOPCServers (HTREEITEM hParent)
{
	HKEY hKey = HKEY_CLASSES_ROOT;		// search under this key
	TCHAR szKey [DEFBUFFSIZE];			// allocate key buffer
	DWORD dwLength = DEFBUFFSIZE;

	// Search the registry for installed OPC Servers:
	for (DWORD dwIndex = 0; 
		RegEnumKey (hKey, dwIndex, szKey, dwLength) == ERROR_SUCCESS; 
		++dwIndex)
	{
		HKEY hSubKey;

		// Open the registry key:
		if (RegOpenKey (hKey, szKey, &hSubKey) == ERROR_SUCCESS)
		{
			// Search for OPC subkey:
			if (RegQueryValue (hSubKey, _T("OPC"), NULL, NULL) == ERROR_SUCCESS)
			{
				// Display the prog ID for this server:
				m_pServerList->InsertItem (szKey, ILI_COMPONENT, ILI_COMPONENT, hParent);
			}

			// Close the registry key:
			RegCloseKey (hSubKey);
		}

		// Re-initialize length:
		dwLength = DEFBUFFSIZE;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CKServerStatusPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKServerStatusPage, CPropertyPage)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerStatusPage, CPropertyPage)
//{{AFX_MSG_MAP(CKServerStatusPage)
//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerStatusPage ()
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
CKServerStatusPage::CKServerStatusPage () : CPropertyPage (CKServerStatusPage::IDD)
{
	// Initialize member variables:

	//{{AFX_DATA_INIT(CKServerStatusPage)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pServer = NULL;
}

// **************************************************************************
// ~CKServerStatusPage ()
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
CKServerStatusPage::~CKServerStatusPage ()
{
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
void CKServerStatusPage::DoDataExchange (CDataExchange *pDX)
{
// Perform default processing:
CPropertyPage::DoDataExchange (pDX);

// Exchange data between controls and associated member variables:
//{{AFX_DATA_MAP(CKServerStatusPage)
// NOTE: the ClassWizard will add DDX and DDV calls here
//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CKServerStatusPage message handlers
/////////////////////////////////////////////////////////////////////////////

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
//	BOOL - TRUE.
// **************************************************************************
BOOL CKServerStatusPage::OnInitDialog () 
{
	IOPCServer *pIServer = NULL;
	OPCSERVERSTATUS *pServerStatus;
	CString strLoader;
	CString strStatus;
	CString strTemp;

	// Perform default processing:
	CPropertyPage::OnInitDialog ();

	// Get IServer interface:
	ASSERT (m_pServer != NULL);
	pIServer = m_pServer->GetIServer ();

	// Add server name to status string:
	strLoader.FormatMessage (IDS_PROGID, m_pServer->GetProgID ());
	strStatus += strLoader;

	// Add remote machine (if applicable) to status string:
	if (m_pServer->GetRemoteMachine ())
	{
		strLoader.FormatMessage (IDS_REMOTEMACHINE, m_pServer->GetRemoteMachine ());
		strStatus += strLoader;
	}

	// Get the server status:
	if (pIServer && SUCCEEDED (pIServer->GetStatus (&pServerStatus)) && pServerStatus)
	{
		FILETIME ftLocal;
		TCHAR szBuffer [DEFBUFFSIZE];

		// Add vendor name to status string:
		if (pServerStatus->szVendorInfo)
		{
			// Convert venter name from UNICODE if needed:
		#ifdef _UNICODE
			lstrcpyn (szBuffer, pServerStatus->szVendorInfo, sizeof (szBuffer) / sizeof (TCHAR));
		#else
			_wcstombsz (szBuffer, pServerStatus->szVendorInfo, sizeof (szBuffer) / sizeof (TCHAR));
		#endif
			// Add vender name:
			strLoader.FormatMessage (IDS_VENDORNAME, szBuffer);
			strStatus += strLoader;

			// Free memory allocated for vender info:
			CoTaskMemFree (pServerStatus->szVendorInfo);
		}

		// Add start time to status string:
		if (FileTimeToLocalFileTime (&pServerStatus->ftStartTime, &ftLocal))
		{
			SYSTEMTIME systime;
			if (FileTimeToSystemTime (&ftLocal, &systime))
			{
				strTemp.Format (_T("%02d:%02d:%02d:%03d"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);			
				strLoader.FormatMessage (IDS_STARTTIME, strTemp);
				strStatus += strLoader;
			}
		}	

		// Add current time to status string:
		if (FileTimeToLocalFileTime (&pServerStatus->ftCurrentTime, &ftLocal))
		{
			SYSTEMTIME systime;
			if (FileTimeToSystemTime (&ftLocal, &systime))
			{
				strTemp.Format (_T("%02d:%02d:%02d:%03d"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);			
				strLoader.FormatMessage (IDS_CURRENTTIME, strTemp);
				strStatus += strLoader;
			}
		}

		// Add last update time to status string:
		if (FileTimeToLocalFileTime (&pServerStatus->ftLastUpdateTime, &ftLocal))
		{
			SYSTEMTIME systime;
			if (FileTimeToSystemTime (&ftLocal, &systime))
			{
				strTemp.Format (_T("%02d:%02d:%02d:%03d"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);			
				strLoader.FormatMessage (IDS_LASTUPDATETIME, strTemp);
				strStatus += strLoader;
			}
		}

		// Create a string describing server state:
		switch (pServerStatus->dwServerState)
		{
		case OPC_STATUS_RUNNING:
			strTemp.LoadString (IDS_RUNNING);
			strLoader.FormatMessage (IDS_SERVERSTATE, strTemp); 
			break;

		case OPC_STATUS_FAILED: 
			strTemp.LoadString (IDS_FAILED);
			strLoader.FormatMessage (IDS_SERVERSTATE, strTemp); 
			break;

		case OPC_STATUS_NOCONFIG: 
			strTemp.LoadString (IDS_NOCONFIG);
			strLoader.FormatMessage (IDS_SERVERSTATE, strTemp); 
			break;

		case OPC_STATUS_SUSPENDED:
			strTemp.LoadString (IDS_SUSPENDED);
			strLoader.FormatMessage (IDS_SERVERSTATE, strTemp); 
			break;

		case OPC_STATUS_TEST: 
			strTemp.LoadString (IDS_TEST);
			strLoader.FormatMessage (IDS_SERVERSTATE, strTemp); 
			break;

		default: 
			strLoader.FormatMessage (IDS_SERVERSTATE, _T("???")); 
			break;			
		}

		// Add server state to status string:
		strStatus += strLoader;

		// Add group count to status string:
		strTemp.Format (_T("%d"), pServerStatus->dwGroupCount);
		strLoader.FormatMessage (IDS_GROUPCOUNT, strTemp);
		strStatus += strLoader;

		// Add major version number to status string:
		strTemp.Format (_T("%d"), pServerStatus->wMajorVersion);
		strLoader.FormatMessage (IDS_MAJORVERSION, strTemp);
		strStatus += strLoader;

		// Add minor version to status string:
		strTemp.Format (_T("%d"), pServerStatus->wMinorVersion);
		strLoader.FormatMessage (IDS_MINORVERSION, strTemp);
		strStatus += strLoader;

		// Add build number to status string:
		strTemp.Format (_T("%d"), pServerStatus->wBuildNumber);
		strLoader.FormatMessage (IDS_BUILDNUMBER, strTemp);
		strStatus += strLoader;

		// Free memeory allocated for server status:
		CoTaskMemFree (pServerStatus);
	}

	// Else if failed, add string stating unable to determin server status
	// to status string:
	else
	{
		strLoader.LoadString (IDS_UNABLE_TO_DETERMINE_STATUS);
		strStatus += strLoader;
	}

	// Display the status string:
	((CStatic *) GetDlgItem (IDC_STATUS))->SetWindowText (strStatus);

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
}


/////////////////////////////////////////////////////////////////////////////
// CKServerInterfacesPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKServerInterfacesPage, CPropertyPage)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerInterfacesPage, CPropertyPage)
//{{AFX_MSG_MAP(CKServerInterfacesPage)
//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerInterfacesPage ()
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
CKServerInterfacesPage::CKServerInterfacesPage () : CPropertyPage (CKServerInterfacesPage::IDD)
{
	// Initialize member variables:

	//{{AFX_DATA_INIT(CKServerInterfacesPage)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pServer = NULL;
}

// **************************************************************************
// ~CKServerInterfacesPage ()
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
CKServerInterfacesPage::~CKServerInterfacesPage ()
{
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
void CKServerInterfacesPage::DoDataExchange (CDataExchange *pDX)
{
	// Perform default processing:
	CPropertyPage::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKServerInterfacesPage)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CKServerInterfacesPage message handlers
/////////////////////////////////////////////////////////////////////////////

#define NUMGROUPINTERFACES	7	// currently 7 possible interfaces at the server level

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
//	BOOL - TRUE.
// **************************************************************************
BOOL CKServerInterfacesPage::OnInitDialog () 
{
	CListCtrl *pList = NULL;
	CString strLoader;
	CString strYes;
	CString strNo;
	int nIndex;

	// Perform default processing:
	CPropertyPage::OnInitDialog ();

	// Get a pointer to our list control:
	pList = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (pList != NULL);

	// Initialize the image list for the interfaces. The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 16x16 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	// Image number		Use
	//	0				not used
	//	1				not used
	//	2				Interface
	m_cImageList.Create (IDB_COMPONENTS, 16, 4, RGB (255, 0, 255));
	m_cImageList.SetBkColor (CLR_NONE);
	pList->SetImageList (&m_cImageList, LVSIL_SMALL);

	// Create interface column:
	strLoader.LoadString (IDS_INTERFACE);
	pList->InsertColumn (0, strLoader, LVCFMT_LEFT, 200);

	// Create required column:
	strLoader.LoadString (IDS_REQUIRED);
	pList->InsertColumn (1, strLoader, LVCFMT_LEFT, 75);

	// Creaete supported column:
	strLoader.LoadString (IDS_SUPPORTED);
	pList->InsertColumn (2, strLoader, LVCFMT_LEFT, 75);

	// The server had better have been set by now (check for debug):
	ASSERT (m_pServer != NULL);

	// Load "Yes" and "No" strings from string resources:
	strNo.LoadString (IDS_NO);
	strYes.LoadString (IDS_YES);

	// Loop over interfaces and set list control items (one per interface):
	nIndex = 0;
	while (nIndex < NUMGROUPINTERFACES)
	{
		switch (nIndex)
		{
		case 0:
			// IOPCServer:
			pList->InsertItem (nIndex, _T ("IOPCServer"), ILI_INTERFACE);		// interface
			pList->SetItemText (nIndex, 1, strYes);				// required

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIServerSupported () ? strYes : strNo);	// supported
			break;

		case 1:
			// IOPCBrowseServerAddressSpace:
			pList->InsertItem (nIndex, _T ("IOPCBrowseServerAddressSpace"), ILI_INTERFACE);	
			pList->SetItemText (nIndex, 1, strNo);						

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIBrowsingSupported () ? strYes : strNo);
			break;

		case 2:
			// IOPCServerPublicGroups:
			pList->InsertItem (nIndex, _T ("IOPCServerPublicGroups"), ILI_INTERFACE);			
			pList->SetItemText (nIndex, 1, strNo);					

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIServerPublicGroupsSupported () ? strYes : strNo);
			break;

		case 3:
			// IPersistFile:
			pList->InsertItem (nIndex, _T ("IPersistFile"), ILI_INTERFACE);			
			pList->SetItemText (nIndex, 1, strNo);					

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIPersistFileSupported () ? strYes : strNo);				
			break;

		case 4:
			// IOPCCommon:
			pList->InsertItem (nIndex, _T ("IOPCCommon (2.0 only)"), ILI_INTERFACE);			
			pList->SetItemText (nIndex, 1, strYes);					

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsICommonSupported () ? strYes : strNo);				
			break;

		case 5:
			// IOPCItemProperties:
			pList->InsertItem (nIndex, _T ("IOPCItemProperties (2.0 only)"), ILI_INTERFACE);			
			pList->SetItemText (nIndex, 1, strYes);					

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIItemPropertiesSupported () ? strYes : strNo);				
			break;

		case 6:
			// IConnectionPointContainer:
			pList->InsertItem (nIndex, _T ("IConnectionPointContainer (2.0 only)"), ILI_INTERFACE);	
			pList->SetItemText (nIndex, 1, strYes);						

			pList->SetItemText (nIndex, 2, 
				m_pServer->IsIConnectionPointContainerSupported () ? strYes : strNo);				
			break;

		default:
			// Unexpected interface index - programmer error:
			ASSERT (FALSE);
			break;
		}

		// Increment interface index:
		nIndex++;
	}
	SetDlgItemText(IDOK,_T("确定"));
	SetDlgItemText(IDCANCEL,_T("关闭"));
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
}


/////////////////////////////////////////////////////////////////////////////
// CKServerPropertySheet
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC (CKServerPropertySheet, CPropertySheet)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CKServerPropertySheet)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerPropertySheet
//
// Description:
//	Constructor.
//
// Parameters:
//  CKServer	*pServer	Pointer to server object.
//
// Returns:
//  none
// **************************************************************************
CKServerPropertySheet::CKServerPropertySheet (CKServer *pServer)
	: CPropertySheet (IDS_SERVER_PROPERTIES)
{
	// Initialize member variables:
	m_pServer = pServer;

	// Add general page if they are selecting a new server (indicated by
	// NULL server pointer):
	if (m_pServer == NULL)
	{
		AddPage (&m_cGeneralPage);
	}

	// Else add status and interface pages if they are selected an existing
	// server:
	else
	{
		// Add status page:
		m_cStatusPage.m_pServer = m_pServer;
		AddPage (&m_cStatusPage);

		// Add interface page:
		m_cInterfacePage.m_pServer = m_pServer;
		AddPage (&m_cInterfacePage);
	}

	// Remove apply button since there will be nothing to change:
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	
}

// **************************************************************************
// ~CKServerPropertySheet ()
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
CKServerPropertySheet::~CKServerPropertySheet ()
{
	
}


/////////////////////////////////////////////////////////////////////////////
// CKServerPropertySheet message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// DoModal ()
//
// Description:
//	Override of CPropertySheet::DoModal function to give us a change to 
//	create a new CKServer object if needed and attempt to connect to 
//	associated OPC server.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
int CKServerPropertySheet::DoModal () 
{
	
	// Allow for default processing.  Save return value:
	int nRC = CPropertySheet::DoModal ();

	// If we are already connected to a server, reset return code to 
	// IDCANCEL so no changes will be applied.
	if (m_pServer)
		nRC = IDCANCEL;

	// If the user selects OK, and the server is a new connection, add server
	// and attempt a connection:
	if (nRC == IDOK)
	{
		///
		//for (int i=0;i<1300;i++)
		//{
		m_pServer = NULL;
		// Create a new server is server pointer is currently NULL:
		if (m_pServer == NULL)
		{
			try
			{
				m_pServer = new CKServer;
			}

			catch (...)
			{
				// Memory allocation error.  Reset return code to IDCANCEL
				// so no changes will be applied:
				ASSERT (FALSE);

				m_pServer = NULL;
				nRC = IDCANCEL;
			}
		}

		// Attempt to connect to a server:
		if (m_pServer)
		{
			/*m_pServer->Connect (
				m_cGeneralPage.m_strProgID,
				m_cGeneralPage.m_strRemoteMachine);*/
			//m_cGeneralPage.m_strProgID = "WFM.OPCSERVER";
			//m_cGeneralPage.m_strRemoteMachine = "\\\\192.168.1.137";
			m_pServer->Connect (
				m_cGeneralPage.m_strProgID,
				m_cGeneralPage.m_strRemoteMachine);
		}
		///
		//}
	}

	// Return the return code:
	return (nRC);
}




