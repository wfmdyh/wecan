// **************************************************************************
// globals.cpp
//
// Description:
//	This is where global variables are initialized and global functions are
//	implemented.
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

// Clipboard formats for data access 1.0 streams:
UINT CF_DATA_CHANGE = RegisterClipboardFormat (_T("OPCSTMFORMATDATA"));
UINT CF_DATA_CHANGE_TIME	= RegisterClipboardFormat (_T("OPCSTMFORMATDATATIME"));
UINT CF_WRITE_COMPLETE = RegisterClipboardFormat (_T("OPCSTMFORMATWRITECOMPLETE"));

// Clipboard formats for cut/copy/paste:
UINT CF_SERVER = RegisterClipboardFormat (_T("QCOpcClientServer"));
UINT CF_GROUP = RegisterClipboardFormat (_T("QCOpcClientGroup"));
UINT CF_ITEM = RegisterClipboardFormat (_T("QCOpcClientItem"));

// Special error messages only used in debug builds:
#ifdef _DEBUG
	LPCTSTR _dbg_szMallocFailedMsg = _T("Memory allocation failure in module %s at line %d\n");
	LPCTSTR _dbg_szExceptionMsg = _T("Exception handler invoked in module %s at line %d\n");
#endif


// **************************************************************************
// LogMsg ()
//
// Description:
//	Log a message to our application's event window.
//
// Parameters:
//  UINT		nResID		String resource ID
//							  15000 - 15999 info
//							  16000 - 16999 error
//							Additional arguments can be strings or 
//							  numerical values to be used in message string.
//							  Place format specifiers in string resource.
//
// Returns:
//  void
// **************************************************************************
void LogMsg (UINT nResID, ...)
	{
	EVENTTYPE eType;

	// Get pointer to additional argument list (which starts after nResID):
	va_list args;
	va_start (args, nResID);

	// Allocate a buffer for message text:
	int nBuf;
	TCHAR szBuffer [512];
	
	// Load the specified string resource.  It should contain format 
	// specifiers if additional arguments are given.
	CString strFmt;
	strFmt.LoadString (nResID);

	// Contruct the message string:
	nBuf = _vsntprintf (szBuffer, sizeof (szBuffer) / sizeof (szBuffer[0]), (LPCTSTR)strFmt, args);

	// Chect to see if there was there an error, indicated by nBuf=0.  Debug only.
	// (was the expanded string too long?)
	ASSERT (nBuf >= 0);

	// Determine the type of message implied by string resource number.
	// (see range in function header above)
	if (nResID >= EVENT_FIRSTINFO && nResID <= EVENT_LASTINFO)
		eType = tEventInformation;
	else if (nResID >= EVENT_FIRSTERROR && nResID <= EVENT_LASTERROR)
		eType = tEventError;
	else
		{
		// String resource ID not in range.  Programmer error.
		ASSERT (FALSE);
		}

	// Add the message to the queue.

	// First get pointer to the application's main window:
	CKMainWnd *pMainWnd = (CKMainWnd *)AfxGetMainWnd ();

	// If pointer looks valid, call LogMsg() function to place message
	// in queue.
	if (pMainWnd && ::IsWindow (pMainWnd->m_hWnd))
		pMainWnd->LogMsg (eType, szBuffer);

	// Reset additional arguments pointer:
	va_end (args);
	}

// **************************************************************************
// UpdateItemCount ()
//
// Description:
//	Update global item count maintained in CKMainWnd object.
//
// Parameters:
//  int			nDelta		Number of items to add to count.
//
// Returns:
//  void
// **************************************************************************
void UpdateItemCount (int nDelta)
	{
	// Add the update item message to the queue.

	// First need to get pointer to application's main window:
	CKMainWnd *pMainWnd = (CKMainWnd *)AfxGetMainWnd ();

	// If pointer looks OK, call UpdateItemCount() function to
	// place message in queue.
	if (pMainWnd && ::IsWindow (pMainWnd->m_hWnd))
		pMainWnd->UpdateItemCount (nDelta);
	}

