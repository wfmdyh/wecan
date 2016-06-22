// **************************************************************************
// opctestclient.h
//
// Description:
//	Defines the OPC Quick Client application root class.
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


#ifndef _OPCTESTCLIENT_H
#define _OPCTESTCLIENT_H


// **************************************************************************
class CKApp : public CWinApp
	{
	public:
		CKApp ();
		~CKApp ();

		void OnUpdateMRUFile (CCmdUI *pCmdUI);

		// Private Label Constants
		LPCTSTR GetCompanyName () {return (m_strCompanyName);}
		LPCTSTR GetProductName () {return (m_strProductName);}
		LPCTSTR GetInternalName () {return (m_strInternalName);}
		LPCTSTR GetDescription () {return (m_strDescription);}
		LPCTSTR GetComments () {return (m_strComments);}
		LPCTSTR GetLegalCopyright () {return (m_strLegalCopyright);}
		LPCTSTR GetDefConnectProgID () {return (m_strDefConnectProgID);}

	protected:
		bool m_bComInitialized;

		// Version information:
		CString m_strCompanyName;
		CString m_strProductName;
		CString m_strInternalName;
		CString m_strDescription;
		CString m_strComments;
		CString m_strLegalCopyright;
		CString m_strDefConnectProgID;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKApp)
	public:
		virtual BOOL InitInstance ();
		virtual int ExitInstance ();
		//}}AFX_VIRTUAL

	// Implementation

		//{{AFX_MSG(CKApp)
		afx_msg void OnAppAbout ();
		afx_msg void OnOptions ();
		afx_msg void OnResetEventLog ();
		afx_msg void OnFileOpen ();
		//}}AFX_MSG
		afx_msg BOOL OnFileOpenMRU (UINT nID);

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _OPCTESTCLIENT_H
