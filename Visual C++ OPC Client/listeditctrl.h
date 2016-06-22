// **************************************************************************
// listeditctrl.h
//
// Description:
//	Defines a special combination list/edit control class used by the 
//	CKItemWriteDlg class.
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


#ifndef _LISTEDITCTRL_H
#define _LISTEDITCTRL_H


// **************************************************************************
class CKListEditCtrl : public CListCtrl
	{
	// Construction
	public:
		CKListEditCtrl ();

	// Attributes
	public:

	// Operations
	public:
		int HitTestEx (CPoint &cPoint, int *pCol);
		CEdit* EditSubLabel (int nItem, int nCol);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKListEditCtrl)
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKListEditCtrl ();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CKListEditCtrl)
		afx_msg void OnEndLabelEdit (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		//}}AFX_MSG
		afx_msg long OnPrevSubLabel (WPARAM wParam, LPARAM lParam);
		afx_msg long OnNextSubLabel (WPARAM wParam, LPARAM lParam);

		DECLARE_MESSAGE_MAP ()

	private:
		int m_nSelItem;
	};

// **************************************************************************
class CKListEdit : public CEdit
	{
	// Construction
	public:
		CKListEdit (int iItem, int iSubItem, CString sInitText);

	// Attributes
	public:

	// Operations
	public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKListEdit)
	public:
		virtual BOOL PreTranslateMessage (MSG *pMsg);
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKListEdit ();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CKListEdit)
		afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnNcDestroy ();
		afx_msg void OnKillFocus (CWnd *pNewWnd);
		afx_msg long OnPaste (WPARAM wParam, LPARAM lParam);
		//}}AFX_MSG
		

		DECLARE_MESSAGE_MAP ()

	private:		
		int m_iItem;	
		int m_iSubItem;	
		CString m_sInitText;	
		BOOL    m_bESC;	 	
	};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _LISTEDITCTRL_H
