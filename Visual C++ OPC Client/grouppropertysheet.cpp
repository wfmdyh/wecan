// **************************************************************************
// grouppropertysheet.cpp
//
// Description:
//	Implements a property sheet class and associated propety page classes for
//	OPC group properties.
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
#include "grouppropertysheet.h"
#include "server.h"
#include "group.h"

// Image list indices:
#define ILI_INTERFACE		2

// Default group name if the server does not provide a name:
#define DEFAULTGROUPNAME	_T("Group0")


/////////////////////////////////////////////////////////////////////////////
// CKGroupGeneralPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKGroupGeneralPage, CPropertyPage)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKGroupGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CKGroupGeneralPage)
	ON_BN_CLICKED (IDC_ACTIVE, OnChange)
	ON_EN_CHANGE (IDC_LANGUAGEID, OnChange)
	ON_EN_CHANGE (IDC_NAME, OnChange)
	ON_EN_CHANGE (IDC_PERCENTDEADBAND, OnChange)
	ON_EN_CHANGE (IDC_TIMEBIAS, OnChange)
	ON_EN_CHANGE (IDC_UPDATERATE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKGroupGeneralPage ()
//
// Description:
//	Constructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKGroupGeneralPage::CKGroupGeneralPage () : CPropertyPage (CKGroupGeneralPage::IDD)
	{
	// Initialize member variables:

	//{{AFX_DATA_INIT(CKGroupGeneralPage)
	m_bActive		= GROUP_DEFAULT_ACTIVESTATE;
	m_dwLanguageID	= GROUP_DEFAULT_LANGUAGEID;
	m_strName		= GROUP_DEFAULT_NAME;
	m_fDeadband		= GROUP_DEFAULT_DEADBAND;
	m_lBias			= GROUP_DEFAULT_TIMEBIAS;
	m_dwUpdateRate	= GROUP_DEFAULT_UPDATERATE;
	m_nUpdateMethod = GROUP_DEFAULT_UPDATEMETHOD;
	//}}AFX_DATA_INIT

	m_bModified = false;
	}

// **************************************************************************
// ~CKGroupGeneralPage ()
//
// Description:
//	Destructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKGroupGeneralPage::~CKGroupGeneralPage ()
	{
	}

// **************************************************************************
// DoDataExchange ()
//
// Description:
//	This method is called by the framework to exchange and validate dialog data.
//
// Parameters:
//  CDataExchange	*pDX		A pointer to a CDataExchange object.
//
// Returns:
//  void
// **************************************************************************
void CKGroupGeneralPage::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CPropertyPage::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKGroupGeneralPage)
	DDX_Check (pDX, IDC_ACTIVE, m_bActive);
	DDX_Text (pDX, IDC_LANGUAGEID, m_dwLanguageID);
	DDV_MinMaxDWord (pDX, m_dwLanguageID, 0, 4294967295);
	DDX_Text (pDX, IDC_NAME, m_strName);
	DDX_Text (pDX, IDC_PERCENTDEADBAND, m_fDeadband);
	DDV_MinMaxFloat (pDX, m_fDeadband, 0.f, 100.f);
	DDX_Text (pDX, IDC_TIMEBIAS, m_lBias);
	DDV_MinMaxLong (pDX, m_lBias, 0, 2147483647);
	DDX_CBIndex (pDX, IDC_UPDATENOTIFICATION, m_nUpdateMethod);
	DDX_Text (pDX, IDC_UPDATERATE, m_dwUpdateRate);
	DDV_MinMaxDWord (pDX, m_dwUpdateRate, 0, 4294967295);
	//}}AFX_DATA_MAP
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupGeneralPage message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnInitDialog ()
//
// Description:
//	Called by framework immediately before the dialog box is displayed,
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE.
// **************************************************************************
BOOL CKGroupGeneralPage::OnInitDialog () 
{
	// Perform default processing:
	CPropertyPage::OnInitDialog ();
	
	// Subclass our numeric controls for character filtering:
	m_cLangIDEdit.SubclassDlgItem (IDC_LANGUAGEID, this);
	m_cDeadbandEdit.SubclassDlgItem (IDC_PERCENTDEADBAND, this);
	m_cBiasEdit.SubclassDlgItem (IDC_TIMEBIAS, this);
	m_cUpdateRateEdit.SubclassDlgItem (IDC_UPDATERATE, this);
	GetParent()->SetDlgItemText(IDOK,STR_OK);
	GetParent()->SetDlgItemText(IDCANCEL,STR_CANCEL);
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE); 
}

