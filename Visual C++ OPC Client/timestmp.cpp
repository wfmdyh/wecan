// **************************************************************************
// timestmp.cpp
//
// Description:
//	Implements a class to contain time stamp data.
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
#include "timestmp.h"

// Define number of day in each month:
static unsigned short anDaysPerMonth [] =
	{
	31,	// January
	28,	// February (except leap year)
	31,	// March
	30,	// April
	31,	// May
	30,	// June
	31,	// July
	31,	// August
	30,	// September
	31,	// October
	30,	// November
	31	// December
	};

// Private static member initialization:
INTLDATEFORMAT CTimeStamp::stDateFmt;
INTLTIMEFORMAT CTimeStamp::stTimeFmt;
HWND CTimeStamp::sm_hwndNotify = NULL;
CCriticalSection CTimeStamp::sm_cs;


// **************************************************************************
// NotifyWndProc ()
//
// Description:
//	Hidden window proc for window created in GlobalInit ().  Used to to keep
//	us infored of date and time format changes.
//
// Parameters:
//  HWND		hWnd		Handle to the window procedure that received the 
//							  message. 
//	UINT		msg			Specifies the message. 
//	WPARAM		wParam		Specifies additional message information. The 
//							  content of this parameter depends on the value
//							  of the Msg parameter. 
//	LPARAM		lParam		Specifies additional message information. The 
//							  content of this parameter depends on the value
//							  of the Msg parameter. 
//
// Returns:
//  long - The return value is the result of the message processing and 
//  depends on the message. 
// **************************************************************************
long WINAPI CTimeStamp::NotifyWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	// Processes WM_WININICHANGE (application ini file change) and WM_SETTINGCHANGE
	// (system setting change) messages:
	if (msg == WM_WININICHANGE || msg == WM_SETTINGCHANGE)
		{
		// Reset the date time format:
		SetDateTimeFormat ();

		// Force a repaint:
		AfxGetMainWnd ()->Invalidate ();
		}

	// Perform default processing and return result:
	return (DefWindowProc (hWnd, msg, wParam, lParam));
	}

// **************************************************************************
// GlobalInit ()
//
// Description:
//	This function must be called once by the application using this class
//	so that a hidden window may be be created to recieve system change
//	notifications.  These notifications will trigger an update of the
//	internal settings used when formatting dates and times.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::GlobalInit ()
	{
	// Create our settings change notification window if it hasn't already
	// been created (indicated by NULL handle):
	if (!sm_hwndNotify)
		{
		WNDCLASS wc;
		
		// Get handle to current application instance (needed to define
		// hidden window):
		HINSTANCE hInst = AfxGetInstanceHandle ();
		ASSERT (hInst);

		// Define hidden window class:
		wc.style         = 0;		// style
		wc.lpfnWndProc   = CTimeStamp::NotifyWndProc;	// pointer to window procedure
		wc.cbClsExtra    = 0;		// number of extra bytes to allocate following the window-class structure
		wc.cbWndExtra    = 0;		// number of extra bytes to allocate following the window instance
		wc.hIcon         = NULL;	// handle to the class icon
		wc.hCursor       = NULL;	// handle to the class cursor
		wc.hInstance     = hInst;	// handle to the instance that contains the window procedure for the class
		wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);	// handle to the class background brush
		wc.lpszMenuName  = 0;					// resource name of menu resource
		wc.lpszClassName = _T("__TCN_CLASS__"); // window class name

		// Register our hidden window class:
		RegisterClass (&wc);

		// Create a hidden window:
		sm_hwndNotify = ::CreateWindow (
			wc.lpszClassName,	// registered class name
			_T(""),				// window name
			0,					// window style
			0, 0,				// horizontal, vertical position of window
			0, 0,				// window width, height
			NULL,				// handle to parent or owner window
			NULL,				// menu handle or child identifier
			hInst,				// handle to application instance
			NULL);				// window-creation data

		ASSERT (sm_hwndNotify);
		TRACE (_T("Time change notification window created (HWND == %08X)\n"), sm_hwndNotify);

		// Initialize the statics:
		SetDateTimeFormat ();
		}
	}

// **************************************************************************
// CTimeStamp ()
//
// Description:
//	Constructor.
//
// Parameters:
//  SYSTEMTIME	&st		Structure containing the initial time and date.	
//
// Returns:
//  none
// **************************************************************************
CTimeStamp::CTimeStamp (SYSTEMTIME &st)
	{
	// Test input data (for debugging only):
	ASSERT (st.wMonth >= 1 && st.wMonth <= 12);
	ASSERT (st.wHour >= 0 && st.wHour <= 23);
	ASSERT (st.wMinute >= 0 && st.wMinute <= 59);
	ASSERT (st.wSecond >= 0 && st.wSecond <= 59);
	ASSERT (st.wMilliseconds >= 0 && st.wMilliseconds <= 999);

	// Save input data:
	m_st = st;

	// Convert date to day of year:
	ConvertDateToDays ();

	// Convert time to milliseconds since midnight:
	ConvertTimeToMS ();
	}

