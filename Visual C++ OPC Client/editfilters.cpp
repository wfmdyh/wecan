// **************************************************************************
// editfilters.cpp
//
// Description:
//	Implements several special purpose edit box classes.
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
#include "ctype.h"
#include "editfilters.h"

// Macro returns TRUE if virtual key code is < VK_SPACE (0x20).  This includes
// VK_LBUTTON, VK_CANCEL, VK_BACK, VK_TAB, VK_CLEAR, VK_RETURN, VK_SHIFT,
// VK_CONTROL, VK_MENU, VK_CAPITAL, and VK_ESCAPE.
#define mIsSystemKey(k)	(k < VK_SPACE)


/////////////////////////////////////////////////////////////////////////////
// Base edit control filter - handles WM_CHAR and WM_PASTE
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CEditBase, CEdit)
	ON_WM_CHAR ()
	ON_MESSAGE (WM_PASTE, OnPaste)
END_MESSAGE_MAP ()


// **************************************************************************
// OnChar ()
//
// Description:
//	Override for CWnd::OnChar that allows us to process the character and 
//	beep if there is a problem.
//
// Parameters:
//	UINT		nChar		Contains the character code value of the key. 
//	UINT		nRepCnt		Contains the repeat count.
//	UINT		nFlags		Flags.
//
// Returns:
//  void
// **************************************************************************
void CEditBase::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags)
	{
	// If key is a system key, as defined by abovve macro, or one of the
	// characters accpted by derived class ProcessChar(), then accept the
	// character by calling default processor and returning.
	if (mIsSystemKey (nChar) || ProcessChar (nChar))
		{
		// Default processing:
		CEdit::OnChar (nChar, nRepCnt, nFlags);
		
		// If this was not a system key, then it was an acceptable character.
		// Clear empty indicator now that a valid char has been recieved:
		if (!mIsSystemKey (nChar))
			m_bEmpty = false;

		// Return now to prevent beep:
		return;
		}

	// If we make it here, then we don't like the character entered.
	// Sound a beep to let user user know.
	MessageBeep (-1);
	}

// **************************************************************************
// OnPaste ()
//
// Description:
//	Allows us to process each character in a paste operation.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
long CEditBase::OnPaste (WPARAM /*wNotUsed*/, LPARAM /*lNotUsed*/)
	{
	// Define string format used by build type (UNICODE or ANSI):
#ifdef _UNICODE
	UINT uFmt = CF_UNICODETEXT;
#else
	UINT uFmt = CF_TEXT;
#endif

	// See if we can get text from clipboard.  If we can't, then sound a beep
	// to let user know there was a problem and return:
	if (!IsClipboardFormatAvailable (uFmt))
		{
		MessageBeep (-1);
		return (0);
		}

	// If we fail to open the clipboard, return:
	if (!OpenClipboard ())
		return (0);

	// If we make it here, clipboard content looks OK and we were able
	// to open it.  Make a local copy of the text in the clipboard.

	// First need to get the handle to clipboard data:
	HANDLE hText = GetClipboardData (uFmt);

	// Next lock the clipboard to prevent any other thread from 
	// overwritting the data while we are copying it.  Copy the
	// data to our local container (strCopy):
	CString strCopy ((LPCTSTR)GlobalLock (hText));

	// Now we are done with the clipboard, so unlock and close:
	GlobalUnlock (hText);
	CloseClipboard ();

	// Validate each char and if OK let the default processing handle
	// the paste.  Get pointer to first character:
	LPCTSTR lpText = strCopy;
	
	// Loop over characters until we hit the NULL terminator:
	do
		{
		// Let derived class ProcessChar() function check the character.
		// If it doesn't like it, sound a beep and return.
		if (!ProcessChar (*lpText))
			{
			MessageBeep (-1);
			return (0);
			}
		}
	while (*++lpText);

	// If we make it here, then all of the characters look OK.  Perform
	// default processing to do the actual paste to control:
	return (Default ());
	}