// **************************************************************************
// OnOK ()
//
// Description:
//	Called when the user clicks the OK button.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupGeneralPage::OnOK () 
	{
	// Force a data exchange between controls and member variables:
	// (Take data from controls and us it to set member variables.)
	UpdateData (TRUE);
	}

// **************************************************************************
// OnChange ()
//
// Description:
//	Called by framework when user modifies data attached to a control.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupGeneralPage::OnChange () 
	{
	// Set our local modified flag:
	m_bModified = true;

	// Set base class modified flag.  This will enable the "Apply" button.
	SetModified (TRUE);
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupInterfacesPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKGroupInterfacesPage, CPropertyPage)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKGroupInterfacesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CKGroupInterfacesPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKGroupInterfacesPage ()
//
// Description:
//	Constructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKGroupInterfacesPage::CKGroupInterfacesPage () : CPropertyPage (CKGroupInterfacesPage::IDD)
	{
	// Initialize member variables:

	//{{AFX_DATA_INIT(CKGroupInterfacesPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	m_pGroup = NULL;
	}

// **************************************************************************
// ~CKGroupInterfacesPage ()
//
// Description:
//	Destructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKGroupInterfacesPage::~CKGroupInterfacesPage ()
	{
	}

// **************************************************************************
// DoDataExchange ()
//
// Description:
//	This method is called by the framework to exchange and validate dialog data.
//
// Parameters:
//  CDataExchange	*pDX		A pointer to a CDataExchange object.
//
// Returns:
//  void
// **************************************************************************
void CKGroupInterfacesPage::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CPropertyPage::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKGroupInterfacesPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupInterfacesPage message handlers
/////////////////////////////////////////////////////////////////////////////

// Define number of possible interfaces at the group level (currently 8).
#define NUMGROUPINTERFACES	8

