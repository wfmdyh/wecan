// **************************************************************************
// updateintervaldlg.cpp
//
// Description:
//	Implements a dialog class for assigning view update interval.  Changes
//	can be obtained from public member variable m_nInterval.
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
#include "updateintervaldlg.h"


/////////////////////////////////////////////////////////////////////////////
// CKUpdateIntervalDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKUpdateIntervalDlg, CDialog)
	//{{AFX_MSG_MAP(CKUpdateIntervalDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKUpdateIntervalDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  WORD		wInterval	Current view update interval.
//	CWnd		*pParent	Pointer to parent window.
//
// Returns:
//  none
// **************************************************************************
CKUpdateIntervalDlg::CKUpdateIntervalDlg (WORD wInterval, CWnd *pParent /*=NULL*/)
	: CDialog (CKUpdateIntervalDlg::IDD, pParent)
	{
	// Initialize member variables:

	//{{AFX_DATA_INIT(CKUpdateIntervalDlg)
	m_nInterval = wInterval;
	//}}AFX_DATA_INIT

	// Make sure input interval is valid.  (Note same limits are set for 
	// m_nInterval using class wizard.  See DoDataExchange().)
	if (m_nInterval < VIEW_MIN_INTERVAL)
		m_nInterval = VIEW_MIN_INTERVAL;
	else if (m_nInterval > VIEW_MAX_INTERVAL)
		m_nInterval = VIEW_MAX_INTERVAL;
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
void CKUpdateIntervalDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKUpdateIntervalDlg)
	DDX_Text (pDX, IDC_INTERVAL, m_nInterval);
	DDV_MinMaxInt (pDX, m_nInterval, 10, 30000);
	//}}AFX_DATA_MAP
	}


/////////////////////////////////////////////////////////////////////////////
// CKUpdateIntervalDlg message handlers
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
BOOL CKUpdateIntervalDlg::OnInitDialog () 
	{
	// Perform default processing:
	CDialog::OnInitDialog ();
	
	// Limit the amount of text that can be typed in interval edit control:
	((CEdit *) GetDlgItem (IDC_INTERVAL))->SetLimitText (5);
	
	return (TRUE);
	}