/////////////////////////////////////////////////////////////////////////////
// Name name edit control filter
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// ProcessChar ()
//
// Description:
//	Name edit control char filter.
//
// Parameters:
//	UINT		nChar		Character code.
//
// Returns:
//  BOOL - FALSE if character is invalid.
// **************************************************************************
BOOL CNameEdit::ProcessChar (UINT nChar)
	{
	// Valid characters are alpha numerics and underscore:
	return (_istalnum (nChar) || nChar == '_');
	}


/////////////////////////////////////////////////////////////////////////////
// Numeric name edit control filter
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CNumericEdit, CEditBase)
	ON_MESSAGE (WM_GETTEXT, OnGetText)
	ON_MESSAGE (WM_SETTEXT, OnSetText)
END_MESSAGE_MAP ()


// **************************************************************************
// ProcessChar ()
//
// Description:
//	Numeric edit control char filter.
//
// Parameters:
//	UINT		nChar		Character code.
//
// Returns:
//  BOOL - FALSE if character is invalid.
// **************************************************************************
BOOL CNumericEdit::ProcessChar (UINT nChar)
	{
	// Valid characters for unsigned decimal format are all digits:
	if (m_eFormat == tUnsignedDecimal)
		return (_istdigit (nChar));

	// Valid characters for signed decimal format are all digits and
	// minus sign:
	if (m_eFormat == tSignedDecimal)
		return (_istdigit (nChar) || nChar == _T('-'));

	// Valid characters for octal format are digits between 0 and 7
	// inclusive:
	if (m_eFormat == tOctal)
		return (nChar >= _T('0') && nChar <= _T('7'));

	// Valid characters for hex format are all hex digits:
	if (m_eFormat == tHex)
		return (_istxdigit (nChar));

	// If we make it here, then format is invalid (programmer error).
	ASSERT (FALSE);
	return (false);
	}

// **************************************************************************
// OnGetText ()
//
// Description:
//	Get text of numeric edit control.
//
// Parameters:
//	WPARAM		wParam		Number of characters to get.
//	LPARAM		lParam		Pointer to output string buffer.
//
// Returns:
//  LRESULT - String length if internal processing (m_bInternal == true).
// **************************************************************************
LRESULT CNumericEdit::OnGetText (WPARAM wParam, LPARAM lParam)
	{
	// Always return the text in decimal format (unless this is the initial call)
	if (!m_bInternal && !m_bEmpty)
		{
		// Format the string in decimal format:
		TCHAR szBuff [32];
		_stprintf (szBuff, _T("%u"), GetValue ());

		// Copy the string to lParam:
		lstrcpyn ((LPTSTR)lParam, szBuff, wParam);

		// Return string length:
		return (lstrlen ((LPCTSTR)lParam));
		}

	// If we make it here, then this is an internal call.  If so, do
	// default processing:
	return (Default ());
	}

// **************************************************************************
// OnSetText ()
//
// Description:
//	Set text of numeric edit control.
//
// Parameters:
//  LPARAM		lParam		Pointer to input string buffer.
//
// Returns:
//  LRESULT - true if success if internal processing (m_bInternal == true).
// **************************************************************************
LRESULT CNumericEdit::OnSetText (WPARAM /*wParam*/, LPARAM lParam)
	{
	// Set the value (unless this is an internal call):
	if (!m_bInternal)
		{
		// We will assume text is formatted in decimal:
		SetValue (_ttol ((LPCTSTR)lParam));

		// Return true to indicate success:
		return (true);
		}

	// Control is no longer empty
	m_bEmpty = false;

	// If we make it here, then this is an internal call.  If so, do
	// default processing:
	return (Default ());
	}

