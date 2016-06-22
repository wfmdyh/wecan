// **************************************************************************
// statusbartext.cpp
//
// Description:
//	Implements a class to contain status bar text.
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
#include "mainwnd.h"


// **************************************************************************
// CKStatusBarText ()
//
// Description:
//	Constructor.  Places specified text in status bar.
//
// Parameters:
//  LPCTSTR		lpszText	Pointer to text string.
//
// Returns:
//  none
// **************************************************************************
CKStatusBarText::CKStatusBarText (LPCTSTR lpszText)
	{
	// Get pointer to main window:
	CKMainWnd *pMainWnd = (CKMainWnd *)AfxGetMainWnd ();

	// If main window is valid, set status bar text:
	if (pMainWnd && ::IsWindow (pMainWnd->m_hWnd))
		pMainWnd->SetStatusBarText (lpszText);
	}

// **************************************************************************
// CKStatusBarText ()
//
// Description:
//	Constructor.  Places specified text in status bar.
//
// Parameters:
//  UINT		uID		Text string resource number.
//
// Returns:
//  none
// **************************************************************************
CKStatusBarText::CKStatusBarText (UINT uID)
	{
	// Get pointer to main window:
	CKMainWnd *pMainWnd = (CKMainWnd *)AfxGetMainWnd ();

	// If main window is valid, set status bar text:
	if (pMainWnd && ::IsWindow (pMainWnd->m_hWnd))
		{
		// Load text from string resource:
		CString strText;
		strText.LoadString (uID);

		pMainWnd->SetStatusBarText (strText);
		}
	}

// **************************************************************************
// ~CKStatusBarText ()
//
// Description:
//	Destructor.	 Places idle message text in status bar.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKStatusBarText::~CKStatusBarText ()
	{
	// Get pointer to main window:
	CKMainWnd *pMainWnd = (CKMainWnd *)AfxGetMainWnd ();

	// If main window is valid, set status bar text:
	if (pMainWnd && ::IsWindow (pMainWnd->m_hWnd))
		{
		// Load idle message string from string resource:
		CString strText;
		strText.LoadString (AFX_IDS_IDLEMESSAGE);

		pMainWnd->SetStatusBarText (strText);
		}
	}
