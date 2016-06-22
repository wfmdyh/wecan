// **************************************************************************
// servergeterrorstringdlg.cpp
//
// Description:
//	Implements a dialog class for retrieving a meaningful description of an
//	error code from the selected OPC server.
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
#include "servergeterrorstringdlg.h"
#include "server.h"


/////////////////////////////////////////////////////////////////////////////
// CKServerGetErrorStringDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKServerGetErrorStringDlg, CDialog)
	//{{AFX_MSG_MAP(CKServerGetErrorStringDlg)
	ON_BN_CLICKED (IDC_APPLY, OnApply)
	ON_EN_CHANGE (IDC_ERROR_CODE, OnChangeErrorCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKServerGetErrorStringDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  CKServer	*pServer	Pointer to CKServer object.
//	CWnd		*pParent	Pointer to parent window.
//
// Returns:
//  none
// **************************************************************************
CKServerGetErrorStringDlg::CKServerGetErrorStringDlg (CKServer *pServer, CWnd *pParent /*=NULL*/)
	: CDialog (CKServerGetErrorStringDlg::IDD, pParent)
	{
	// Initialize member variables:
	ASSERT (pServer != NULL);

	m_pIServer = pServer->GetIServer ();
	ASSERT (m_pIServer != NULL);

	//{{AFX_DATA_INIT(CKServerGetErrorStringDlg)
	m_strErrorCode = _T("");
	m_strErrorDescription = _T("");
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
void CKServerGetErrorStringDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data beteen controls and associated member variables:
	//{{AFX_DATA_MAP(CKServerGetErrorStringDlg)
	DDX_Text (pDX, IDC_ERROR_CODE, m_strErrorCode);
	DDV_MaxChars (pDX, m_strErrorCode, 10);
	DDX_Text (pDX, IDC_ERROR_STRING, m_strErrorDescription);
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
//	BOOL - TRUE.
// **************************************************************************
BOOL CKServerGetErrorStringDlg::OnInitDialog () 
	{
	// Perform default processing:
	CDialog::OnInitDialog ();

	// Disable the Apply button (We will enable only after changes have been
	// made.):
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (FALSE);
	
	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
	}


/////////////////////////////////////////////////////////////////////////////
// CKServerGetErrorStringDlg message handlers
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
void CKServerGetErrorStringDlg::OnApply () 
	{
	// Initialize some variables needed for request:
	HRESULT hr = E_FAIL;

	DWORD dwError = -1;
	LPWSTR pszError = NULL;

	bool bIsHex = false;
	TCHAR cChar;
	
	// Look for invalid characters in error code.  Loop over
	// all characters in error code string:
	for (int i = 0; i < m_strErrorCode.GetLength (); i++)
		{
		// Get current character:
		cChar = m_strErrorCode [i];

		// All digits are valid.  Other characters may be valid.
		if (!_istdigit (cChar))
			{
			// If first non-digit character is "x" or "X", then
			// we are probably looking at a hexidecimal value.  In
			// that case, accept the character, set the bIsHex flag,
			// and continue checking the string.
			if (!bIsHex && (cChar == _T('x') || cChar == _T('X')))
				bIsHex = true;

			// If we have seen the "X", and this character is a hex
			// digit, then it is valid.
			else if (bIsHex && _istxdigit (cChar))
				continue;

			// Else we have seen the "X", but current character is not
			// a valid hex digit.  Show message box that tells user error
			// code is invalid the return.
			else
				{
				// "Invalid hexadecimal number."
				AfxMessageBox (IDS_INVALIDHEXNUMBER);
				return;
				}
			}
		}

	// If we make it here, then error code has a valid format.
	// Convert string to number.
	if (bIsHex)
		_stscanf (m_strErrorCode, _T("%X"), &dwError);
	else
		dwError = (DWORD) _ttol (m_strErrorCode);

	// Get the error string:
	hr = m_pIServer->GetErrorString (dwError, 0, &pszError);

	// If request succeeded, process the results:
	if (SUCCEEDED (hr) && pszError != NULL)
		{
		m_strErrorDescription = pszError;
		}

	// Else if request failed, inform user of problem:
	else
		{
		CString strMsg;
		
		// "The server could not resolve this error code."
		strMsg.Format (IDS_UNABLETORESOLVEERRORCODE);
		AfxMessageBox (strMsg);		
		}

	// Free memory server allocated for error string:
	if (pszError)
		CoTaskMemFree (pszError);

	// Reformat the error code as a hex number:
	m_strErrorCode.Format (_T("0x%08X"), dwError);

	// Update the controls to show reformatted code and error string:
	UpdateData (FALSE);
	}

// **************************************************************************
// OnChangeErrorCode ()
//
// Description:
//	Handles notification that contents of Error Code edit box have changed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServerGetErrorStringDlg::OnChangeErrorCode () 
	{
	// Update member variables associated with controls:
	UpdateData (true);

	// Enable/disable apply button based on an error code:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (!m_strErrorCode.IsEmpty ());
	}