// **************************************************************************
// SetValue ()
//
// Description:
//	Set value of nummeric edit control.
//
// Parameters:
//  DWORD		dwVal		Value to set.
//
// Returns:
//  void
// **************************************************************************
void CNumericEdit::SetValue (DWORD dwVal)
	{
	TCHAR szBuf [32];
	TCHAR szOldText [32];

	// Convert value to string.  Format as defined by m_eFormat:
	switch (m_eFormat)
		{
		case tSignedDecimal:
			_stprintf (szBuf, _T("%d"), dwVal);
			break;

		case tUnsignedDecimal:
			_stprintf (szBuf, _T("%u"), dwVal);
			break;

		case tOctal:
			_stprintf (szBuf, _T("%o"), dwVal);
			break;

		case tHex:
			_stprintf (szBuf, _T("%X"), dwVal);
			break;

		default:
			// Invalid format.  Programmer error.  Create NULL string:
			ASSERT (FALSE);
			szBuf [0] = 0;
			break;
		}

	// Place the string we just created in the window.

	// First set the internal flag so we can get text without translation:
	m_bInternal = true;
	
	// Get the current text:
	GetWindowText (szOldText, _countof (szOldText));

	// If the new text is different, then update the control:
	if (lstrcmp (szOldText, szBuf))
		SetWindowText (szBuf);

	// Clear the internal flag:
	m_bInternal = false;
	}

// **************************************************************************
// GetValue ()
//
// Description:
//	Get value of numeric edit control.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - Current value.
// **************************************************************************
DWORD CNumericEdit::GetValue ()
	{
	CString str;

	// Get the current window text.

	// First set the internal flag so we can get text without translation:
	m_bInternal = true;

	// Get the text:
	GetWindowText (str);
	
	// Clear the internal flag:
	m_bInternal = false;

	// Initialize the output value.  This will be the value returned
	// in case of bad format:
	DWORD dwVal = 0;

	// Set output value.  Interpret text according to format specified
	// by m_eFormat:
	switch (m_eFormat)
		{
		case tSignedDecimal:
		case tUnsignedDecimal:
			dwVal = _ttol (str);
			break;

		case tOctal:
			_stscanf (str, _T("%o"), &dwVal);
			break;

		case tHex:
			_stscanf (str, _T("%X"), &dwVal);
			break;

		default:
			// Unexpected format type.  Programmer error.
			ASSERT (FALSE);
			break;
		}

	// Return the value:
	return (dwVal);
	}

// **************************************************************************
// SetFormat ()
//
// Description:
//	Set format of numeric edit control.
//
// Parameters:
//  INTEGERFORMAT	eFormat		Format (tSignedDecimal, tUnsignedDecimal,
//								  tOctal, tHex).
//
// Returns:
//  void
// **************************************************************************
void CNumericEdit::SetFormat (INTEGERFORMAT eFormat)
	{
	// If format hasn't changed, don't need to do anything:
	if (m_eFormat == eFormat)
		return;

	// If we make it here, the format has changed.  We therefore need to
	// reformat the text currently displyed.

	// Get the current value (using old format):
	DWORD dwVal = GetValue ();

	// Reset the format:
	m_eFormat = eFormat;

	// Reset the value displayed (using new format):
	SetValue (dwVal);
	}


/////////////////////////////////////////////////////////////////////////////
// Real number edit control filter
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// ProcessChar ()
//
// Description:
//	Numeric edit control char filter.
//
// Parameters:
//	UINT		nChar		Character code.
//
// Returns:
//  BOOL - TRUE if character is valid.
// **************************************************************************
BOOL CRealNumEdit::ProcessChar (UINT nChar)
	{
	// Valid characters are all digits, decimal point, exponent E or e,
	// and minus sign:
	return (_istdigit (nChar) || nChar == _T('.') || nChar == _T('E') || nChar == _T('e') || nChar == _T('-'));
	}


/////////////////////////////////////////////////////////////////////////////
// File name edit control filter
/////////////////////////////////////////////////////////////////////////////

// Valid but unusual file characters:
LPCTSTR CFileNameEdit::sm_szFileChars = _T(" _!#$%^()'&@}{~-");

// **************************************************************************
// ProcessChar ()
//
// Description:
//	File name edit control char filter.
//
// Parameters:
//	UINT		nChar		Character code.
//
// Returns:
//  BOOL - TRUE if character is valid.
// **************************************************************************
BOOL CFileNameEdit::ProcessChar (UINT nChar)
	{
	// Valid characters are alphanumerics and other unusual file name 
	// characters defined above:
	return (_istalnum (nChar) || _tcschr (sm_szFileChars, nChar));
	}

