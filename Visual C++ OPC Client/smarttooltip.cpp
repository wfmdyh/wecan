// **************************************************************************
// smarttooltip.cpp
//
// Description:
//	Wrapper for CToolTipCtrl that allows it to be used with a child window
//	on a dialog box.  This tool requires access to mouse messages for windows
//	of this type and we would have to subclass each control to pass the mouse
//	messages on.
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
#include "smarttooltip.h"


// **************************************************************************
// AddWindowTool ()
//
// Description:
//	Registers a tool with a ToolTip control.  Text will be obtained from 
//	the window itself.
//
// Parameters:
//	CWnd		*pWnd		Pointer to window we will be associated with.
//
// Returns:
//  BOOL - TRUE if success
// **************************************************************************
BOOL CSmartToolTip::AddWindowTool (CWnd *pWnd)
	{
	TOOLINFO ti;
	CString strText;

	// Obtain the tool tip text from the window itself (good for buttons):
	pWnd->GetWindowText (strText);
	
	// Trim off appended text for hot key:
	int nExtra = strText.Find (_T('\n'));
	
	if (nExtra > 0)
		strText = strText.Left (nExtra);

	// Fill in the tool info structure:
	// (Set flags to indicates that the uId member is the window handle
	// to the tool, and to indicate that the ToolTip control should subclass
	// the tool's window to intercept messages.)
	ti.cbSize = sizeof (ti);						// size of this structure
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;		// flags
	ti.hwnd = pWnd->GetParent ()->GetSafeHwnd ();	// handle to the window that contains the tool
	ti.uId = (UINT)pWnd->GetSafeHwnd ();			// application-defined identifier of the tool
	ti.hinst = AfxGetResourceHandle ();				// handle to the instance that contains the string resource for the tool
	ti.lpszText = (LPTSTR)(LPCTSTR)strText;			// pointer to the buffer that contains the text for the tool

	// Send add tool message and return result:
	return ((BOOL)SendMessage (TTM_ADDTOOL, 0, (LPARAM)&ti));
	}

// **************************************************************************
// AddWindowTool ()
//
// Description:
//	Registers a tool with a ToolTip control.  Text will be obtained from 
//	specified string resource.
//
// Parameters:
//	CWnd		*pWnd		Pointer to window we will be associated with.
//	int			nStringID	String resource number.
//
// Returns:
//  BOOL - TRUE if success
// **************************************************************************
BOOL CSmartToolTip::AddWindowTool (CWnd *pWnd, int nStringID)
	{
	TOOLINFO ti;

	// Fill tool tip info structure:
	// (Set flags to indicates that the uId member is the window handle
	// to the tool, and to indicate that the ToolTip control should subclass
	// the tool's window to intercept messages.)
	ti.cbSize = sizeof (ti);						// size of this structure
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;		// flags
	ti.hwnd = pWnd->GetParent ()->GetSafeHwnd ();	// handle to the window that contains the tool
	ti.uId = (UINT)pWnd->GetSafeHwnd ();			// application-defined identifier of the tool
	ti.hinst = AfxGetResourceHandle ();				// handle to the instance that contains the string resource for the tool
	ti.lpszText = MAKEINTRESOURCE (nStringID);		// pointer to the buffer that contains the text for the tool

	// Send add tool message and return result:
	return ((BOOL)SendMessage (TTM_ADDTOOL, 0, (LPARAM)&ti));
	}

// **************************************************************************
// AddWindowTool ()
//
// Description:
//	Registers a tool with a ToolTip control.  Text is passed in as a parameter.
//
// Parameters:
//	CWnd		*pWnd		Pointer to window we will be associated with.
//	LPCTSTR		lpsz		Pointer to text string.
//
// Returns:
//  BOOL - TRUE if success
// **************************************************************************
BOOL CSmartToolTip::AddWindowTool (CWnd *pWnd, LPCTSTR lpsz)
	{
	TOOLINFO ti;

	// Fill tool tip info structure:
	// (Set flags to indicates that the uId member is the window handle
	// to the tool, and to indicate that the ToolTip control should subclass
	// the tool's window to intercept messages.)
	ti.cbSize = sizeof (ti);						// size of this structure
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;		// flags
	ti.hwnd = pWnd->GetParent ()->GetSafeHwnd ();	// handle to the window that contains the tool
	ti.uId = (UINT)pWnd->GetSafeHwnd ();			// application-defined identifier of the tool
	ti.hinst = AfxGetResourceHandle ();				// handle to the instance that contains the string resource for the tool
	ti.lpszText = (LPTSTR)lpsz;						// pointer to the buffer that contains the text for the tool

	// Send add tool message and return result:
	return ((BOOL)SendMessage (TTM_ADDTOOL, 0, (LPARAM)&ti));
	}
