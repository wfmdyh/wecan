// **************************************************************************
// group.h
//
// Description:
//	Defines the CKGroup class.
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


#ifndef _GROUP_H
#define _GROUP_H

#include "mainwnd.h"

class CKServer;
class CKItem;
class CKAdviseSink;
class IKDataSink20;


// **************************************************************************
class CKGroup : public CObject  
	{
	public:
		// construction/destruction
		CKGroup (CKServer *pParent);
		~CKGroup ();

	public:
		// property accessor/manipulators
		void SetName (CString &strName) {m_strName = strName;}
		void SetName (LPCTSTR lpszName) {m_strName = lpszName;}
		LPCTSTR GetName () {return (m_strName);}

		void SetUpdateRate (DWORD dwRate) {m_dwUpdateRate = dwRate;}
		DWORD GetUpdateRate () {return (m_dwUpdateRate);}

		void SetLanguageID (LCID lcid) {m_dwLanguageID = lcid;}
		LCID GetLanguageID () {return (m_dwLanguageID);}

		void SetActive (BOOL bActive, BOOL bApply = FALSE) 
			{
			m_bActive = bActive;

			// apply change to server now
			if (bApply && m_pIGroupState)
				{
				DWORD dwRevRate; // for [out] parm
				m_pIGroupState->SetState (NULL, &dwRevRate, &m_bActive, NULL, NULL, NULL, NULL);

				// select the activated group
				CKMainWnd *pWnd = (CKMainWnd *) AfxGetMainWnd ();
				if (pWnd)
					pWnd->PostMessage (UM_SELECT_GROUP, 0, (LPARAM) this);

				// log status
				LogMsg (IDS_SET_GROUP_ACTIVE_STATE, bActive, GetName ());
				}
			}

		BOOL IsActive () {return (m_bActive);}

		void SetBias (long lBias) {m_lBias = lBias;}
		long GetBias () {return (m_lBias);}

		void SetDeadband (float fDeadband) {m_fDeadband = fDeadband;}
		float GetDeadband () {return (m_fDeadband);}

		void SetServerHandle (OPCHANDLE hServer) {m_hServer = hServer;}
		OPCHANDLE GetServerHandle () {return (m_hServer);}

		void SetValid (BOOL bValid) {m_bValid = bValid;}
		BOOL IsValid () {return (m_bValid);}

		void SetUpdateMethod (DWORD dwMethod) {m_dwUpdateMethod = dwMethod;}
		DWORD GetUpdateMethod () {return (m_dwUpdateMethod);}

		// flag accessor/manipulators
		void ForceDeletion (BOOL bSet) {m_bfFlags.bOnDeleteForceDeletion = bSet;}
		BOOL IsForceDeletion () {return (m_bfFlags.bOnDeleteForceDeletion);}

		// OPC Specifics
		void Initialize (LPUNKNOWN pUnk);
		void Uninitialize (bool bDelete = true);

		bool SetItemActiveState (CObArray &cItemList, DWORD cdwItems, bool bActive);

		void ReadSync (CObArray &cItemList, DWORD cdwItems, bool bDeviceRead, bool bPostMsg = true);
		void WriteSync (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems);

		void ReadAsync10 (CObArray &cItemList, DWORD cdwItems, bool bDeviceRead);
		void RefreshAsync10 (bool bDeviceRead);
		void WriteAsync10 (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems);

		void ReadAsync20 (CObArray &cItemList, DWORD cdwItems);
		void RefreshAsync20 (bool bDeviceRead);
		void WriteAsync20 (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems);

		CKGroup* Clone ();

		// serialization / loading project helpers
		virtual void Serialize (CArchive &ar);
		void Start ();

		void ExportCsv (CStdioFile &csv);
		void ImportCsv (CStdioFile &csv, CObArray &cItemList, DWORD &cdwItems);

		// cut/copy/paste
		void Copy (CFixedSharedFile &sf);
		void Paste (CFixedSharedFile &sf);

		// parent server access
		CKServer* GetParentServer () {return (m_pServer);}

		// list management
		void SetPrev (CKGroup *pPrev) {m_pPrev = pPrev;}
		CKGroup* GetPrev () {return (m_pPrev);}

		void SetNext (CKGroup *pNext) {m_pNext = pNext;}
		CKGroup* GetNext () {return (m_pNext);}

		// item management
		void AddItems (CObArray &cItemList, DWORD dwCount, bool bLoadingProject = false);

		void RemoveItems (CObArray &cItemList, DWORD dwCount, bool bDelete = true);
		void RemoveAllItems (bool bDelete = true);

		CKItem* GetItemHead () {return (m_pItemHead);}
		DWORD GetItemCount () {return (m_cdwItems);}

		// GUI management
		void SetGUIHandle (HTREEITEM hItem) {m_hTreeItem = hItem;}
		HTREEITEM GetGUIHandle () {return (m_hTreeItem);}

		bool IsIGroupStateMgtSupported () {return (m_pIGroupState != NULL);}
		bool IsIPublicGroupStateMgtSupported () {return (m_pIPublicGroupState != NULL);}
		bool IsIItemMgtSupported () {return (m_pIItemMgt != NULL);}
		bool IsISyncIOSupported () {return (m_pISync != NULL);}
		bool IsIAsyncIOSupported () {return (m_pIAsync != NULL);}
		bool IsIDataObjectSupported () {return (m_pIDataObject != NULL);}
		bool IsIAsyncIO2Supported () {return (m_pIAsync2 != NULL);}
		bool IsIConnectionPointContainerSupported () {return (m_pIConnPtContainer != NULL);}

		IOPCItemMgt* GetIItemMgt () {return (m_pIItemMgt);}
		IOPCGroupStateMgt* GetIGroupStateMgt () {return (m_pIGroupState);}
		IOPCSyncIO* GetISyncIO () {return (m_pISync);}

	private:
		bool MapStringValToVariant (CString &strValue, VARIANT &vtVal, VARTYPE vtType);
		
		typedef enum _tagGETARRELEMRET
			{
			tElement = 0,
			tEndRow,
			tInvalid,
			tOverflow,
			tDone
			} GETARRELEMRET;

		GETARRELEMRET GetArrayElement (LPCTSTR szInBuff, int *pnStart, LPTSTR szOutBuff, int nBuffSize);
		bool MapStringValToArrayVariant (CString &strValue, VARIANT *pvtSrc, VARIANT *pvtDst);

		void AddItemToList (CKItem *pItem);
		void RemoveItemFromList (CKItem *pItem);
		
	private:
		// properties
		CString m_strName;		// group name
		
		DWORD m_dwUpdateRate;	// update rate in milliseconds
		LCID m_dwLanguageID;	// language ID

		BOOL m_bActive;			// active state
		long m_lBias;			// time bias in minutes
		float m_fDeadband;		// percent deadband

		OPCHANDLE m_hServer;	// server handle for this group
		BOOL m_bValid;			// TRUE if successfully added to the OPC server
		DWORD m_dwUpdateMethod;	// update method used by this group (see globals.h)

		typedef struct _flags
			{
			DWORD bOnDeleteForceDeletion	: 1;	// TRUE if the server should force deletion of group even if references exists
//			DWORD bOnDeleteRemoveItems		: 1;	// TRUE if the client should remove items before remove group
			DWORD Reserved					: 31;
			} FLAGS;

		FLAGS m_bfFlags;

		// OPC specifics
		IOPCGroupStateMgt *m_pIGroupState;
		IOPCPublicGroupStateMgt *m_pIPublicGroupState;
		IOPCItemMgt *m_pIItemMgt;
		IOPCSyncIO *m_pISync;
		IOPCAsyncIO *m_pIAsync;
		IDataObject *m_pIDataObject;
		IOPCAsyncIO2 *m_pIAsync2;
		IConnectionPointContainer *m_pIConnPtContainer;

		IKDataSink20 *m_pIDataSink20;
		DWORD m_dwCookieDataSink20;

		CKAdviseSink *m_pIAdviseSink;
		DWORD m_dwCookieRead;
		DWORD m_dwCookieWrite;

		// parent server
		CKServer *m_pServer;

		// list management
		CKGroup *m_pPrev;
		CKGroup *m_pNext;

		// item management
		CKItem *m_pItemHead;
		DWORD m_cdwItems;

		// GUI management
		HTREEITEM m_hTreeItem;
	};


#endif // _GROUP_H
