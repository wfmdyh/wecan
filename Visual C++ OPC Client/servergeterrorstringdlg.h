// **************************************************************************
// servergeterrorstringdlg.h
//
// Description:
//	Defines a dialog class for retrieving a meaningful description of an
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


#ifndef _SERVERGETERRORSTRINGDLG_H
#define _SERVERGETERRORSTRINGDLG_H

class CKServer;


// **************************************************************************
class CKServerGetErrorStringDlg : public CDialog
	{
	// Construction
	public:
		CKServerGetErrorStringDlg (CKServer *pServer, CWnd *pParent = NULL);   // standard constructor

	// Dialog Data
		//{{AFX_DATA(CKServerGetErrorStringDlg)
		enum { IDD = IDD_SERVER_GETERROR };
		CString	m_strErrorCode;
		CString	m_strErrorDescription;
		//}}AFX_DATA

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKServerGetErrorStringDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	private:
		IOPCServer *m_pIServer;

	protected:

		// Generated message map functions
		//{{AFX_MSG(CKServerGetErrorStringDlg)
		afx_msg void OnApply ();
		virtual BOOL OnInitDialog ();
		afx_msg void OnChangeErrorCode ();
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _SERVERGETERRORSTRINGDLG_H
