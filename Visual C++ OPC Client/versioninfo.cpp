// **************************************************************************
// versioninfo.cpp
//
// Description:
//	Implements a class to contain application version information.
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
#include "versioninfo.h"

#ifndef _WIN32_WCE
#pragma comment (lib, "version.lib")
#endif


// **************************************************************************
// CVersionInfo ()
//
// Description:
//	Constructor.
//
// Parameters:
//  LPCTSTR		lpAppName	Pointer to application name string.
//
// Returns:
//  none
// **************************************************************************
CVersionInfo::CVersionInfo (LPCTSTR lpAppName)
	{
	// Initialize member variables:
	ASSERT (lpAppName);
	Initialize (lpAppName);
	}

// **************************************************************************
// CVersionInfo ()
//
// Description:
//	Constructor.
//
// Parameters:
//  HINSTANCE	hInst		Handle of application instance.
//
// Returns:
//  none
// **************************************************************************
CVersionInfo::CVersionInfo (HINSTANCE hInst)
	{
	// Define buffer for application name:
	TCHAR szAppName [_MAX_PATH + 1];

	// Get the application name (name of .exe file):
	if (!GetModuleFileName (hInst, szAppName, _countof (szAppName)))
		{
		// Failed to get name.  Set string to NULL and return:
		TRACE (_T("GetModuleFileName () failed, OS Error == %08X\n"), GetLastError ());
		ZeroMemory (&m_stFixedInfo, sizeof (m_stFixedInfo));
		return;
		}

	// Initialize member variables:
	Initialize (szAppName);
	}

// **************************************************************************
// Initialize ()
//
// Description:
//	Initialize object of this class from application executable file version
//	data.
//
// Parameters:
//  LPCTSTR		szAppName	Pointer to application name string.
//
// Returns:
//  void
// **************************************************************************
void CVersionInfo::Initialize (LPCTSTR szAppName)
	{
	ASSERT (szAppName);

	// Initialize fixed info structure:
	ZeroMemory (&m_stFixedInfo, sizeof (m_stFixedInfo));

	// Load the version information
	// Load signon version information
	DWORD dwHandle;
	DWORD dwSize;
  
	// Determine the size of the VERSIONINFO resource:
	if (!(dwSize = GetFileVersionInfoSize ((LPTSTR)szAppName, &dwHandle)))
		{
		TRACE (_T("GetFileVesionInfoSize () failed on %s"), szAppName);
		return;
		}

	// Declare pointer to version info resource:
	LPBYTE lpVerInfo = NULL;
	
#ifdef _WIN32_WCE
	TRY
#else
	try
#endif
		{
		// Allocate memory to hold version info:
		lpVerInfo = new BYTE [dwSize];
  
		// Read the VERSIONINFO resource from the file:
		if (GetFileVersionInfo ((LPTSTR)szAppName, dwHandle, dwSize, lpVerInfo))
			{
			VS_FIXEDFILEINFO *lpFixedInfo;
			LPCTSTR lpText;
			UINT uSize;
    
			// Read the FILE DESCRIPTION:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\FileDescription"), (void **)&lpText, &uSize))
				m_strDescription = lpText;

			// Read additional comment:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\Comments"), (void **)&lpText, &uSize))
				m_strComments = lpText;

			// Read company name:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\CompanyName"), (void **)&lpText, &uSize))
				m_strCompany = lpText;

			// Read product name:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\ProductName"), (void **)&lpText, &uSize))
				m_strProductName = lpText;

			// Read internal name:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\InternalName"), (void **)&lpText, &uSize))
				m_strInternalName = lpText;

			// Read legal copyright:
			if (VerQueryValue (lpVerInfo, _T("\\StringFileInfo\\040904B0\\LegalCopyright"), (void **)&lpText, &uSize))
				m_strLegalCopyright = lpText;

			// Read the FIXEDINFO portion:
			if (VerQueryValue (lpVerInfo, _T("\\"), (void **)&lpFixedInfo, &uSize))
				m_stFixedInfo = *lpFixedInfo;
			}
		}

	// Handle exceptions:
#ifdef _WIN32_WCE
	CATCH (CException, e)
		{
		e->Delete ();
#else
	catch (...)
		{
#endif
		EXCEPTIONMSG ();
		}

#ifdef _WIN32_WCE
	END_CATCH
#endif

	// Free memory allocated for version info:
	delete [] lpVerInfo;
	}

// **************************************************************************
// Format ()
//
// Description:
//	Places application executable filie version in output string, using
//	V<major version>.<minor version>.<build>[ - U][ (Debug)] format, where "U"
//	is for UNICODE build.
//
// Parameters:
//  CString		&strOutput		Output string.
//
// Returns:
//  void
// **************************************************************************
void CVersionInfo::Format (CString &strOutput)
	{
	// Append version number information to the sign on string:
	strOutput.Format (_T("V%d.%d%d.%d"), 
		HIWORD (m_stFixedInfo.dwFileVersionMS),
		LOWORD (m_stFixedInfo.dwFileVersionMS),
		HIWORD (m_stFixedInfo.dwFileVersionLS),
		LOWORD (m_stFixedInfo.dwFileVersionLS));

	// Add UNICODE tag:
#ifndef _WIN32_WCE
#ifdef _UNICODE
	strOutput += _T(" - U");
#endif
#endif

	// If the file was built for debugging, include a debug indication:
	if (m_stFixedInfo.dwFileFlags & VS_FF_DEBUG)
		strOutput += _T(" (Debug)");
	}

