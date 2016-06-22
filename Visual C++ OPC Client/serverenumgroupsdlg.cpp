// **************************************************************************
// serverenumgroupsdlg.cpp
//
// Description:
//	Implements a dialog class for group enumerations.
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
#include "serverenumgroupsdlg.h"

// Enumerate by scope defines, correspond to the scope combo-box list index:
#define SCOPE_PRIVATE	0
#define SCOPE_PUBLIC	1
#define SCOPE_ALL		2

// Enumerate by interface defines, correspond to the interface combo-box list
// index:
#define ITF_OBJECT		0
#define ITF_NAME		1


/////////////////////////////////////////////////////////////////////////////
// CKServerEnumerateGroupsDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerEnumerateGroupsDlg, CDialog)
	//{{AFX_MSG_MAP(CKServerEnumerateGroupsDlg)
	ON_CBN_SELCHANGE (IDC_GROUPENUM_INTERFACE, OnChange)
	ON_BN_CLICKED (IDC_APPLY, OnApply)
	ON_CBN_SELCHANGE (IDC_GROUPENUM_SCOPE, OnChange)
	ON_NOTIFY (LVN_ITEMCHANGED, IDC_LIST, OnSelectItem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerEnumerateGroupsDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  IOPCServer	*pIServer	Pointer to IOPCServer interface.
//	CWnd		*pParent	Pointer to parent window.
//
// Returns:
//  none
// **************************************************************************
CKServerEnumerateGroupsDlg::CKServerEnumerateGroupsDlg (IOPCServer *pIServer, CWnd *pParent /*=NULL*/)
	: CDialog (CKServerEnumerateGroupsDlg::IDD, pParent)
	{
	// Initialize member variables:
	ASSERT (pIServer != NULL);
	m_pIServer = pIServer;
	m_pEnumList = NULL;

	//{{AFX_DATA_INIT(CKServerEnumerateGroupsDlg)
	m_bActive = FALSE;
	m_nInterface = ITF_NAME;
	m_nScope = SCOPE_ALL;
	m_dwLanguageID = 0;
	m_strName = _T("");
	m_dwUpdateRate = 0;
	m_fDeadband = 0.0f;
	m_lBias = 0;
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
void CKServerEnumerateGroupsDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange date between controls and associated member variables:
	//{{AFX_DATA_MAP(CKServerEnumerateGroupsDlg)
	DDX_Check (pDX, IDC_ACTIVE, m_bActive);
	DDX_CBIndex (pDX, IDC_GROUPENUM_INTERFACE, m_nInterface);
	DDX_CBIndex (pDX, IDC_GROUPENUM_SCOPE, m_nScope);
	DDX_Text (pDX, IDC_LANGUAGEID, m_dwLanguageID);
	DDX_Text (pDX, IDC_NAME, m_strName);
	DDX_Text (pDX, IDC_UPDATERATE, m_dwUpdateRate);
	DDX_Text (pDX, IDC_PERCENTDEADBAND, m_fDeadband);
	DDX_Text (pDX, IDC_TIMEBIAS, m_lBias);
	//}}AFX_DATA_MAP
	}

/////////////////////////////////////////////////////////////////////////////
// CKServerEnumerateGroupsDlg message handlers
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
BOOL CKServerEnumerateGroupsDlg::OnInitDialog () 
	{
	// Perform deafult processing:
	CDialog::OnInitDialog ();
	
	// Get pointer to our list control:
	m_pEnumList = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (m_pEnumList != NULL);

	// Insert a blank column for our list control:
	CRect rc;
	m_pEnumList->GetClientRect (&rc);
	m_pEnumList->InsertColumn (0, _T(""), LVCFMT_LEFT, rc.Width ());	

	// Apply enumeration with default settings:
	OnApply ();

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE	
	return (TRUE);  
	}

// **************************************************************************
// OnChange ()
//
// Description:
//	Default event handler for control change notifications.	
// 
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServerEnumerateGroupsDlg::OnChange () 
	{
	// Enable the apply button:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (TRUE);
	}

// **************************************************************************
// OnApply ()
//
// Description:
//	Apply button event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServerEnumerateGroupsDlg::OnApply () 
	{
	// Force an update data:
	UpdateData (true);

	// Enumerate the groups:
	EnumerateGroups ();

	// Disable the apply button:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (FALSE);
	}

// **************************************************************************
// OnSelectItem ()
//
// Description:
//	Handles notification that a new item from list control has been selected.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKServerEnumerateGroupsDlg::OnSelectItem (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	NM_LISTVIEW* pnmlv= (NM_LISTVIEW*)pNMHDR;
	
	// Wait for selection change then call UpdateState() to update all
	// controls:
	if (pnmlv->uNewState)
		UpdateState (m_pEnumList->GetItemText (pnmlv->iItem, pnmlv->iSubItem));
		
	*pResult = 0;
	}	


