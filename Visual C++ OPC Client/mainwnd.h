// **************************************************************************
// mainwnd.h
//
// Description:
//	Defines a CFrameWnd derived class.  This the main window of our GUI which
//	contains the group view, item view, and event view panes - the view parts
//	of our MFC SDI document/view architecture.
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


#ifndef _MAINWND_H
#define _MAINWND_H

class CKGroupView;
class CKItemView;
class CKEventView;


// **************************************************************************
class CKMainWnd : public CFrameWnd
	{
	protected: // create from serialization only
		CKMainWnd ();
		DECLARE_DYNCREATE (CKMainWnd)

	// Attributes
	public:

	private:
		bool m_bLoaded;

		CToolBar m_cToolBar;
		CStatusBar m_cStatusBar;
		CSplitterWnd m_cDataView;
		CSplitterWnd m_cEventView;
		CKEventView *m_pEventLog;

		int m_cnItems;
		
		CString m_strStatusText;
		CString m_strIdleStatusText;
		bool m_bForceStatusText;

		CCriticalSection m_csStatus;

	// Operations
	public:
		void LogMsg (EVENTTYPE eType, LPCTSTR lpszBuffer);
		void UpdateItemCount (int nDelta);
		void SetStatusBarText (LPCTSTR lpszText);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKMainWnd)
	public:
		virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
		virtual void ActivateFrame (int nCmdShow = -1);
		virtual BOOL OnCmdMsg (UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);
	protected:
		virtual BOOL OnCreateClient (LPCREATESTRUCT lpcs, CCreateContext *pContext);
	//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKMainWnd ();
	#ifdef _DEBUG
		virtual void AssertValid () const;
		virtual void Dump (CDumpContext& dc) const;
	#endif

	protected:
		void LoadSettings ();
		void SaveSettings ();

		CKGroupView* GetGroupView () {return ((CKGroupView *)m_cDataView.GetPane (0, 0));}
		CKItemView* GetItemView () {return ((CKItemView *)m_cDataView.GetPane (0, 1));}
		CKEventView* GetEventView () {return (m_pEventLog);}

	// Generated message map functions
	protected:
		//{{AFX_MSG(CKMainWnd)
		afx_msg void OnClose ();
		afx_msg void OnEndSession (BOOL bEnding);
		afx_msg void OnUpdateCmdUI (CCmdUI *pCmdUI);
		afx_msg void OnTimer (UINT nIDEvent);
		afx_msg void OnDestroy ();
		//}}AFX_MSG
		afx_msg long OnServerShutdownRequest (WPARAM wParam, LPARAM lParam);
		afx_msg long OnSelectGroup (WPARAM wParam, LPARAM lParam);
		afx_msg long OnRefreshItemView (WPARAM wParam, LPARAM lParam);
		afx_msg long OnItemReAdd (WPARAM wParam, LPARAM lParam);
		afx_msg long OnChangeView (WPARAM wParam, LPARAM lParam);

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _MAINWND_H
