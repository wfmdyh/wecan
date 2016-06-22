// **************************************************************************
// updateintervaldlg.h
//
// Description:
//	Defines a dialog class for assigning view update interval.
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


#ifndef _UPDATEINTERVALDLG_H
#define _UPDATEINTERVALDLG_H


// **************************************************************************
class CKUpdateIntervalDlg : public CDialog
	{
	// Construction
	public:
		CKUpdateIntervalDlg (WORD wInterval, CWnd *pParent = NULL);   // standard constructor

	// Dialog Data
		//{{AFX_DATA(CKUpdateIntervalDlg)
		enum { IDD = IDD_VIEW_UPDATEINTERVAL };
		int	m_nInterval;
		//}}AFX_DATA

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKUpdateIntervalDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:

		// Generated message map functions
		//{{AFX_MSG(CKUpdateIntervalDlg)
		virtual BOOL OnInitDialog ();
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP ()
	};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _UPDATEINTERVALDLG_H