/////////////////////////////////////////////////////////////////////////////
// CKServerEnumerateGroupsDlg helpers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// EnumerateGroups ()
//
// Description:
//	Performs the actual group enumeration.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServerEnumerateGroupsDlg::EnumerateGroups ()
	{
	HRESULT hr = E_FAIL;
	OPCENUMSCOPE eScope;
	TCHAR szBuffer [DEFBUFFSIZE];
	
	// Delete any items that are currently displayed:
	m_pEnumList->DeleteAllItems ();

	// Also re-initialize selected group properties to empty:
	m_bActive = FALSE;
	m_dwLanguageID = 0;
	m_strName = _T("");
	m_dwUpdateRate = 0;
	m_fDeadband = 0.0f;
	m_lBias = 0;

	// Update controls:
	UpdateData (false);
	
	// Convert scope combo-box index to true scope:
	switch (m_nScope)
		{
		case SCOPE_PRIVATE:
			eScope = OPC_ENUM_PRIVATE;
			break;

		case SCOPE_PUBLIC:
			eScope = OPC_ENUM_PUBLIC;
			break;

		case SCOPE_ALL:
			eScope = OPC_ENUM_ALL;
			break;

		default:
			break;
		}

	// If enumerate by object:
	if (m_nInterface == ITF_OBJECT)
		{
		// Ask server to create group enumerator interface:
		IEnumUnknown *pEnum = NULL;
		hr = m_pIServer->CreateGroupEnumerator (
			eScope,	IID_IEnumUnknown, (IUnknown**)&pEnum);

		// Process results if succeed:
		if (SUCCEEDED (hr) && pEnum != NULL)
			{
			IUnknown *pIUnknown;
			ULONG uFetched;
			IOPCGroupStateMgt *pIGroupStateMgt = NULL;
			
			// Loop over enumerated groups (call enemerator's next member to
			// reference next group and return its IUnknown interface):
			while ((hr = pEnum->Next (1, &pIUnknown, &uFetched)) == S_OK)
				{
				// Get group state management interface of current group:
				hr = pIUnknown->QueryInterface (IID_IOPCGroupStateMgt, (void**)&pIGroupStateMgt);

				if (SUCCEEDED (hr))
					{
					// Declare some variable needed to get the group name
					// from the server:
					OPCHANDLE hClient;
					OPCHANDLE hServer;
					WCHAR *pszName;

					// Get group state (we are only interested in the group name):
					hr = pIGroupStateMgt->GetState (
						&m_dwUpdateRate,
						&m_bActive,
						&pszName, 
						&m_lBias,
						&m_fDeadband,
						&m_dwLanguageID,
						&hClient,
						&hServer);

					// If succeed, process results:
					if (SUCCEEDED (hr))
						{
						// COM will return group name in UNICODE format.  Will have
						// to convert to ANSI format if ANSI build.  Copy result
						// to buffer:
#ifdef _UNICODE
						lstrcpyn (szBuffer, pszName, sizeof (szBuffer) / sizeof (TCHAR));
#else
						_wcstombsz (szBuffer, pszName, sizeof (szBuffer) / sizeof (TCHAR));
#endif

						// COM requires us to free memory allocated by server
						// for name string:
						if (pszName)
							CoTaskMemFree (pszName);

						// Insert group name into the list control:
						m_pEnumList->InsertItem (m_pEnumList->GetItemCount (), szBuffer);
						}

					// Release group state management interface:
					pIGroupStateMgt->Release ();
					}

				// Release the enumerated unknown interface:
				pIUnknown->Release ();
				}

			// Now release our enumerator:
			pEnum->Release ();
			}
		}

	// Else enumerate by name (generally faster):
	else
		{
		// Ask server to create group enumerator:
		ASSERT (m_nInterface == ITF_NAME);

		IEnumString *pEnum = NULL;
		hr = m_pIServer->CreateGroupEnumerator (
			eScope,	IID_IEnumString, (IUnknown**)&pEnum);

		// Process results if succeed:
		if (SUCCEEDED (hr) && pEnum != NULL)
			{
			LPOLESTR pString;
			ULONG uFetched;

			// Loop over enumerated groups (call enemerator's next member to
			// reference next group and return its name string):
			while ((hr = pEnum->Next (1, &pString, &uFetched)) == S_OK)
				{
				// COM will return group name in UNICODE format.  Will have
				// to convert to ANSI format if ANSI build.  Copy result
				// to buffer:
#ifdef _UNICODE
				lstrcpyn (szBuffer, pString, sizeof (szBuffer) / sizeof (TCHAR));
#else
				_wcstombsz (szBuffer, pString, sizeof (szBuffer) / sizeof (TCHAR));
#endif

				// Insert group name into the list control:
				m_pEnumList->InsertItem (m_pEnumList->GetItemCount (), szBuffer);

				// Free enumerator allocations:
				if (pString)
					CoTaskMemFree (pString);
				}

			// Now release our enumerator:
			pEnum->Release ();
			}
		}

	// Select the first item in the list:
	m_pEnumList->SetItemState (0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	// Force the list control to scroll if necessary to show 
	// selected item:
	m_pEnumList->EnsureVisible (0, TRUE);
	}

// **************************************************************************
// UpdateState ()
//
// Description:
//	Get state of OPC group and update controls accordingly.
//
// Parameters:
//  CString		&strName	Name of OPC Group.
//
// Returns:
//  void
// **************************************************************************
void CKServerEnumerateGroupsDlg::UpdateState (CString &strName) 
	{
	WCHAR *pszName;
	IOPCGroupStateMgt *pIGroupStateMgt;
	HRESULT hr = E_FAIL;

	// Allocate memory for name string:
	pszName = (WCHAR *) CoTaskMemAlloc ((strName.GetLength () + 1) * sizeof (WCHAR));

	// Copy name string into allocated memory.  COM requires all strings to 
	// be in UNICODE format.  Convert if this is an ANSI build:
#ifdef _UNICODE
	lstrcpyn (pszName, strName, strName.GetLength () + 1);
#else
	_mbstowcsz (pszName, strName, strName.GetLength () + 1);
#endif

	// Get group by name:
	hr = m_pIServer->GetGroupByName (
		pszName, IID_IOPCGroupStateMgt, (IUnknown**)&pIGroupStateMgt);

	// We are done with memory allocated for name string, so free it:
	CoTaskMemFree (pszName);

	// If request was successful, process results:
	if (SUCCEEDED (hr))
		{
		// Declare variables neede to get group state:
		OPCHANDLE hClient;
		OPCHANDLE hServer;

		// Get group state:
		hr = pIGroupStateMgt->GetState (
			&m_dwUpdateRate,
			&m_bActive,
			&pszName, 
			&m_lBias,
			&m_fDeadband,
			&m_dwLanguageID,
			&hClient,
			&hServer);

		// If succeeded, process results:
		if (SUCCEEDED (hr))
			{
			// Update group name:
			m_strName = strName;

			// Free memory server allocated for name string:
			if (pszName)
				CoTaskMemFree (pszName);
			}

		// Release our get group by name reference:
		pIGroupStateMgt->Release ();
		}

	// Update dilaog controls:
	UpdateData (false);
	}