// **************************************************************************
// Assign ()
//
// Description:
//	Assigns time and date specified in st, and performs date and time 
//	conversion if specified by bInitialize.
//
// Parameters:
//	SYSTEMTIME	&st			Structure containing the time and data.
//	bool		bInitialize	Set to true to initialize member variables.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::Assign (SYSTEMTIME &st, bool bInitialize)
	{
	// Test input data (for debugging only):
	ASSERT (st.wMonth >= 1 && st.wMonth <= 12);
	ASSERT (st.wHour >= 0 && st.wHour <= 23);
	ASSERT (st.wMinute >= 0 && st.wMinute <= 59);
	ASSERT (st.wSecond >= 0 && st.wSecond <= 59);
	ASSERT (st.wMilliseconds >= 0 && st.wMilliseconds <= 999);

	// Save input data:
	m_st = st;

	// Initialize (convert date and time) if requested:
	if (bInitialize)
		{
		ConvertDateToDays ();
		ConvertTimeToMS ();
		}
	}

// **************************************************************************
// Assign ()
//
// Description:
//	Assigns current local time and date, and performs date and time 
//	conversion.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::Assign ()
	{
	// Set member variables for local time:
	GetLocalTime (&m_st);

	// Convert date to day of year:
	ConvertDateToDays ();

	// Convert time to milliseconds since midnight:
	ConvertTimeToMS ();
	}

