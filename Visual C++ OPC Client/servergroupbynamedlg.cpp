// **************************************************************************
// servergroupbynamedlg.cpp
//
// Description:
//	Implements a dialog class for enumerating OPC groups by name.
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
#include "servergroupbynamedlg.h"


/////////////////////////////////////////////////////////////////////////////
// CKServerGroupByNameDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerGroupByNameDlg, CDialog)
	//{{AFX_MSG_MAP(CKServerGroupByNameDlg)
	ON_BN_CLICKED (IDC_APPLY, OnApply)
	ON_EN_CHANGE (IDC_NAME, OnChangeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerGroupByNameDlg ()
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
CKServerGroupByNameDlg::CKServerGroupByNameDlg (IOPCServer *pIServer, CWnd *pParent /*=NULL*/)
	: CDialog (CKServerGroupByNameDlg::IDD, pParent)
	{
	// Initialize member variables:
	ASSERT (pIServer != NULL);
	m_pIServer = pIServer;

	//{{AFX_DATA_INIT(CKServerGroupByNameDlg)
	m_bActive = FALSE;
	m_dwLanguageID = 0;
	m_strName = _T("");
	m_fDeadband = 0.0f;
	m_lBias = 0;
	m_dwUpdateRate = 0;
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
void CKServerGroupByNameDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member varibles:
	//{{AFX_DATA_MAP(CKServerGroupByNameDlg)
	DDX_Check (pDX, IDC_ACTIVE, m_bActive);
	DDX_Text (pDX, IDC_LANGUAGEID, m_dwLanguageID);
	DDX_Text (pDX, IDC_NAME, m_strName);
	DDX_Text (pDX, IDC_PERCENTDEADBAND, m_fDeadband);
	DDX_Text (pDX, IDC_TIMEBIAS, m_lBias);
	DDX_Text (pDX, IDC_UPDATERATE, m_dwUpdateRate);
	//}}AFX_DATA_MAP
	}


/////////////////////////////////////////////////////////////////////////////
// CKServerGroupByNameDlg message handlers
/////////////////////////////////////////////////////////////////////////////

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
void CKServerGroupByNameDlg::OnApply () 
	{
	// Declare some variables needed for get group by name request:
	HRESULT hr;
	WCHAR szBuffer [DEFBUFFSIZE];
	IOPCGroupStateMgt *pIGroupStateMgt;

	// Update member variables associated with controls:
	UpdateData (true);

	// Copy group name to buffer.  COM requires all strings be in UNICODE
	// format, so convert if this is an ANSI build:
#ifdef _UNICODE
	lstrcpyn (szBuffer, m_strName, sizeof (szBuffer) / sizeof (WCHAR));
#else
	_mbstowcsz (szBuffer, m_strName, sizeof (szBuffer) / sizeof (WCHAR));
#endif

	// Attempt to get the group by name:
	hr = m_pIServer->GetGroupByName (szBuffer, IID_IOPCGroupStateMgt, (IUnknown**)&pIGroupStateMgt);

	// If succeeded, get group state:
	if (SUCCEEDED (hr))
		{
		// Declare vairables for get group state request:
		OPCHANDLE hClient;
		OPCHANDLE hServer;
		LPWSTR pszName;

		// Request group state:
		hr = pIGroupStateMgt->GetState (
			&m_dwUpdateRate,
			&m_bActive,
			&pszName,
			&m_lBias,
			&m_fDeadband,
			&m_dwLanguageID,
			&hClient,
			&hServer);

		// COM requires us to free memory allocated by server for the 
		// name string:
		if (pszName)
			CoTaskMemFree (pszName);

		// Release this reference:
		pIGroupStateMgt->Release ();
		}

	// On failure (get group or get group state request), set appropriate
	// defaults:
	if (FAILED (hr))
		{
		m_bActive = FALSE;
		m_dwLanguageID = 0;
		m_strName = _T("");
		m_fDeadband = 0.0f;
		m_lBias = 0;
		m_dwUpdateRate = 0;
		}

	// Update controls:
	UpdateData (false);
	}

// **************************************************************************
// OnChangeName ()
//
// Description:
//	Handles notification that contents of Name edit box has changed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServerGroupByNameDlg::OnChangeName () 
	{
	// Enable the apply button based on the current name state (if empty
	// name string, then disable):
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (
		((CEdit *) GetDlgItem (IDC_NAME))->GetWindowTextLength ());
	}
