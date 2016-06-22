// **************************************************************************
// itemview.h
//
// Description:
//	Defines a CListView derived class.  This the top right pane of our GUI
//	which shows the OPC items.
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


#ifndef _ITEMVIEW_H
#define _ITEMVIEW_H

class CKItem;
class CKGroup;
class CKServer;


// **************************************************************************
class CKItemView : public CListView
	{
	protected:
		CKItemView ();           // protected constructor used by dynamic creation
		DECLARE_DYNCREATE (CKItemView)

	// Attributes
	public:

	// Operations
	public:
		int GetSelectedCount ();

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKItemView)
	public:
		virtual BOOL Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext = NULL);
	protected:
		virtual void OnDraw (CDC *pDC);      // overridden to draw this view
		virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
		virtual void OnUpdate (CView *pSender, LPARAM lHint, CObject *pHint);
		virtual void PreSubclassWindow ();
		//}}AFX_VIRTUAL

	public:
		virtual int OnToolHitTest (CPoint cPoint, TOOLINFO *pTI) const;

	protected:
		CImageList m_cImageList;

		WORD m_wUpdateInterval;

		static WORD sm_wSortColumn;			// access from static qsort function
		static WORD sm_wSortOrder;			// access from static qsort function
		static DWORD *sm_pSortedItemList;	// access from static qsort function
		int m_cnSortedItems;
		int m_nSortedListSize;
		
		CCriticalSection m_csSortedList;

	// Implementation
	protected:
		virtual ~CKItemView ();
	#ifdef _DEBUG
		virtual void AssertValid () const;
		virtual void Dump (CDumpContext& dc) const;
	#endif

		void Insert (CKItem *pItem);
		void DeleteAllItems ();
		
		void SortList ();

		static int CompareItems (const void *pItem1, const void *pItem2);

		int GetSelectedItems (CObArray &cItemList);
		CKGroup* GetSelectedGroup ();
		CKServer* GetSelectedServer ();

		int GetCellRectFromPoint (CPoint &cPoint, CRect &rc, int *pCol) const;
		bool RequireCellToolTip (int nRow, int nCol, CRect rc) const;
		
		// Generated message map functions
	protected:
		//{{AFX_MSG(CKItemView)
		afx_msg void OnDestroy ();
		afx_msg void OnNewItem ();
		afx_msg void OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnProperties ();
		afx_msg void OnColumnClick (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnTimer (UINT nIDEvent);
		afx_msg void OnDelete ();
		afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnUpdateTimer ();
		afx_msg void OnSyncWrite ();
		afx_msg void OnAsync10Write ();
		afx_msg void OnAsync20Write ();
		afx_msg void OnSetActive ();
		afx_msg void OnSetInactive ();
		afx_msg void OnSyncReadCache ();
		afx_msg void OnSyncReadDevice ();
		afx_msg void OnAsync10ReadCache ();
		afx_msg void OnAsync10ReadDevice ();
		afx_msg void OnAsync20ReadDevice ();
		afx_msg void OnAsync10RefreshCache ();
		afx_msg void OnAsync10RefreshDevice ();
		afx_msg void OnAsync20RefreshCache ();
		afx_msg void OnAsync20RefreshDevice ();
		afx_msg void OnCopy ();
		afx_msg void OnCut ();
		afx_msg void OnPaste ();
		afx_msg void OnDblclk (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnToolTipText (UINT uID, NMHDR *pNMHDR, LRESULT *pResult);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _ITEMVIEW_H
