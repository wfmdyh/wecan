// **************************************************************************
// document.h
//
// Description:
//	Defines a CDocument derived class.  This is the document part of our MFC
//	SDI document/view architecture.
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


#ifndef _DOCUMENT_H
#define _DOCUMENT_H

class CKServer;
class CKGroup;
class CKItem;


// **************************************************************************
class CKDocument : public CDocument
	{
	protected: // create from serialization only
		CKDocument ();
		DECLARE_DYNCREATE (CKDocument)

	// Attributes
	public:

	// Operations
	public:
		void SetModified (bool bModified = true);

		bool IsLocked ();
		void SetLocked (bool bSet);

		BOOL RouteCmdMsg (CView *pActiveView, UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo);

		// add/modify/remove server object
		void AddServer ();
		void AddServer (CKServer *pServer, bool bConnect = false);
		void EditServer (CKServer *pServer);
		void RemoveServer (CKServer *pServer);
		void ConnectServer (CKServer *pServer);
		void DisconnectServer (CKServer *pServer);

		DWORD GetServerCount () {return (m_cdwServers);}

		// add/modify/delete group
		void AddGroup ();
		void AddGroup (CKGroup *pGroup);
		void CloneGroup ();
		void EditGroup (CKGroup *pGroup);
		void RemoveGroup (CKGroup *pServer);
		
		// add/delete item
		void AddItem ();
		void AddItems (CObArray &cList, DWORD dwCount);
		void RemoveItems (CObArray &cList, DWORD dwCount);

		// group view management
		void SetSelectedServerGroup (CKServer *pServer, CKGroup *pGroup) 
			{
			m_pCurSelServer = pServer;
			m_pCurSelGroup = pGroup;

			if (pGroup)
				{
				ASSERT (pServer == NULL);
				UpdateAllViews (NULL, HINT_SELECT_GROUP, (CObject *)pGroup);
				}
			else
				{
				ASSERT (pServer != NULL);
				ASSERT (pGroup == NULL);
				UpdateAllViews (NULL, HINT_SELECT_SERVER, (CObject *)pServer);
				}
			}

		CKServer* GetServerHead () {return (m_pServerHead);}
		CKServer* GetSelectedServer () {return (m_pCurSelServer);}
		CKGroup* GetSelectedGroup () {return (m_pCurSelGroup);}

		// notifications
		void OnServerShutdown (CKServer *pServer);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CKDocument)
	public:
		virtual BOOL OnNewDocument ();
		virtual void Serialize (CArchive& ar);
		virtual void DeleteContents ();
		virtual BOOL OnOpenDocument (LPCTSTR lpszPathName);
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CKDocument ();
	#ifdef _DEBUG
		virtual void AssertValid () const;
		virtual void Dump (CDumpContext& dc) const;
	#endif

	private:
		typedef struct _tagWORKERTHREADARG
			{
			enum TASK
				{
				START_SINGLE_SERVER,
				START_MULTIPLE_SERVER,
				STOP_SINGLE_SERVER,
				STOP_MULTIPLE_SERVER,
				ADD_ITEMS,
				};
			
			_tagWORKERTHREADARG ()
				{
				pvObjectA = NULL;
				pvObjectB = NULL;
				}

			TASK eTask;
			void *pvObjectA;
			void *pvObjectB;
			} WORKERTHREADARG;

		void RunWorkerThread (WORKERTHREADARG *pArg);
		static unsigned _stdcall WorkerThread (void *pvArgs);

	protected:
		// server management
		CKServer *m_pServerHead;
		DWORD m_cdwServers;

		// group view management
		CKServer *m_pCurSelServer;
		CKGroup *m_pCurSelGroup;

		// document locked for edits
		CCriticalSection m_csDoc;
		bool m_bLocked;

	// Generated message map functions
	protected:

		DECLARE_MESSAGE_MAP ()
	};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _DOCUMENT_H
