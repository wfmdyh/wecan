// **************************************************************************
// server.h
//
// Description:
//	Defines the CKServer class.
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


#ifndef _SERVER_H
#define _SERVER_H

class CKGroup;
class IKShutdownSink;


// **************************************************************************
class CKServer : public CObject  
	{
	public:
		// construction/destruction
		CKServer ();
		CKServer (CString &strProgID, CString &strRemoteMachine);

		~CKServer ();

	public:
		// property accessor/manipulators
		void SetProgID (CString &strProgID);
		LPCTSTR GetProgID () {return (m_strProgID);}

		void SetRemoteMachine (CString &strRemoteMachine);
		LPCTSTR GetRemoteMachine () 
			{
			if (m_strRemoteMachine.IsEmpty ())
				return (NULL);

			return (m_strRemoteMachine);
			}

		bool IsConnected () {return (m_bConnected);}
		bool IsKepServerEx () {return (m_bfFlags.bIsKepServerEx);}
		bool IsAlive ();

		// OPC specifics
		bool Connect ();
		bool Connect (CString &strProgID, CString &strRemoteMachine);
		void Disconnect ();

		void ShutdownRequest (LPCTSTR lpszReason);
		
		// serialization / loading project helpers
		virtual void Serialize (CArchive &ar);

		void Start ();
		void Stop ();

		// cut/copy/paste
		void Copy (CFixedSharedFile &sf);
		void Paste (CFixedSharedFile &sf);

		// list management
		void SetPrev (CKServer *pPrev) {m_pPrev = pPrev;}
		CKServer* GetPrev () {return (m_pPrev);}

		void SetNext (CKServer *pNext) {m_pNext = pNext;}
		CKServer* GetNext () {return (m_pNext);}

		// group management
		void AddGroup (CKGroup *pGroup, bool bLoadingProject = false);
		void AddClonedGroup (CKGroup *pClone);

		void RemoveGroup (CKGroup *pGroup, bool bDelete = true);
		void RemoveAllGroups (bool bDelete = true);

		CKGroup* GetGroupHead () {return (m_pGroupHead);}

		bool GenerateGroupName (CString &strName);
		bool FindGroup (LPCTSTR lpszName);
		
		CKGroup* GetGroup (LPCTSTR lpszName);

		// GUI management
		void SetGUIHandle (HTREEITEM hItem) {m_hTreeItem = hItem;}
		HTREEITEM GetGUIHandle () {return (m_hTreeItem);}

		// supported interfaces
		bool IsIServerSupported () {return (m_pIServer != NULL);}
		bool IsICommonSupported () {return (m_pICommon != NULL);}
		bool IsIConnectionPointContainerSupported () {return (m_pIConnPtContainer != NULL);}
		bool IsIItemPropertiesSupported () {return (m_pIItemProps != NULL);}
		bool IsIBrowsingSupported () {return (m_pIBrowse != NULL);}
		bool IsIServerPublicGroupsSupported () {return (m_pIPublicGroups != NULL);}
		bool IsIPersistFileSupported () {return (m_pIPersistFile != NULL);}

		IOPCServer* GetIServer () {return (m_pIServer);}
		IOPCBrowseServerAddressSpace* GetIBrowse () {return (IsAlive () ? m_pIBrowse : NULL);}
		IOPCItemProperties* GetIItemProps () {return (m_pIItemProps);}
	private:
		// OPC specifics
		HRESULT GetCLSID (CLSID &clsid);

	private:
		// properties
		CString m_strProgID;			
		CString m_strRemoteMachine;

		typedef struct _flags
			{
			DWORD bIsKepServerEx			: 1;
			DWORD Reserved					: 31;
			} FLAGS;

		FLAGS m_bfFlags;

		// OPC specifics
		MULTI_QI m_arrMultiQI [7];	// array of interfaces we will query for each server
		
		IOPCServer *m_pIServer;		// interface pointers returned from CoCreateInstanceEx ()
		IOPCCommon *m_pICommon;
		IConnectionPointContainer *m_pIConnPtContainer;
		IOPCItemProperties *m_pIItemProps;
		IOPCBrowseServerAddressSpace *m_pIBrowse;
		IOPCServerPublicGroups *m_pIPublicGroups;
		IPersistFile *m_pIPersistFile;

		IKShutdownSink *m_pIShutdownSink;
		DWORD m_dwCookieShutdownSink;

		bool m_bConnected;			// our connect state

		// list management
		CKServer *m_pPrev;
		CKServer *m_pNext;

		// group management
		CKGroup *m_pGroupHead;
		DWORD m_cdwGroups;

		// GUI management
		HTREEITEM m_hTreeItem;
	};


#endif // _SERVER_H
