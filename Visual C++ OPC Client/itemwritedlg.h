// **************************************************************************
// itemwritedlg.h
//
// Description:
//	Defines a dialog class for assigning values to write to selected OPC
//	items.
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


#ifndef _ITEMWRITEDLG_H
#define _ITEMWRITEDLG_H

#include "listeditctrl.h"

class CKGroup;

#define WRITE_SYNC		0
#define WRITE_ASYNC10	1
#define WRITE_ASYNC20	2	


/////////////////////////////////////////////////////////////////////////////
// CKItemWriteDlg dialog

// **************************************************************************
class CKItemWriteDlg : public CDialog
	{
	// Construction
	public:
		CKItemWriteDlg (CKGroup *pGroup, CObArray &cItemList, DWORD cdwItems, WORD wType);   

	// Dialog Data
		//{{AFX_DATA(CKItemWriteDlg)
		enum { IDD = IDD_ITEM_WRITE };
			// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKItemWriteDlg)
		protected:
		virtual void DoDataExchange (CDataExchange *pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	public:

	// Implementation
	protected:
		CKGroup *m_pGroup;
		CObArray *m_pItemList;
		DWORD m_cdwItems;
		WORD m_wType;
		
		CKListEditCtrl m_cListEditCtrl;
		CImageList m_cImageList;
		
		// Generated message map functions
		//{{AFX_MSG(CKItemWriteDlg)
		virtual BOOL OnInitDialog ();
		virtual void OnOK ();
		afx_msg void OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnTimer (UINT nIDEvent);
		afx_msg void OnDestroy ();
		afx_msg void OnApply ();
		//}}AFX_MSG
		afx_msg long OnWriteValueChanged (WPARAM wParam, LPARAM lParam);

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _ITEMWRITEDLG_H
