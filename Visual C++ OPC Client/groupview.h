// **************************************************************************
// groupview.h
//
// Description:
//	Defines a CTreeView derived class.  This the top left pane of our GUI
//  which shows the OPC servers and groups.
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


#ifndef _GROUPVIEW_H
#define _GROUPVIEW_H

class CKServer;
class CKGroup;


// **************************************************************************
class CKGroupView : public CTreeView
	{
	protected:
		CKGroupView ();           // protected constructor used by dynamic creation
		DECLARE_DYNCREATE (CKGroupView)

	// Attributes
	public:

	// Operations
	public:
		void Insert (HTREEITEM hParent, CObject *pObject, bool bSelect = true);
		void Delete (HTREEITEM hItem);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKGroupView)
	public:
		virtual BOOL Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext = NULL);
	protected:
		virtual void OnDraw (CDC *pDC);      // overridden to draw this view
		virtual void OnUpdate (CView *pSender, LPARAM lHint, CObject *pHint);
		//}}AFX_VIRTUAL

	protected:
		CImageList m_cImageList;

	// Implementation
	protected:
		virtual ~CKGroupView ();
	#ifdef _DEBUG
		virtual void AssertValid () const;
		virtual void Dump (CDumpContext& dc) const;
	#endif

		// Generated message map functions
	protected:
		//{{AFX_MSG(CKGroupView)
		afx_msg void OnNewServer ();
		afx_msg void OnNewGroup ();
		afx_msg void OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnProperties ();
		afx_msg void OnSelectionChanged (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnDelete ();
		afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk (UINT nFlags, CPoint point);
		afx_msg void OnGetErrorString ();
		afx_msg void OnCloneGroup ();
		afx_msg void OnGetGroupByName ();
		afx_msg void OnEnumerateGroups ();
		afx_msg void OnConnect ();
		afx_msg void OnDisconnect ();
		afx_msg void OnReconnect ();
		afx_msg void OnCopy ();
		afx_msg void OnCut ();
		afx_msg void OnPaste ();
		afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnExportCsv ();
		afx_msg void OnImportCsv ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _GROUPVIEW_H