// **************************************************************************
// OnInitDialog ()
//
// Description:
//	Called by framework immediately before the dialog box is displayed,
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE.
// **************************************************************************
BOOL CKGroupInterfacesPage::OnInitDialog () 
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

	// Insert our columns and headings:

	// Interface column:
	strLoader.LoadString (IDS_INTERFACE);
	pList->InsertColumn (0, strLoader, LVCFMT_LEFT, 200);

	// Reguired column:
	strLoader.LoadString (IDS_REQUIRED);
	pList->InsertColumn (1, strLoader, LVCFMT_LEFT, 75);

	// Supported column:
	strLoader.LoadString (IDS_SUPPORTED);
	pList->InsertColumn (2, strLoader, LVCFMT_LEFT, 75);

	// The group should have been set by now:
	ASSERT (m_pGroup != NULL);

	// Insert the interface data:

	// First load "Yes" and "No" string resources:
	strNo.LoadString (IDS_NO);
	strYes.LoadString (IDS_YES);

	// Loop over all interfaces:
	nIndex = 0;
	while (nIndex < NUMGROUPINTERFACES)
		{
		switch (nIndex)
			{
			// IOPCGroupStateMgt
			case 0:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCGroupStateMgt"), ILI_INTERFACE);
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);
				
				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIGroupStateMgtSupported () ? strYes : strNo);
				break;

			// IOPCPublicGroupStateMgt
			case 1:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCPublicGroupStateMgt"), ILI_INTERFACE);
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strNo);						

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIPublicGroupStateMgtSupported () ? strYes : strNo);
				break;

			// IItemMgt
			case 2:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCItemMgt"), ILI_INTERFACE);	
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);					

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIItemMgtSupported () ? strYes : strNo);
				break;

			// IOPCSyncIO
			case 3:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCSyncIO"), ILI_INTERFACE);

				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);					

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsISyncIOSupported () ? strYes : strNo);				
				break;

			// IOPCAsyncIO
			case 4:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCAsyncIO"), ILI_INTERFACE);
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);					

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIAsyncIOSupported () ? strYes : strNo);				
				break;

			// IDataObject
			case 5:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IDataObject"), ILI_INTERFACE);
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);					

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIDataObjectSupported () ? strYes : strNo);				
				break;

			// IOPCAsyncIO2
			case 6:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IOPCAsyncIO2 (2.0 only)"), ILI_INTERFACE);
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);						

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIAsyncIO2Supported () ? strYes : strNo);				
				break;

			// IConnectionPointContainer
			case 7:
				// Insert interface item in list control:
				pList->InsertItem (nIndex, _T ("IConnectionPointContainer (2.0 only)"), ILI_INTERFACE);	
				
				// Set required field:
				pList->SetItemText (nIndex, 1, strYes);										

				// Set supported field:
				pList->SetItemText (nIndex, 2, 
					m_pGroup->IsIConnectionPointContainerSupported () ? strYes : strNo);				
				break;

			default:
				// Unexpected interface index.  Programmer error.
				ASSERT (FALSE);
				break;
			}

		// Increment interface index for next time around:
		nIndex++;
		}

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupPropertySheet
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC (CKGroupPropertySheet, CPropertySheet)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKGroupPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CKGroupPropertySheet)
	ON_BN_CLICKED (ID_APPLY_NOW, OnApplyNow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKGroupPropertySheet ()
//
// Description:
//	Constructor for creating a new group object.
//
// Parameters:
//  CKServer	*pServer		Pointer to server object.
//
// Returns:
//  none
// **************************************************************************
CKGroupPropertySheet::CKGroupPropertySheet (CKServer *pServer)
	: CPropertySheet (IDS_GROUP_PROPERTIES)
	{
	// Server should have been specified:
	ASSERT (pServer != NULL);

	// Initialize member variables:
	m_pServer = pServer;
	m_pGroup = NULL;

	// Add general page:
	AddPage (&m_cGeneralPage);

	// Remove apply button for group creation (This contructor version will
	// only be used when a new group is being created).
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	}

// **************************************************************************
// CKGroupPropertySheet ()
//
// Description:
//	Constructor for editing an existing group object.
//
// Parameters:
//  CKGroup		*pGroup		Pointer to group object.
//
// Returns:
//  none
// **************************************************************************
CKGroupPropertySheet::CKGroupPropertySheet (CKGroup *pGroup)
	: CPropertySheet (IDS_GROUP_PROPERTIES)
	{
	// Group should have been specified:
	ASSERT (pGroup != NULL);

	// Initialize member variables:
	m_pGroup = pGroup;
	m_pServer = NULL;
	
	HRESULT hr = E_FAIL;
	OPCHANDLE hClient;
	OPCHANDLE hServer;
	LPWSTR pszName;

	IOPCGroupStateMgt* pIGSM = m_pGroup->GetIGroupStateMgt ();

	// Get the OPC Group state as known by the server:
	if (pIGSM != NULL)
		{
		hr = pIGSM->GetState (
			&m_cGeneralPage.m_dwUpdateRate,
			&m_cGeneralPage.m_bActive,
			&pszName,
			&m_cGeneralPage.m_lBias,
			&m_cGeneralPage.m_fDeadband,
			&m_cGeneralPage.m_dwLanguageID,
			&hClient,
			&hServer);

		if (SUCCEEDED (hr))
			{
			ASSERT (hClient == (OPCHANDLE) pGroup);

			// Save and free group name:
			if (pszName)
				{
				m_cGeneralPage.m_strName = pszName;
				CoTaskMemFree (pszName);
				}
			}
		}

	// Else use the state currently set for the CKGroup object:
	else
		{
		m_cGeneralPage.m_strName = m_pGroup->GetName ();
		m_cGeneralPage.m_dwUpdateRate = m_pGroup->GetUpdateRate ();
		m_cGeneralPage.m_bActive = m_pGroup->IsActive ();
		m_cGeneralPage.m_lBias = m_pGroup->GetBias ();
		m_cGeneralPage.m_fDeadband = m_pGroup->GetDeadband ();
		m_cGeneralPage.m_dwLanguageID = m_pGroup->GetLanguageID ();
		}

	m_cGeneralPage.m_nUpdateMethod	= m_pGroup->GetUpdateMethod ();

	// Add general page:
	AddPage (&m_cGeneralPage);

	// Pass a pointer to the supported interface page:
	m_cInterfacePage.m_pGroup = m_pGroup;

	// Add the interface page:
	AddPage (&m_cInterfacePage);
	}

// **************************************************************************
/// ~CKGroupPropertySheet ()
//
// Description:
//	Destructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKGroupPropertySheet::~CKGroupPropertySheet ()
	{
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupPropertySheet message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// DoModal ()
//
// Description:
//	Augment DoModal behaviour of dialog.  Create new group object if
//	necessary.  Apply changes to object properties.
//
// Parameters:
//  none
//
// Returns:
//  int - IDOK, IDCANCEL.
// **************************************************************************
int CKGroupPropertySheet::DoModal () 
	{
	// Perform default processing:
	int nRC = CPropertySheet::DoModal ();

	// If the user selects "OK" add/modify group:
	if (nRC == IDOK)
		{
		// If m_pGroup is NULL, we need to create a new group:
		if (m_pGroup == NULL)
			{
			ASSERT (m_pServer != NULL);

			try
				{
				// Instantiate a new CKGroup:
				m_pGroup = new CKGroup (m_pServer);
				}
			
			catch (...)
				{
				// If problem, delete the group and return with IDCANCEL code:
				ASSERT (FALSE);

				m_pGroup = NULL;
				return (IDCANCEL);
				}
			}
		
		// Otherwise apply changes to existing group:
		else
			OnApplyNow ();

		// Set attributes from general page:

		// Create name if currently undefined:
		if (m_cGeneralPage.m_strName.IsEmpty ())
			{
			if (!m_pServer || !m_pServer->IsAlive ())
				{
				m_cGeneralPage.m_strName = DEFAULTGROUPNAME;
				m_pServer->GenerateGroupName (m_cGeneralPage.m_strName);
				}
			}

		m_pGroup->SetName (m_cGeneralPage.m_strName);
		m_pGroup->SetActive (m_cGeneralPage.m_bActive);
		m_pGroup->SetLanguageID (m_cGeneralPage.m_dwLanguageID);
		m_pGroup->SetDeadband (m_cGeneralPage.m_fDeadband);
		m_pGroup->SetBias (m_cGeneralPage.m_lBias);
		m_pGroup->SetUpdateRate (m_cGeneralPage.m_dwUpdateRate);
		m_pGroup->SetUpdateMethod (m_cGeneralPage.m_nUpdateMethod);

		// If this is a new group (indicated by non-NULL m_pServer),
		// add it to the server:
		if (m_pServer)
			m_pServer->AddGroup (m_pGroup);
		}

	// Pass along return code from default processing:
	return (nRC);
	}

// **************************************************************************
// OnApplyNow ()
//
// Description:
//	Apply changes to group object properties.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupPropertySheet::OnApplyNow ()
	{
	// See if there are any changes to apply:
	if (m_cGeneralPage.IsChanged ())
		{
		// Force an update data:
		if (m_cGeneralPage.m_hWnd)
			m_cGeneralPage.UpdateData (TRUE);
		
		// Get the group state management interface:
		ASSERT (m_pGroup != NULL);
		IOPCGroupStateMgt* pIGSM = m_pGroup->GetIGroupStateMgt ();

		// Set OPC Group state:
		if (pIGSM)
			{
			HRESULT hr = E_FAIL;
			DWORD dwRevUpdateRate = 0;					// for [out] args

			OPCHANDLE hClient = (OPCHANDLE) m_pGroup;	// keep same client handle
			OPCHANDLE hServer;
			LPWSTR pszName;

			// Set the state based on current settings:
			hr = pIGSM->SetState ( 
				&m_cGeneralPage.m_dwUpdateRate,		// update rate
				&dwRevUpdateRate,					// revised update rate
				&m_cGeneralPage.m_bActive,			// active state
				&m_cGeneralPage.m_lBias,			// time bias
				&m_cGeneralPage.m_fDeadband,		// deadband
				&m_cGeneralPage.m_dwLanguageID,		// language id
				&hClient);

			ASSERT (SUCCEEDED (hr));

			// Allocate memorty to contain the name of the group:
			pszName = (LPWSTR) CoTaskMemAlloc ((m_cGeneralPage.m_strName.GetLength () + 1) *
				sizeof (WCHAR));

			// Set the group name:
			if (pszName)
				{
				// COM requires that all string be in UNICODE format, so 
				// conversion may be necessary.
#ifdef _UNICODE
				// This is a UNICODE build, so no need to convert.  Just copy
				// our name to output buffer:
				lstrcpyn (pszName, m_cGeneralPage.m_strName, m_cGeneralPage.m_strName.GetLength () + 1);
#else
				// This is an ANSI build, so convert to UNICODE and place
				// result in output buffer:
				_mbstowcsz (pszName, m_cGeneralPage.m_strName, m_cGeneralPage.m_strName.GetLength () + 1);
#endif
				// Set the name:
				pIGSM->SetName (pszName);

				// Free the memory allocated for the name:
				CoTaskMemFree (pszName);
				}

			// Now get the state to verify that the server changed our settings:
			hr = pIGSM->GetState (
				&m_cGeneralPage.m_dwUpdateRate,
				&m_cGeneralPage.m_bActive,
				&pszName,
				&m_cGeneralPage.m_lBias,
				&m_cGeneralPage.m_fDeadband,
				&m_cGeneralPage.m_dwLanguageID,
				&hClient,
				&hServer);

			ASSERT (SUCCEEDED (hr));
			ASSERT (hClient == (OPCHANDLE) m_pGroup);
			ASSERT (hServer == m_pGroup->GetServerHandle ());

			// Get and free group name:
			if (pszName)
				{
				m_cGeneralPage.m_strName = pszName;
				CoTaskMemFree (pszName);
				}

			// Gransfer get state data to dialog if it is not closing from an OK:
			if (m_cGeneralPage.m_hWnd)
				m_cGeneralPage.UpdateData (FALSE);
			}

		// Set changed flag:
		m_cGeneralPage.SetChanged (false);
		}
	}

// **************************************************************************
// OnInitDialog ()
//
// Description:
//	Called by framework immediately before the dialog box is displayed,
//
// Parameters:
//  none
//
// Returns:
//  BOOL - Result of base class processing.
// **************************************************************************
BOOL CKGroupPropertySheet::OnInitDialog () 
{
	// Perform default processing:
	BOOL bResult = CPropertySheet::OnInitDialog ();
	
	// Append item count to title:
	if (m_pGroup)
	{
		// Get current title text:
		CString strTitle;
		GetWindowText (strTitle);

		// Format a string to contain item count:
		CString strItemCount;
		strItemCount.Format (IDS_APPENDITEMCOUNT, m_pGroup->GetItemCount ());

		// Append count to tile:
		strTitle += strItemCount;

		// Set window title to modified:
		SetWindowText (strTitle);
	}
	SetDlgItemText(ID_APPLY_NOW,STR_APPLY);
	// Return result from default processing:
	return (bResult);
}
