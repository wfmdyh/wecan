// **************************************************************************
// serverenumgroupsdlg.h
//
// Description:
//	Defines a dialog class for group enumerations.
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


#ifndef _SERVERENUMGROUPSDLG_H
#define _SERVERENUMGROUPSDLG_H


// **************************************************************************
class CKServerEnumerateGroupsDlg : public CDialog
	{
	// Construction
	public:
		CKServerEnumerateGroupsDlg (IOPCServer *pIServer, CWnd *pParent = NULL);   // standard constructor

	// Dialog Data
		//{{AFX_DATA(CKServerEnumerateGroupsDlg)
		enum { IDD = IDD_SERVER_ENUMERATEGROUPS };
		BOOL	m_bActive;
		int		m_nInterface;
		int		m_nScope;
		DWORD	m_dwLanguageID;
		CString	m_strName;
		DWORD	m_dwUpdateRate;
		float	m_fDeadband;
		long	m_lBias;
		//}}AFX_DATA

	private:
		IOPCServer *m_pIServer;
		CListCtrl *m_pEnumList;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKServerEnumerateGroupsDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		void EnumerateGroups ();
		void UpdateState (CString &strName);

		// Generated message map functions
		//{{AFX_MSG(CKServerEnumerateGroupsDlg)
		afx_msg void OnChange ();
		afx_msg void OnApply ();
		virtual BOOL OnInitDialog ();
		afx_msg void OnSelectItem (NMHDR *pNMHDR, LRESULT *pResult);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _SERVERENUMGROUPSDLG_H