// **************************************************************************
// SetTime ()
//
// Description:
//	Set the time.
//
// Parameters:
//	LPCTSTR		lpTime		Pointer to string representation of time.  Time
//							  must be in hh:mm:ss AM/PM format.  Delimiter
//							  could be that specified in system time format
//							  instead of ":".
//	bool		bInitialize	Set to true to initialize member variables.
//
// Returns:
//  bool - true if success
// **************************************************************************
bool CTimeStamp::SetTime (LPCTSTR lpTime, bool bInitialize)
	{
	SYSTEMTIME tm;
	int nState = 1;
	TCHAR ch;
	TCHAR chSep;
	BOOL bPM = FALSE;

	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&sm_cs);

	// Formatting info must be initialized:
	if (!stTimeFmt.nFmtLen)
		SetDateTimeFormat ();

	// Determine the selected delimiter:
	chSep = *stTimeFmt.szSeparator;

	// Initialize seconds in case they are not specified:
	tm.wSecond = 0;

	// Parse lpTime string using state machine approach.  (What we see
	// now determines the state.  The state determines what we expect
	// to see next).  Initial state set to 1.
	// (Loop until we get NULL terminator or nState set to -1 which
	// flags an invalid time string.)
	while (*lpTime && nState != -1)
		{
		// Save off current character, then increment pointer so we get next
		// character in string next time around loop:
		ch = *lpTime++;

		// Skip all whitespace:
		if (_istspace (ch))
			continue;

		// Process character according to state:
		switch (nState)
			{
			// Looking for white space or first hour digit:
			case 1:
				// If character is a digit, we will assume it is the first
				// digit of the hour.  Save as hour and set state to 2:
				if (_istdigit (ch))
					{
					tm.wHour = ch - _T('0'); // Convert from ASCII
					++nState;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for second hour digit or separator:
			case 2:
				// If characer is a digit, we will assume it is the second
				// digit of the hour.  Update hour and set state to 3:
				if (_istdigit (ch))
					{
					tm.wHour *= 10; // First digit parsed becomes second digit
					tm.wHour += (ch - _T('0')); // Convert from ASCII
					++nState;
					}

				// If character is a separator, set state to 4:
				else if (ch == chSep || ch == _T(':'))
					nState = 4;

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for a separator:
			case 3:
				// If character is a separator, set state to 4:
				if (ch == chSep || ch == _T(':'))
					nState = 4;

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for first minute digit:
			case 4:
				// If character is a digit, we will assume it is the first
				// digit of minute.  Save as minute and set state to 5:
				if (_istdigit (ch))
					{
					tm.wMinute = ch - _T('0'); // Convert from ASCII
					++nState;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for second minute digit, separator or first character
			// of "AM" or "PM":
			case 5:
				// If characer is a digit, we will assume it is the second
				// digit of the minute.  Update minute and set state to 6:
				if (_istdigit (ch))
					{
					tm.wMinute *= 10; // First digit parsed becomes second digit
					tm.wMinute += (ch - _T('0')); // Convert from ASCII
					++nState;
					}

				// Else if character is a separator, set state to 7:
				else if (ch == chSep || ch == _T(':'))
					nState = 7;

				// Else if character is an "a" or "A", set state to 11:
				else if (ch == _T('a') || ch == _T('A'))
					nState = 11;

				// Else if character is a "p" or "P", set state to 11:
				else if (ch == _T('p') || ch == _T('P'))
					{
					nState = 11;
					bPM = TRUE;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for a separator or first character of "AM" or PM":
			case 6:
				// If character is a separator, set state to 7:
				if (ch == chSep || ch == _T(':'))
					nState = 7;

				// Else if character is an "a" or "A", set state to 11:
				else if (ch == _T('a') || ch == _T('A'))
					nState = 11;

				// Else if character is a "p" or "P", set state to 11:
				else if (ch == _T('p') || ch == _T('P'))
					{
					nState = 11;
					bPM = TRUE;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for first second digit:
			case 7:
				// If character is a digit, we will assume it is the first
				// digit of second.  Save as second and set state to 8:
				if (_istdigit (ch))
					{
					tm.wSecond = ch - _T('0'); // Convert from ASCII
					++nState;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for a digit, A, P or white space
			case 8:
				// If characer is a digit, we will assume it is the second
				// digit of the second.  Update second and set state to 9:
				if (_istdigit (ch))
					{
					tm.wSecond *= 10; // First digit parsed becomes second digit
					tm.wSecond += (ch - _T('0')); // Convert from ASCII
					++nState;
					}

				// Else if character is an "a" or "A", set state to 11:
				else if (ch == _T('a') || ch == _T('A'))
					nState = 11;

				// Else if character is a "p" or "P", set state to 11:
				else if (ch == _T('p') || ch == _T('P'))
					{
					nState = 11;
					bPM = TRUE;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for first character of "AM" or "PM":
			case 9:
				// Else if character is an "a" or "A", set state to 11:
				if (ch == _T('a') || ch == _T('A'))
					nState = 11;

				// Else if character is a "p" or "P", set state to 11:
				else if (ch == _T('p') || ch == _T('P'))
					{
					nState = 11;
					bPM = TRUE;
					}

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Looking for "M":
			case 11:
				// If character is "m" or "M", set state to 12:
				if (ch == _T('m') || ch == _T('M'))
					nState = 12;

				// Else character is invalid.  Set state to -1 to break out
				// of loop:
				else
					nState = -1;

				break;

			// Only white spaces at this point.  If we hit here, then
			// character must not be a white space, so set state to -1
			// to break out of loop:
			case 12:
				nState = -1;

				break;
			}
		}

	// Check for invalid stop state:
	if (nState < 5 || nState == 7)
		{
		nState = -1;
		goto ErrorExit;
		}

	// Check for valid 24 hour format:
	if (nState < 11)
		{
		if (tm.wHour > 23 || tm.wMinute > 59 || tm.wSecond > 59)
			{
			nState = -1;
			goto ErrorExit;
			}
		}

	// Check for valid 12 hour format:
	else
		{
		if (tm.wHour < 1 || tm.wHour > 12 || tm.wMinute > 59 || tm.wSecond > 59)
			{
			nState = -1;
			goto ErrorExit;
			}

		// Adjust the hour for PM if less than 12:
		if (bPM && tm.wHour < 12)
			tm.wHour += 12;

		// Adjust for midnight:
		else if (!bPM && tm.wHour == 12)
			tm.wHour = 0;
		}

	// If we make it here time string was valid, so save it:
	m_st.wHour = tm.wHour;
	m_st.wMinute = tm.wMinute;
	m_st.wSecond = tm.wSecond;
	m_st.wMilliseconds = 0;

	// If the init flag is set then convert time to milliseconds from 
	// midnight:
	if (bInitialize)
		ConvertTimeToMS ();

ErrorExit:
	// Return true of time string was valid (nState not -1):
	return (nState != -1);
	}

// **************************************************************************
// SetDate ()
//
// Description:
//	Set the date.
//
// Parameters:
///	LPCTSTR		lpDate		Pointer to string representation of date.  Date
//							  format is obtained from system.
//	bool		bInitialize	Set to true to initialize member variables.
//
// Returns:
//  bool - true if success
// **************************************************************************
bool CTimeStamp::SetDate (LPCTSTR lpDate, bool bInitialize)
	{
	TCHAR szFmt [32];
	int nRetVal;
	unsigned uMM, uDD, uYYYY;

	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&sm_cs);

	// Formatting info must be initialized:
	if (!stDateFmt.nFmtLen)
		SetDateTimeFormat ();

	// Create a format string using correct separator charactor:
	wsprintf (szFmt, _T("%%d%c%%d%c%%d"),
		*stDateFmt.szSeparator,
		*stDateFmt.szSeparator);

	// Extract month, day and year from input string according to system
	// time format type. _stscanf will return number of fields successfully
	// extracted from input string:
	switch (stDateFmt.nFmt)
		{
		case VDATE_MMDDYY:
			nRetVal = _stscanf (lpDate, szFmt, &uMM, &uDD, &uYYYY);
			break;

		case VDATE_DDMMYY:
			nRetVal = _stscanf (lpDate, szFmt, &uDD, &uMM, &uYYYY);
			break;

		case VDATE_YYMMDD:
			nRetVal = _stscanf (lpDate, szFmt, &uYYYY, &uMM, &uDD);
			break;

		default:
			nRetVal = 0;
			break;
		}

	// _stscanf should have extracted 3 fields from input string.  If not
	// return FALSE to indicate error:
	if (nRetVal != 3)
		return (FALSE);

	// Correct year if given as 2 digits:
	// (If between 90 and 99 assume 19xx.  if less than 90 assume 20xx):
	if (uYYYY < 100)
		uYYYY += (uYYYY >= 90) ? 1900 : 2000;

	// Check for valid month:
	if (uMM < 1 || uMM > 12)
		return (FALSE);

	// Make sure day is a not zero:
	if (uDD < 1)
		return (FALSE);

	// Make sure day is not greater than number of days in month:
	// If February, must account for leap year:
	if (uMM == 2)
		{
		// Use table to check  Use ISLEAP macro to adjust for leap year:
		if (uDD > (unsigned)(anDaysPerMonth [1] + ISLEAP (uYYYY)))
			return (FALSE);
		}

	// If any other month, just use table to check day:
	else
		{
		if (uDD > anDaysPerMonth [uMM - 1])
			return (FALSE);
		}

	// If we make it here, date is valid so save it:
	m_st.wYear = uYYYY;
	m_st.wDay = uDD;
	m_st.wMonth = uMM;

	// Convert date to day of year if specified:
	if (bInitialize)
		ConvertDateToDays ();

	// Return TRUE to indicate success:
	return (TRUE);
	}

// **************************************************************************
// FormatDate ()
//
// Description:
//	Place the current date setting in specified output string buffer.  Format
//  will be obtained from the system.  If output buffer is too small, it will
//  be filled with "#" characters.
//
// Parameters:
//  LPTSTR		szDate		Output string buffer.
//	int			cnChars		Size of output string buffer.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::FormatDate (LPTSTR szDate, int cnChars)
	{
	int nYear;

	// Can't do anything if we are not given a valid output string
	// pointer, so return:
	if (!szDate)
		return;

	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&sm_cs);

	// Formatting info must be initialized:
	if (!stDateFmt.nFmtLen)
		SetDateTimeFormat ();

	// If the buffer is too small then fill with placeholders ("#") and
	// return:
	int i;
	if (cnChars <= stDateFmt.nFmtLen)
		{
		for (i = 0; i < cnChars; i++)
			szDate [i] = _T('#');

		szDate [i] = _T('\0');
		return;
		}

	// Truncate year for non-full year representation:
	nYear = m_st.wYear;
	if (!stDateFmt.bFullYear)
		nYear %= 100;

	// Format the date string according to system date format type:
	switch (stDateFmt.nFmt)
		{
		case VDATE_MMDDYY:
			wsprintf (szDate, stDateFmt.szFmtString, m_st.wMonth, m_st.wDay, nYear);
			break;

		case VDATE_DDMMYY:
			wsprintf (szDate, stDateFmt.szFmtString, m_st.wDay, m_st.wMonth, nYear);
			break;

		case VDATE_YYMMDD:
			wsprintf (szDate, stDateFmt.szFmtString, nYear, m_st.wMonth, m_st.wDay);
			break;

		default:
			wsprintf (szDate, _T("%d/%02d/%02d"), m_st.wMonth, m_st.wDay, m_st.wYear % 100);
			break;
		}
	}

// **************************************************************************
// FormatTime ()
//
// Description:
//	Place the current time setting in specified output string buffer.  Format
//  will be obtained from the system.  If output buffer is too small, it will
//  be filled with "#" characters.
//
// Parameters:
//  LPTSTR		szTime		Output string buffer.
//	int			cnChars		Size of output string buffer.
//	bool		bUseSec		Set to true to include seconds.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::FormatTime (LPTSTR szTime, int cnChars, bool bUseSec)
	{
	// Can't do anything if we are not given a valid output string
	// pointer, so return:
	if (!szTime)
		return;

	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&sm_cs);

	// Formatting info must be initialized:
	if (!stTimeFmt.nFmtLen)
		SetDateTimeFormat ();

	// Determine the minimum buffer requirements (add 3 characters for
	// seconds if used):
	int nMinChars = (bUseSec) ? stTimeFmt.nFmtLen + 3 : stTimeFmt.nFmtLen;

	// If the buffer is too small then fill with placeholders ("#") and
	// return:
	int i;
	if (cnChars <= nMinChars)
		{
		for (i = 0; i < cnChars; i++)
			szTime [i] = _T('#');

		szTime [i] = 0;
		return;
		}

	// Localize and round up to the nearest second:
	unsigned short wMinute = m_st.wMinute;
	unsigned short wSecond = m_st.wSecond;
	unsigned short wHour = m_st.wHour;

	// Need to round seconds up if milliseconds are >= 500:
	if (m_st.wMilliseconds >= 500)
		{
		// Increment seconds.  If this causes the seconds to equal
		// 60, we need to reset to 0 and increment minutes:
		if (++wSecond == 60)
			{
			wSecond = 0;
			
			// Increment minutes.  If this causes the minutes to equal
			// 60, we need to reset to 0 and increment hours:
			if (++wMinute == 60)
				{
				wMinute = 0;
				
				// Increment hours.  If this causes the hours to equal
				// 24, we need to reset to 0.
				if (++wHour == 24)
					{
					wHour = 0;
					}
				}
			}
		}
	
	// Save the proper separator character:
	TCHAR ch = *stTimeFmt.szSeparator;

	// Set flag if PM:
	bool bNight = (wHour >= 12);

	// Adjust for non-military system time format.  Hour value is currently
	// in 24 hour, military, format.
	if (!stTimeFmt.b24Hr)
		{
		// If PM, subtract 12 (value of 12 is OK as is):
		if (bNight && wHour > 12)
			wHour -= 12;

		// Else if 0, reset to 12:
		else if (!wHour)
			wHour = 12;
		}

	// Format the time with appropriate leading zero and seconds if required:
	if (bUseSec)
		{
		if (!stTimeFmt.bLeadingZero)
			wsprintf (szTime,	_T("%2d%c%02d%c%02d"), wHour, ch, wMinute, ch, wSecond);
		else
			wsprintf (szTime,	_T("%02d%c%02d%c%02d"),	wHour, ch, wMinute, ch, wSecond);
		}
	else
		{
		if (!stTimeFmt.bLeadingZero)
			wsprintf (szTime,	_T("%2d%c%02d"), wHour, ch, wMinute);
		else
			wsprintf (szTime,	_T("%02d%c%02d"),	wHour, ch, wMinute);
		}

	// Apply AM/PM indication if not using 24 hour format:
	if (!stTimeFmt.b24Hr)
		{
		// Add a space:
		lstrcat (szTime, _T(" "));

		// Add "PM" if night, or "AM" if not:
		lstrcat (szTime, (bNight) ? stTimeFmt.szPMString : stTimeFmt.szAMString);
		}
	}

// **************************************************************************
// operator <
//
// Description:
//	Custom less than operator.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  bool - true if we are "less than" cts.
// **************************************************************************
bool CTimeStamp::operator < (CTimeStamp &cts)	const
	{
	// If our year is greater than, then our timestamp can't be less than.
	// Return false.
	if (m_st.wYear > cts.m_st.wYear)
		return (false);

	// If we make it hear, our year must be less than or equal to.  If our 
	// year is less than, timestamp is less than. Return true.
	else if (m_st.wYear < cts.m_st.wYear)
		return (true);

	// If we make it here, we know years must be equal, so we need to
	// look at the day to make comparison.  If our day is less than, 
	// timestamp is less than.  Return true:
	else if (m_nDay < cts.m_nDay)
		return (true);

	// If we make it here, we know our day is greater than or equal.
	// If days are equal, we need to look at time of day to make
	// comparison.
	else if (m_nDay == cts.m_nDay)
		return (m_lMS < cts.m_lMS);

	// If we make it here, we know our day is greater than, so timestamp
	// can't be less than.  Return false.
	return (false);
	}

// **************************************************************************
// operator <=
//
// Description:
//	Custom less than or equal to operator.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  bool - true if we are "less than or equal to" cts.
// **************************************************************************
bool CTimeStamp::operator <= (CTimeStamp &cts)	const
	{
	// If our year is greater than, then our timestamp can't be less than or 
	// equal to.  Return false.
	if (m_st.wYear > cts.m_st.wYear)
		return (false);

	// If we make it hear, our year must be less than or equal to.  If our 
	// year is less than, timestamp is less than. Return true.
	else if (m_st.wYear < cts.m_st.wYear)
		return (true);

	// If we make it here, we know years must be equal, so we need to
	// look at the day to make comparison.  If our day is less than, 
	// timestamp is less than.  Return true:
	else if (m_nDay < cts.m_nDay)
		return (true);

	// If we make it here, we know our day is greater than or equal.
	// If days are equal, we need to look at time of day to make
	// comparison.
	else if (m_nDay == cts.m_nDay)
		return (m_lMS <= cts.m_lMS);

	// If we make it here, we know our day is greater than, so timestamp
	// can't be less than or equal to.  Return false.
	return (false);
	}

// **************************************************************************
// operator >
//
// Description:
//	Custom greater than operator.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  bool - true if we are "greater than" cts.
// **************************************************************************
bool CTimeStamp::operator > (CTimeStamp &cts)	const
	{
	// If our year is less than, then our timestamp can't be greater than.
	// Return false.
	if (m_st.wYear < cts.m_st.wYear)
		return (false);

	// If we make it hear, our year must be greater than or equal to.  If our 
	// year is greater than, timestamp is greater than. Return true.
	if (m_st.wYear > cts.m_st.wYear)
		return (true);

	// If we make it here, we know years must be equal, so we need to
	// look at the day to make comparison.  If our day is greater than, 
	// timestamp is greater than.  Return true:
	else if (m_nDay > cts.m_nDay)
		return (true);

	// If we make it here, we know our day is less than or equal.
	// If days are equal, we need to look at time of day to make
	// comparison.
	else if (m_nDay == cts.m_nDay)
		return (m_lMS > cts.m_lMS);

	// If we make it here, we know our day is less than, so timestamp
	// can't be greater than.  Return false.
	return (false);
	}

// **************************************************************************
// operator >=
//
// Description:
//	Custom greater than or equal to operator.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  bool - true if we are "greater than or equal to" cts.
// **************************************************************************
bool CTimeStamp::operator >= (CTimeStamp &cts)	const
	{
	// If our year is less than, then our timestamp can't be greater than or
	// equal to. Return false.
	if (m_st.wYear < cts.m_st.wYear)
		return (false);

	// If we make it hear, our year must be greater than or equal to.  If our 
	// year is greater than, timestamp is greater than. Return true.
	if (m_st.wYear > cts.m_st.wYear)
		return (true);

	// If we make it here, we know years must be equal, so we need to
	// look at the day to make comparison.  If our day is greater than, 
	// timestamp is greater than.  Return true:
	else if (m_nDay > cts.m_nDay)
		return (true);

	// If we make it here, we know our day is less than or equal.
	// If days are equal, we need to look at time of day to make
	// comparison.
	else if (m_nDay == cts.m_nDay)
		return (m_lMS >= cts.m_lMS);

	// If we make it here, we know our day is less than, so timestamp
	// can't be greater than or equal to.  Return false.
	return (false);
	}

// **************************************************************************
// operator ==
//
// Description:
//	Custom equals operator.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  bool - true if we are "equal to" cts.
// **************************************************************************
bool CTimeStamp::operator == (CTimeStamp &cts) const
	{
	// If years aren't the same, timestamps can't be the same:
	if (m_st.wYear != cts.m_st.wYear)
		return (false);

	// If days aren't the same, timestamps can't be the same:
	if (m_nDay != cts.m_nDay)
		return (false);

	// If time of days aren't the same, timestamps can't be the same:
	if (m_lMS != cts.m_lMS)
		return (false);

	// If we make it here, we must be the same:
	return (true);
	}

// **************************************************************************
// Compare ()
//
// Description:
//	Compares this object with another instance.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to be compared with.
//
// Returns:
//  int - -1 if we are greater than, 1 if we are less than, 0 if equal to cts.
// **************************************************************************
int CTimeStamp::Compare (CTimeStamp &cts) const
	{
	// If our year is greater than, then our timestamp is greater than:
	if (m_st.wYear > cts.m_st.wYear)
		return (-1);

	// If our year is less than, then our timestamp is less than:
	else if (m_st.wYear < cts.m_st.wYear)
		return (1);

	// If we make it here, we know years are equal.  We need to look
	// at day to make comparison.

	// If our day is greater than, then our timestamp is greater than:
	else if (m_nDay > cts.m_nDay)
		return (-1);

	// If days are the same, we need to look at time of day to make
	// comparison:
	else if (m_nDay == cts.m_nDay)
		{
		// If our time of day is greater than, than our timestamp is 
		// greater than:
		if (m_lMS > cts.m_lMS)
			return (-1);

		// If time of days are equal, then our timestamps are equal:
		else if (m_lMS == cts.m_lMS)
			return (0);

		// If we make it here, we know our time of day is less than,
		// so out timestamp is less than:
		else
			return (1);
		}

	// If we make it here, we know our day is less than, so our timestamp
	// is less than:
	else
		return (1);
	}

// **************************************************************************
// DiffMS ()
//
// Description:
//	Called to compute the difference in milliseconds between this object's
//	current settings and another.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to compute difference with.
//
// Returns:
//  long - difference = (this - cts) MS.
// **************************************************************************
long CTimeStamp::DiffMS (CTimeStamp &cts) const
	{
	long lDiff;
	int cnDayDiff;

	// Compute the difference in days:
	cnDayDiff = m_nDay - cts.m_nDay;

	// Must account for number of days in a year if years are different:

	// If our year is less, subtract 365 days, or 366 if a leap year:
	if (m_st.wYear < cts.m_st.wYear)
		cnDayDiff -= (ISLEAP (m_st.wYear) ? 366 : 365);

	// If our year is greater than, add 365 days, or 366 if a leap year:
	else if (m_st.wYear > cts.m_st.wYear)
		cnDayDiff += (ISLEAP (cts.m_st.wYear) ? 366 : 365);

	// Convert difference in days to milliseconds:
	lDiff = MSPERDAY * (long)cnDayDiff;

	// Add difference in time of day (in milliseconds)
	lDiff += (m_lMS - cts.m_lMS);

	// Return result:
	return (lDiff);
	}

// **************************************************************************
// DiffSec ()
//
// Description:
//	Called to computer the difference in seconds between this object's
//	current settings and another.
//
// Parameters:
//  CTimeStamp	&cts	CTimeStamp object to compute difference with.
//
// Returns:
//  long - difference = (this - cts) Sec.
// **************************************************************************
long CTimeStamp::DiffSec (CTimeStamp &cts) const
	{
	long lDiff;
	int cnDayDiff;

	// Compute the difference in days:
	cnDayDiff = m_nDay - cts.m_nDay;

	// Must account for number of days in a year if years are different:

	// If our year is less, subtract 365 days, or 366 if a leap year:
	if (m_st.wYear < cts.m_st.wYear)
		cnDayDiff -= (ISLEAP (m_st.wYear) ? 366 : 365);

	// If our year is greater than, add 365 days, or 366 if a leap year:
	else if (m_st.wYear > cts.m_st.wYear)
		cnDayDiff += (ISLEAP (cts.m_st.wYear) ? 366 : 365);

	// Convert difference in days to seconds:
	lDiff = SECPERDAY * (long)cnDayDiff;

	// Add difference in time of day (in milliseconds) and convert to seconds:
	lDiff += ((m_lMS - cts.m_lMS) / 1000L);

	// Return result:
	return (lDiff);
	}

// **************************************************************************
// IncDay ()
//
// Description:
//	Increment current settings by specified number of days.
//
// Parameters:
//  int			nDayInc		Number of days to increment by.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::IncDay (int nDayInc)
	{
	// Increment the day:
	m_nDay += nDayInc;

	// If result is negative, decrement years until positive:
	if (m_nDay < 0)
		{
		while (m_nDay < 0)
			{
			// Decrement year:
			--m_st.wYear;

			// Add number of day in year (366 if leap year) to
			// account for decremented year:
			m_nDay += ISLEAP (m_st.wYear) ? 366 : 365;
			}
		}

	// Else make sure result is less than the number of days in year:
	else
		{
		// Get number of day in current year (366 if leap year)
		int nDaysPerYear = ISLEAP (m_st.wYear) ? 366 : 365;

		// If day is beyond end of year, increment year until day of year
		// is valid:
		while (m_nDay > nDaysPerYear - 1)
			{
			// Subtract number of days in current year:
			m_nDay -= nDaysPerYear;

			// Increment year to account for incremented year:
			++m_st.wYear;

			// Get number of days in new year for next test:
			nDaysPerYear = ISLEAP (m_st.wYear) ? 366 : 365;
			}
		}

	ConvertDateFromDays ();
	}

// **************************************************************************
// IncMS ()
//
// Description:
//	Increment current settings by specified number of milliseconds.
//
// Parameters:
//  int			lIntervalMS		Number of milliseconds to increment by.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::IncMS (long lIntervalMS)
	{
	int nDayInc;

	// No need to do anything if interval is zero:
	if (!lIntervalMS)
		return;

	// Calculate whole number of days to increment by:
	nDayInc = (int)(lIntervalMS / MSPERDAY);

	// Calculate fractional number of days to increment by:

	// If interval is positive:
	if (lIntervalMS > 0)
		{
		// Increment milliseconds:
		m_lMS += (lIntervalMS % MSPERDAY);

		// If result is greater than the number of milliseconds per day,
		// decrement by number of milliseconds per day and increment day:
		if (m_lMS >= MSPERDAY)
			{
			m_lMS -= MSPERDAY;
			++nDayInc;
			}
		}

	// If interval is negative:
	else
		{
		// Decrement milliseconds:
		m_lMS -= (-lIntervalMS % MSPERDAY);

		// If result is negative, increment by number of milliseconds per
		// day and decrement the day:
		if (m_lMS < 0)
			{
			m_lMS += MSPERDAY;
			--nDayInc;
			}
		}

	// Increment day if needed:
	if (nDayInc)
		IncDay (nDayInc);

	// Update the time members of the time stamp struct:
	ConvertTimeFromMS ();
	}

// **************************************************************************
// IncSec ()
//
// Description:
//	Increment current settings by specified number of seconds.
//
// Parameters:
//  int			lIntervalSec	Number of seconds to increment by.
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::IncSec (long lIntervalSec)
	{
	int nDayInc;

	// No need to do anything if interval is zero:
	if (!lIntervalSec)
		return;

	// Calculate whole number of days to increment by:
	nDayInc = (int)(lIntervalSec / SECPERDAY);

	// Calculate fractional number of days to increment by:

	// If interval is positive:
	if (lIntervalSec > 0)
		{
		// Increment milliseconds:
		m_lMS += (lIntervalSec % SECPERDAY) * 1000;

		// If result is greater than the number of milliseconds per day,
		// decrement by number of milliseconds per day and increment day:
		if (m_lMS >= MSPERDAY)
			{
			m_lMS -= MSPERDAY;
			++nDayInc;
			}
		}

	// If interval is negative:
	else
		{
		// Decrement milliseconds:
		m_lMS -= (-lIntervalSec % SECPERDAY) * 1000;

		// If result is negative, increment by number of milliseconds per
		// day and decrement the day:
		if (m_lMS < 0)
			{
			m_lMS += MSPERDAY;
			--nDayInc;
			}
		}

	// Increment day if needed:
	if (nDayInc)
		IncDay (nDayInc);

	// Update the time members of the time stamp struct:
	ConvertTimeFromMS ();
	}

// **************************************************************************
// ConvertDateToDays ()
//
// Description:
//	Convert date to day of the year.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::ConvertDateToDays ()
	{
	int nMonth = 0;

	// Assign number of days in February:
	anDaysPerMonth [1] = ISLEAP (m_st.wYear) ? 29 : 28;

	// Save of day of month (zero based):
	m_nDay = m_st.wDay - 1;

	// Add number of days in each previous month:
	while (nMonth < m_st.wMonth - 1)
		m_nDay += anDaysPerMonth [nMonth++];

	}

// **************************************************************************
// ConvertDateFromDays ()
//
// Description:
//	Convert day of year to date.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::ConvertDateFromDays ()
	{
	// Day of year as one based value:
	int cnDays = m_nDay + 1;

	// Assign number of days in February
	anDaysPerMonth [1] = ISLEAP (m_st.wYear) ? 29 : 28;
	m_st.wMonth = 0;

	// Subtract number of days in each month until result is less than
	// the number of days in month.  At that point we know what month and
	// what day of that month date is for:
	while (cnDays > anDaysPerMonth [m_st.wMonth])
		cnDays -= anDaysPerMonth [m_st.wMonth++];

	++m_st.wMonth;			// Month is one based
	m_st.wDay = cnDays;		// Date is one based
	}

// **************************************************************************
// ConvertTimeToMS ()
//
// Description:
//	Convert time to milliseconds from midnight.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::ConvertTimeToMS ()
	{
	// Take value in m_st and convert it to milliseconds.  Store result in
	// m_lMS:
	m_lMS = (long)m_st.wHour * 3600;
	m_lMS += (long)m_st.wMinute * 60;
	m_lMS += m_st.wSecond;
	m_lMS *= 1000;
	m_lMS += m_st.wMilliseconds;
	}

// **************************************************************************
// ConvertTimeFromMS ()
//
// Description:
//	Convert milliseconds from midnight to time.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::ConvertTimeFromMS ()
	{
	// Save time of day in milliseconds:
	long lTime = m_lMS;

	// Compute whole number of hours in this number of milliseconds.  
	m_st.wHour = (unsigned short)(lTime / (3600L * 1000L));

	// Subtract this number of hours.  Remainder is number of minutes:
	lTime -= (long)m_st.wHour * (3600L * 1000L);

	// Compute whole number of minutes in remainder:
	m_st.wMinute = (unsigned short)(lTime / (60L * 1000L));

	// Subtract this number of minutes.  Remainder is number of seconds:
	lTime -= (long)m_st.wMinute * (60L * 1000L);

	// Compute whole number of seconds in remainder:
	m_st.wSecond = (unsigned short)(lTime / 1000L);

	// Subtract this number of seconds.  Remainder is number of milliseconds:
	lTime -= (long)m_st.wSecond * 1000L;

	// Remander is number of milliseconds:
	m_st.wMilliseconds = (unsigned short)lTime;
	}

// **************************************************************************
// SetDateTimeFormat ()
//
// Description:
//	Obtain date and time format from system.  Called when a program starts up
//	and when win.ini changes
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CTimeStamp::SetDateTimeFormat ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&sm_cs);

	TRACE (_T("Reading system date/time settings ...\n"));

	// Create registry section string.  This is where time and date formats
	// are stored:
	LPCTSTR lpintl = _T("intl");

	// Get numerical format properties:
	stDateFmt.nFmt = GetProfileInt (lpintl, _T("iDate"), VDATE_MMDDYY);
	stTimeFmt.b24Hr = GetProfileInt (lpintl, _T("iTime"), FALSE);
	stTimeFmt.bLeadingZero = GetProfileInt (lpintl, _T("iTLZero"), FALSE);

	// Get string format properties:
	GetProfileString (lpintl, _T("sDate"), _T("/"), stDateFmt.szSeparator, 2);
	GetProfileString (lpintl, _T("sTime"), _T(":"), stTimeFmt.szSeparator, 2);
	GetProfileString (lpintl, _T("sShortDate"), _T("M/d/yy"), stDateFmt.szShortDate,
		_countof (stDateFmt.szShortDate));

	GetProfileString (lpintl, _T("s1159"), _T("am"), stTimeFmt.szAMString, 8);
	GetProfileString (lpintl, _T("s2359"), _T("pm"), stTimeFmt.szPMString, 8);


	// Determine where to place leading zeroes:
	int nLen = lstrlen (stDateFmt.szShortDate);

	// Initialize number of leading zero members so that when we increment 
	// for each corresponding field in format string, result will give
	// us the correct number of leading zeros:
	stDateFmt.bMonthLeadingZero = -1;	// Expect at least one digit
	stDateFmt.bDayLeadingZero = -1;		// Expect at least one digit
	stDateFmt.bFullYear = -2;			// Expect at least two digits

	// Process each character in format string.  Add increment leading
	// zero members each time we encounter one of their digit place holders
	// in string:
	for (int i = 0; i < nLen; i++)
		{
		switch (stDateFmt.szShortDate [i])
			{
			// Month place holders:
			case _T('m'):
			case _T('M'):
				++stDateFmt.bMonthLeadingZero;
				break;

			// Day place holders:
			case _T('d'):
			case _T('D'):
				++stDateFmt.bDayLeadingZero;
				break;

			// Year place holders:
			case _T('y'):
			case _T('Y'):
				++stDateFmt.bFullYear;
				break;

			// Skip over all other characters:
			default:
				continue;
			}
		}

	// Construct a format string for date display:
	switch (stDateFmt.nFmt)
		{
		case VDATE_MMDDYY:
			wsprintf (
				stDateFmt.szFmtString,
				_T("%%.%dd%c%%.%dd%c%%.%dd"),
				(stDateFmt.bMonthLeadingZero) ? 2 : 1,
				*stDateFmt.szSeparator,
				(stDateFmt.bDayLeadingZero) ? 2 : 1,
				*stDateFmt.szSeparator,
				(stDateFmt.bFullYear) ? 4 : 2
			);
			break;

		case VDATE_DDMMYY:
			wsprintf (
				stDateFmt.szFmtString,
				_T("%%.%dd%c%%.%dd%c%%.%dd"),
				(stDateFmt.bDayLeadingZero) ? 2 : 1,
				*stDateFmt.szSeparator,
				(stDateFmt.bMonthLeadingZero) ? 2 : 1,
				*stDateFmt.szSeparator,
				(stDateFmt.bFullYear) ? 4 : 2
			);
			break;

		case VDATE_YYMMDD:
			wsprintf (
				stDateFmt.szFmtString,
				_T("%%.%dd%c%%.%dd%c%%.%dd"),
				(stDateFmt.bFullYear) ? 4 : 2,
				*stDateFmt.szSeparator,
				(stDateFmt.bMonthLeadingZero) ? 2 : 1,
				*stDateFmt.szSeparator,
				(stDateFmt.bDayLeadingZero) ? 2 : 1
			);
			break;
		}

	// Initialize the time format length:
	stTimeFmt.nFmtLen = 5; // Minimum (hh:mm)

	// Adjust format length if 12 hour format:
	if (!stTimeFmt.b24Hr)
		{
		int nMaxLen;

		// Save length of "PM" string:
		nMaxLen = lstrlen (stTimeFmt.szPMString);

		// If length of "AM" string is longer than "PM" string, save it
		// instead:
		if (lstrlen (stTimeFmt.szAMString) > nMaxLen)
			nMaxLen = lstrlen (stTimeFmt.szAMString);

		// Add length of "AM" or "PM" string plus 1 for space:
		stTimeFmt.nFmtLen += (nMaxLen + 1);
		}

	// Assign the date format length:
	stDateFmt.nFmtLen = 8;	// Minimum (mm/dd/yy)

	// Add 2 if using 4 digit years:
	if (stDateFmt.bFullYear)
		stDateFmt.nFmtLen += 2;
	}