// **************************************************************************
// VartypeFromString ()
//
// Description:
//	Returns a vartype based on a string description.
//
// Parameters:
//  LPCTSTR		lpszType		Variant type (Boolean, Byte, etc)
//
// Returns:
//  VARTYPE - Type (VT_BOOL, VTU1, etc.), or VT_EMPTY if input invalid.
// **************************************************************************
VARTYPE VartypeFromString (LPCTSTR lpszType)
	{
	VARTYPE vtType;

	// Compare input type string with supported types and return the
	// corresponding variant type.  (A match is found when lstrcmpi returns
	// zero.)  If specified type is not supported, then return VT_EMPTY.
	// These strings must match those used below in StringFromVartype().
	if (lstrcmpi (lpszType, _T("Boolean")) == 0)
		vtType = VT_BOOL;
	else if (lstrcmpi (lpszType, _T("Byte")) == 0)
		vtType = VT_UI1;
	else if (lstrcmpi (lpszType, _T("Byte Array")) == 0)
		vtType = VT_UI1 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Char")) == 0)
		vtType = VT_I1;
	else if (lstrcmpi (lpszType, _T("Char Array")) == 0)
		vtType = VT_I1 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Word")) == 0)
		vtType = VT_UI2;
	else if (lstrcmpi (lpszType, _T("Word Array")) == 0)
		vtType = VT_UI2 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Short")) == 0)
		vtType = VT_I2;
	else if (lstrcmpi (lpszType, _T("Short Array")) == 0)
		vtType = VT_I2 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("DWord")) == 0)
		vtType = VT_UI4;
	else if (lstrcmpi (lpszType, _T("DWord Array")) == 0)
		vtType = VT_UI4 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Long")) == 0)
		vtType = VT_I4;
	else if (lstrcmpi (lpszType, _T("Long Array")) == 0)
		vtType = VT_I4 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Float")) == 0)
		vtType = VT_R4;
	else if (lstrcmpi (lpszType, _T("Float Array")) == 0)
		vtType = VT_R4 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("Double")) == 0)
		vtType = VT_R8;
	else if (lstrcmpi (lpszType, _T("Double Array")) == 0)
		vtType = VT_R8 | VT_ARRAY;
	else if (lstrcmpi (lpszType, _T("String")) == 0)
		vtType = VT_BSTR;
	else
		vtType = VT_EMPTY;

	// Return variant type:
	return (vtType);
	}

// **************************************************************************
// StringFromVartype ()
//
// Description:
//	Returns a string description based on a vartype.
//
// Parameters:
//	VARTYPE		vtType		Input variant type (VT_BOOL, VT_UI1, etc.)
//	CString		&strType	Output string corresponding to input type. If no
//							  type specified, then return "Native".
//
// Returns:
//	void
// **************************************************************************
void StringFromVartype (VARTYPE vtType, CString &strType)
	{
	// Return a string describing the specified variant type.  These strings
	// must match those used above in VartypeFromString().
	switch (vtType & ~VT_ARRAY)
		{
		case VT_BOOL:	strType = _T("Boolean");	break;
		case VT_UI1:	strType = _T("Byte");		break;
		case VT_I1:		strType = _T("Char");		break;
		case VT_UI2:	strType = _T("Word");		break;
		case VT_I2:		strType = _T("Short");		break;
		case VT_UI4:	strType = _T("DWord");		break;
		case VT_I4:		strType = _T("Long");		break;
		case VT_R4:		strType = _T("Float");		break;
		case VT_R8:		strType = _T("Double");		break;
		case VT_BSTR:	strType = _T("String");		break;
		default:		strType = _T("Native");		break;
		}

	// Append " Array" if a variant array type:
	if ((vtType & VT_ARRAY) != 0)
		strType += _T(" Array");
	}
