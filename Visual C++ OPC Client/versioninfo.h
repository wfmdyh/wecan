// **************************************************************************
// versioninfo.h
//
// Description:
//	Defines a class to contain application version information.
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


#ifndef _VERSIONINFO_H
#define _VERSIONINFO_H


// **************************************************************************
class CVersionInfo
	{
	public:
		CVersionInfo (LPCTSTR lpszAppName);	// If you know the filename
		CVersionInfo (HINSTANCE hInst);			// If the file is in memory

		WORD GetMajorVersion () {return (HIWORD (m_stFixedInfo.dwFileVersionMS));}
		WORD GetMinorVersion () {return (LOWORD (m_stFixedInfo.dwFileVersionMS));}
		WORD GetBuildNumber () {return (LOWORD (m_stFixedInfo.dwFileVersionLS));}
		
		LPCTSTR GetDescription () {return (m_strDescription);}
		LPCTSTR GetComments () {return (m_strComments);}
		LPCTSTR GetCompany () {return (m_strCompany);}
		LPCTSTR GetProductName () {return (m_strProductName);}
		LPCTSTR GetInternalName () {return (m_strInternalName);}
		LPCTSTR GetLegalCopyright () {return (m_strLegalCopyright);}

		void Format (CString &strOutput);

		const VS_FIXEDFILEINFO &GetFixedInfo () {return (m_stFixedInfo);}

	protected:
		void Initialize (LPCTSTR lpszAppName);

		VS_FIXEDFILEINFO m_stFixedInfo;
		CString m_strDescription;
		CString m_strComments;
		CString m_strCompany;
		CString m_strProductName;
		CString m_strInternalName;
		CString m_strLegalCopyright;
	};


#endif	// _VERSIONINFO_H
