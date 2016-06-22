// **************************************************************************
// eventview.h
//
// Description:
//	Defines a CListView derived class.  This the bottom pane of our GUI which
//	shows event messages.
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


#ifndef _EVENTVIEW_H
#define _EVENTVIEW_H

class CKEvent;


// **************************************************************************
class CKEventView : public CListView
	{
	protected:
		CKEventView ();           // protected constructor used by dynamic creation
		DECLARE_DYNCREATE (CKEventView)

	// Attributes
	public:

	// Operations
	public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKEventView)
	public:
		virtual BOOL Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext = NULL);
	protected:
		virtual void OnDraw (CDC *pDC);      // overridden to draw this view
		virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	protected:
		CCriticalSection m_csLog;

		CKEvent **m_pEventList;
		int m_cnEvents;
		int m_cnAllocEvents;

		CKEvent **m_pPendingEventList;
		int m_cnPendingEvents;
		int m_cnPendingAllocEvents;

		bool m_bAutoScroll;
		bool m_bLogErrorsOnly;

		CImageList m_cImageList;

	// Implementation
	public:
		void LogMsg (EVENTTYPE eType, LPCTSTR lpszMessage);
		
		int GetEventCount () {return (m_cnEvents);}
		bool LogErrorsOnly () {return (m_bLogErrorsOnly);}

	protected:
		bool AddEvent (CKEvent *pEvent);

	protected:
		virtual ~CKEventView ();
	#ifdef _DEBUG
		virtual void AssertValid () const;
		virtual void Dump (CDumpContext& dc) const;
	#endif

		// Generated message map functions
	protected:
		//{{AFX_MSG(CKEventView)
		afx_msg void OnDestroy ();
		afx_msg void OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnClear ();
		afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnTimer (UINT nIDEvent);
		afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnLogErrorsOnly ();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP ()
	};

// **************************************************************************
// Define a class to contain information about an event and to aid in 
// creating strings describing the event.
// **************************************************************************
class CKEvent
	{
	public:
		CKEvent () {}
		CKEvent (LPCTSTR lpszMessage, EVENTTYPE eType)
			{
			m_strMsg = lpszMessage;
			m_eType = eType;
			m_cTimeStamp.Assign ();
			}

		LPCTSTR GetMessage () {return (m_strMsg);}
		EVENTTYPE GetType () {return (m_eType);}

		void FormatDate (LPTSTR lpsz, int cnBytes) {m_cTimeStamp.FormatDate (lpsz, cnBytes);}
		void FormatTime (LPTSTR lpsz, int cnBytes) {m_cTimeStamp.FormatTime (lpsz, cnBytes, true);}

	private:
		CString m_strMsg;
		CTimeStamp m_cTimeStamp;
		EVENTTYPE m_eType;
	};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _EVENTVIEW_H
