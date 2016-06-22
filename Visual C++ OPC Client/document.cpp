// **************************************************************************
// document.cpp
//
// Description:
//	Implements a CDocument derived class.  This is the document part of our
//  MFC SDI document/view architecture.
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


#include "stdafx.h"
#include "opctestclient.h"
#include "document.h"
#include "server.h"
#include "serverpropertysheet.h"
#include "group.h"
#include "grouppropertysheet.h"
#include "item.h"
#include "itemadddlg.h"

#define LARGE_ADDITEM_COUNT		64


/////////////////////////////////////////////////////////////////////////////
// CKDocument
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKDocument, CDocument)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKDocument, CDocument)
END_MESSAGE_MAP ()


/////////////////////////////////////////////////////////////////////////////
// CKDocument construction/destruction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKDocument ()
//
// Description:
//	Constructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKDocument::CKDocument ()
	{
	// Initialize member variables:
	m_pServerHead = NULL;
	m_cdwServers = 0;

	m_pCurSelServer = NULL;
	m_pCurSelGroup = NULL;

	m_bLocked = false;
	}

// **************************************************************************
// ~CKDocument ()
//
// Description:
//	Destructor
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKDocument::~CKDocument ()
	{
	}

// **************************************************************************
// SetLocked ()
//
// Description:
//	Locks the document object.
//
// Parameters:
//	bool		bSet		Set to "true" to lock, "false" to unlock
//
// Returns:
//  void
// **************************************************************************
void CKDocument::SetLocked (bool bSet)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDoc);

	// Use a flag to indicate locked state:
	m_bLocked = bSet;
	}

// **************************************************************************
// IsLocked ()
//
// Description:
//	Returns the locked state of document object.
//
// Parameters:
//  none
//
// Returns:
//  bool - true if locked.
// **************************************************************************
bool CKDocument::IsLocked ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDoc);

	// Return locked state:
	return (m_bLocked);
	}

// **************************************************************************
// OnNewDocument ()
//
// Description:
//	New document event handler.
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKDocument::OnNewDocument ()
	{
	// Perform default processing:
	if (!CDocument::OnNewDocument ())
		return (FALSE);

	// SDI documents reuse this document so perform reinitialization:
	SetModified (FALSE);
	return (TRUE);
	}

// **************************************************************************
// OnOpenDocument ()
//
// Description:
//	Open document event handler.
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKDocument::OnOpenDocument (LPCTSTR lpszPathName) 
	{
	// Perform default processing:
	if (!CDocument::OnOpenDocument (lpszPathName))
		return (FALSE);

	// Must do some additional work if we have a server object:
	if (m_pServerHead)
		{
		// Update status bar text to indicate we are loading project:
		CKStatusBarText cText (IDS_LOADING_PROJECT);

		// Update all views so the user can see progress:
		UpdateAllViews (NULL, HINT_LOAD_PROJECT, m_pServerHead);

		// Build a list of servers to load to pass along to a worker thread 
		// for processing:
		CObArray cServerList;

		// Catch memory exceptions that could get thrown by object array:
		try
			{
			// Allocate memory for array of server objects:
			cServerList.SetSize (m_cdwServers);

			// Create a CKServer pointer and set it to first server in linked list:
			CKServer *pServer = m_pServerHead;

			// Loop over servers in linked list:
			for (DWORD dw = 0; dw < m_cdwServers; dw++)
				{
				// Put pointer to server in object array:
				ASSERT (pServer);
				cServerList [dw] = (CObArray *)pServer;

				// Get next server in linked list:
				pServer = pServer->GetNext ();
				}

			ASSERT (pServer == NULL);	// list and count in sync ?
			}
		
		catch (CMemoryException *e)
			{
			// If memory exception thrown, delet exception object and return FALSE
			// to indicate failure.
			e->Delete ();
			return (FALSE);
			}

		// Fill a structre to pass along to worker thread.  Structure will
		// contain a pointer to out list of servers and task specification
		// (start multiple servers).
		WORKERTHREADARG tArg;
		tArg.eTask = WORKERTHREADARG::START_MULTIPLE_SERVER;
		tArg.pvObjectA = (void *)&cServerList;

		// Run the worker thread to start all server connections:
		RunWorkerThread (&tArg);
		}

	// If we make it here, all went OK, so return TRUE:
	return (TRUE);
	}

// **************************************************************************
// DeleteContents ()
//
// Description:
//	Reset the document object for reuse.  This includes disconnect from all
//	OPC servers and deleting all server objects.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::DeleteContents () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;
	
	// Update status bar text to indicate that we are unloading project:
	CKStatusBarText cText (IDS_UNLOADING_PROJECT);
	
	// Notify all views that the project is closing so that no request for
	// server/group/item is made (as in OnGetDispInfo) for a deleted object:
	UpdateAllViews (NULL, HINT_CLOSE_PROJECT, NULL);

	// Free all server connections maintained by the document.  Start with
	// head of linked list and work our way to end:
	while (m_pServerHead)
		{
		// Need to make a copy of current head of linked list:
		CKServer *pServer = m_pServerHead;

		// Reset linked list head to next server:
		m_pServerHead = pServer->GetNext ();

		// Delete server (previously the head of linked list):
		RemoveServer (pServer);
		}

	// Check that we deleted all servers in linked list (debug only):
	ASSERT (m_cdwServers == 0);

	// Invalidate currently selected server and group:
	m_pCurSelServer = NULL;
	m_pCurSelGroup = NULL;

	// Default processing:
	CDocument::DeleteContents ();
	}

// **************************************************************************
// SetModified ()
//
// Description:
//	Set the document modified flag.  Update title bar to signal that document
//	has been modified.
//
// Parameters:
//  bool		bModified		Set to "true" if document has been modified.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::SetModified (bool bModified /* = true */)
	{
	// If we are currently unmodified, we may need to add or remove an
	// asterisk from title bar. (The framework automatically clears the 
	// asterisk when saving the document):
	if (!IsModified ())
		{
		// Get document title:
		CString strTitle = GetTitle ();

		// If we are modifying, see if we should add asterisk:
		if (bModified)
			{
			// If we can't find an asterisk in title, we need to add one:
			if (strTitle.Find (_T(" *")) < 0)
				{
				strTitle += " *";
				SetTitle (strTitle);
				}
			}

		// Otherwise, remove asterisk if any:
		else
			{
			// If we can find an asterisk in title, we need to remove it:
			if (strTitle.Find (_T(" *")) >= 0)
				{
				strTitle.TrimRight (_T(" *"));
				SetTitle (strTitle);
				}
			}
		}

	// Set modified flag:
	SetModifiedFlag (bModified);
	}

// **************************************************************************
// OnServerShutdown ()
//
// Description:
//	Server shutdown event handler.
//
// Parameters:
//  CKServer		*pServer		Pointer to server that has given us a
//									shutdown notification.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::OnServerShutdown (CKServer *pServer)
	{
	ASSERT (pServer != NULL);

	// Shutdown the server:
	pServer->Stop ();

	// Refresh our group view so that new active status is shown:
	UpdateAllViews (NULL, HINT_REFRESH_GROUPVIEW, NULL);
	}


////////////////////////////////////////////////////////////////////////////
// CKDocument serialization
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Serialize ()
//
// Description:
//	Save or load project settings.
//
// Parameters:
//  CArchive		&ar			The archive to save or load project settings.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::Serialize (CArchive &ar)
	{
	// Be prepared to catch exceptions:
	try
		{
		// Save project:
		if (ar.IsStoring ())
			{
			// Save the number of server object first so we will know how
			// many to load later:
			ar << m_cdwServers;

			// Now serialize each server object.  Start with head of linked
			// list and work our way to end:
			CKServer *pServer = m_pServerHead;

			// Keep looping until we hit end of linked list, indicated by 
			// a NULL pointer:
			while (pServer)			
				{
				// Serialize server object:
				pServer->Serialize (ar);

				// Get pointer to next server in linked list:
				pServer = pServer->GetNext ();
				}
			}

		// Else load project:
		else
			{
			// First get the number of servers to load:
			DWORD cdwServers;
			ar >> cdwServers;

			// Loop over servers:
			for (DWORD dwIndex = 0; dwIndex < cdwServers; dwIndex++)
				{
				// Create a reusable pointer to a CKServer object:
				CKServer *pServer = NULL;

				// Instantiate a new CKServer object:
				pServer = new CKServer ();

				// Load it's settings from archive:
				pServer->Serialize (ar);

				// Add the server to the project:
				AddServer (pServer);
				}
			}
		}

	// Catch archive exceptions.  Any object exceptions thrown will be
	// self-deleted by the object so we do not have to worry about memory
	// leaks at this outer-level.
	catch (CArchiveException *ae)
		{
		// Make a copy of the cause of the exception before we delete it:
		int nCause = ae->m_cause;

		// Delete the exception:
		ae->Delete ();
		
		// Re-throw the exception to be processed by the framework:
		AfxThrowArchiveException (nCause);
		}

	// Catch any other kind of excetion and re-throw it to be processed
	// by the framework:
	catch (...)
		{
		AfxThrowArchiveException (CArchiveException::generic);		
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKDocument diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CKDocument::AssertValid () const
	{
	CDocument::AssertValid ();
	}

void CKDocument::Dump (CDumpContext &dc) const
	{
	CDocument::Dump (dc);
	}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKDocument commands
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// RouteCmdMsg ()
//
// Description:
//	Routes a message onto all inactive views or until one of the views 
//	processes the message.
//
// Parameters:
//	CView				*pActiveView	Pointer to the active view.
//	UINT				nID				Specifies the container for the 
//										  command identifier. 
//	int					nCode			Identifies the command notification
//										  code.
//	void				*pExtra			Used according to the value of 
//										  nCode.
//	AFX_CMDHANDLERINFO	*pHandlerInfo	If not NULL, OnCmdMsg fills in the
//										  pTarget and pmf members of the 
//										  pHandlerInfo structure instead of
//										  dispatching the command. Typically, 
//										  this parameter should be NULL.
//
// Returns:
//  BOOL - TRUE if message was processed.
// **************************************************************************
BOOL CKDocument::RouteCmdMsg (CView *pActiveView, UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
	{
	// Get the position of the first view in the list of views associated
	// with this document:
	POSITION pos = GetFirstViewPosition ();

	// Exhaust all views.  When we get to end of list of views, pos will be NULL:
	while (pos != NULL)
		{
		// Get pointer to view at present POSITION (pos).  Function will
		// automatically reset pos to POSITION of next view, or NULL if 
		// end of list:
		CView *pNextView = GetNextView (pos);

		// By definition, we do no send message to active view:
		// (We shouldn't get called if active view processes the message.)
		if (pNextView != pActiveView)
			{
			// If the view processes the message then we are a success.  Return TRUE
			// to indicate message was processed:
			if (((CCmdTarget *) pNextView)->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo))
				return (TRUE);
			}
		}

	// If we make it here, then message not processed.  Return FALSE to
	// indicate that message was not processed.
	return (FALSE);
	}

// **************************************************************************
// AddServer ()
//
// Description:
//	Allows for the addition of a new server item to the document.  Posts a 
//	dialog to enter server attributes and attaches it to the document list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddServer ()
	{
	// Create a server property sheet object:
	CKServerPropertySheet psh;

	// Show as modal property sheet.  If user hits "OK" then add a server
	// to the project with the settings made in property sheet:
	if (psh.DoModal () == IDOK)
		{

			
		// Get pointer to server object created and configured in property
		// sheet:
		CKServer *pServer = psh.GetServer ();

		// Add the server to our list:
		AddServer (pServer);

		// Notify all views that new server has been addeded:
		UpdateAllViews (NULL, HINT_ADD_SERVER, pServer);

		// Set modified flag:
		SetModified ();
		}
	}

// **************************************************************************
// AddServer ()
//
// Description:
//	Adds a server that was created during document load from disk or during a
//	cut and paste operation to the document.  
//
// Parameters:
//	CKServer	*pServer		Pointer to server object to add.
//	bool		bConnect		Set to true to connect to server after add.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddServer (CKServer *pServer, bool bConnect /* = false */)
	{
	// Check that pointer to server is not NULL (for debug only):
	ASSERT (pServer != NULL);

	// Add the server to head of linked list list.

	// Next item will be previous head of linked list:
	pServer->SetNext (m_pServerHead);

	// If we had a head to the linked list, it's previous item will be
	// new server:
	if (m_pServerHead)	
		m_pServerHead->SetPrev (pServer);
		
	// New server is now the head of the linked list:
	m_pServerHead = pServer;

	// Increment the server count:
	++m_cdwServers;			
				
	// Connect to the OPC Server if asked:
	if (bConnect)
		{
		// Start the server:
		pServer->Start ();

		// Update views to indicate new server connection status.
		// Architecture will assue that new server will get added
		// to views if we don't connect at this point.
		UpdateAllViews (NULL, HINT_ADD_SERVER_AND_GROUPS, pServer);

		// Set document modified flag:
		SetModified ();
		}
	}

// **************************************************************************
// EditServer ()
//
// Description:
//	Allows for the properties of a server item to modified.
//
// Parameters:
//	CKServer	*pServer		Pointer to server object to edit.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::EditServer (CKServer *pServer)
	{
	// Check that pointer to server is not NULL (for debug only):
	ASSERT (pServer != NULL);

	// Create a server property sheet object, and give it pointer to the 
	// CKServer object to edit:
	CKServerPropertySheet psh (pServer);
	
	// Show as modal property sheet.  If user hits "OK", we need to set
	// document modified flag.  Property sheet object will apply edits to
	// server object if "OK" is hit.
	if (psh.DoModal () == IDOK)
		SetModified ();
	}

// **************************************************************************
// RemoveServer ()
//
// Description:
//	Allows for a server item to be removed from the document.
//
// Parameters:
//	CKServer	*pServer		Pointer to server object to remove.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::RemoveServer (CKServer *pServer)
	{
	// Check that pointer to server is not NULL (for debug only):
	ASSERT (pServer != NULL);

	// Remove the server from our linked list:

	// Will need to save next and previous items in list before we delete 
	// server object:
	CKServer *pPrev = pServer->GetPrev ();
	CKServer *pNext = pServer->GetNext ();
	
	// If server had previous item, it's next will be server's next:
	if (pPrev)
		pPrev->SetNext (pNext);

	// If server had a next item, it's previous will be server's previous:
	if (pNext)
		pNext->SetPrev (pPrev);

	// If server was head of linked list, new head will be server's next:
	if (pServer == m_pServerHead)
		m_pServerHead = pNext;

	// Decrement the server count:
	--m_cdwServers;

	// If there are no servers left, invalidate current server and group
	// selections:
	if (!m_cdwServers)
		{
		m_pCurSelServer = NULL;
		m_pCurSelGroup = NULL;
		}

	// Notify all views that server has been removed:
	UpdateAllViews (NULL, HINT_REMOVE_SERVER, pServer);

	// Set the document modified flag:
	SetModified ();

	// We are now free to delete the server object:
	delete pServer;
	}

// **************************************************************************
// ConnectServer ()
//
// Description:
//	Connects to a server, adds all groups and items.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::ConnectServer (CKServer *pServer)
	{
	// Check that pointer to server is not NULL (for debug only):
	ASSERT (pServer != NULL);

	// Update status bar text to indicate that we are connecting::
	CKStatusBarText cText (IDS_CONNECTING);

	// Create and fill a structure to pass along to worker thread.  Structure
	// will contain pointer to server to connect to and specify a "start
	// single server" task:
	WORKERTHREADARG tArg;
	tArg.eTask = WORKERTHREADARG::START_SINGLE_SERVER;
	tArg.pvObjectA = (void *)pServer;

	// Run a worker thread to start the server:
	RunWorkerThread (&tArg);

	// Update group view to indicate new server status:
	UpdateAllViews (NULL, HINT_REFRESH_GROUPVIEW, NULL);
	}

// **************************************************************************
// DisconnectServer ()
//
// Description:
//	Disconnects a server, removes all groups and items.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::DisconnectServer (CKServer *pServer)
	{
	// Check that pointer to server is not NULL (for debug only):
	ASSERT (pServer != NULL);

	// Update status bar text to indicate that we are disconnecting:
	CKStatusBarText cText (IDS_DISCONNECTING);

	// Create and fill a structure to pass along to worker thread.  Structure
	// will contain pointer to server to disconnect to and specify a "stop
	// single server" task
	WORKERTHREADARG tArg;
	tArg.eTask = WORKERTHREADARG::STOP_SINGLE_SERVER;
	tArg.pvObjectA = (void *)pServer;

	// Run a worker thread to disconnect the server:
	RunWorkerThread (&tArg);

	// Update group view to indicate new server status:
	UpdateAllViews (NULL, HINT_REFRESH_GROUPVIEW, NULL);
	}

// **************************************************************************
// AddGroup ()
//
// Description:
//	Allows for the addition of a new group to a server item.  The group will
//	be attached the server's group list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddGroup ()
	{
	// Get the currently selected server object:
	CKServer *pServer = GetSelectedServer ();
	ASSERT (pServer != NULL);

	// Create a group property sheet.  Pass it a pointer to the currently
	// selected server so that it can add the new group to it.
	CKGroupPropertySheet psh (pServer);

	// Show as modal property sheet.  If user hits "OK", we need to set
	// document modified flag.  Property sheet object will add new group to
	// server object if "OK" is hit.  
	if (psh.DoModal () == IDOK)
		{
		// Notify all views that new group has been added:
		UpdateAllViews (NULL, HINT_ADD_GROUP, psh.GetGroup ());

		// Set document modified flag:
		SetModified ();
		}
	}

// **************************************************************************
// AddGroup ()
//
// Description:
//	Adds a group that was created during document load from disk or during a
//	cut and paste operation to the document.
//
// Parameters:
//	CKGroup		*pGroup			Pointer to group object
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddGroup (CKGroup *pGroup)
	{
	// Get the currently selected server object:
	CKServer *pServer = GetSelectedServer ();
	ASSERT (pGroup != NULL);
	ASSERT (pServer != NULL);

	// Get current name of group:
	CString strName (pGroup->GetName ());

	// Generate a unique name for the group:
	if (pServer->GenerateGroupName (strName))
		pGroup->SetName (strName);

	// Add the group to the selected server:
	pServer->AddGroup (pGroup);

	// Start the group (add all OPC items):
	pGroup->Start ();

	// Notify all views that new group has been added:
	UpdateAllViews (NULL, HINT_ADD_GROUP, pGroup);

	// Set the document modified flag:
	SetModified ();
	}

// **************************************************************************
// CloneGroup ()
//
// Description:
//	Allows for the cloning of a new group from an existing group. The new 
//	group will be attached the server's group list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::CloneGroup ()
	{
	// Get currently selected group object:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Clone group and save pointer to clone:
	CKGroup *pNewGroup = pSelGroup->Clone ();

	// If group was sucessfully cloned, indicated by non-NULL pointer, then
	// we need to update group view and set document modified flag:
	if (pNewGroup != NULL)
		{
		UpdateAllViews (NULL, HINT_ADD_GROUP, pNewGroup);
		SetModified ();
		}
	}

// **************************************************************************
// EditGroup ()
//
// Description:
//	Allows for the propeties of a group to be modified.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::EditGroup (CKGroup *pGroup)
	{
	// Check that pointer to group is not NULL (for debug only):
	ASSERT (pGroup != NULL);

	// Create a group property sheet.  Pass it a pointer of group object to 
	// edit:
	CKGroupPropertySheet psh (pGroup);

	// Show as modal property sheet.  If user hits "OK", we need to update
	// group view and set document modified flag.  Property sheet object will
	// apply edits to group object if "OK" is hit.   
	if (psh.DoModal () == IDOK)
		{
		SetModified ();
		UpdateAllViews (NULL, HINT_REFRESH_GROUPVIEW, NULL);
		}
	}

// **************************************************************************
// RemoveGroup ()
//
// Description:
//	Allows for the removal of a group from a server item.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::RemoveGroup (CKGroup *pGroup)
	{
	// Check that pointer to group is not NULL (for debug only):
	ASSERT (pGroup != NULL);

	// Notify all views that group has been removed:
	UpdateAllViews (NULL, HINT_REMOVE_GROUP, pGroup);

	// Set document modified flag:
	SetModified ();

	// Get the server this group belongs to:
	CKServer *pServer = pGroup->GetParentServer ();
	ASSERT (pServer != NULL);

	// Remove the group from the server:
	pServer->RemoveGroup (pGroup);
	}

// **************************************************************************
// AddItem ()
//
// Description:
//	Allows for the addition of a group's item(s).
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddItem ()
	{
	// Get currently selected group:
	CKGroup *pGroup = GetSelectedGroup ();
	ASSERT (pGroup != NULL);

	// Get pointer to server that selected group belongs to:
	CKServer *pServer = pGroup->GetParentServer ();
	ASSERT (pServer != NULL);

	// Create an item add dialog.  Pass it a pointer to the group object
	// items are to be added to, and the associated OPC Server's browser
	// interface:
	CKItemAddDlg dlg (pGroup, pServer->GetIBrowse ());

	// Show as modal dialog.  If user hits "OK", we need to add items to 
	// project:
	if (dlg.DoModal () == IDOK)
		{
		// Get the number of items to add specified in dialog:
		int cnItems = dlg.GetItemCount ();

		// If number of items is non-zero, then add them to project:
		if (cnItems > 0)
			{
			// Get an object array containing the list of items to be added:
			CObArray &cList = dlg.GetItemList ();

			// If the number of items is large, then use a worker thread
			// to add them:
			if (cnItems > LARGE_ADDITEM_COUNT)
				{
				// Update status bar to indicate that we are adding items:
				CKStatusBarText cText (IDS_ADDING_ITEMS);

				// Create and fill a structure to pass to worker thread that
				// contains the items to add and specifies the "add items" task:
				WORKERTHREADARG tArg;
				tArg.eTask = WORKERTHREADARG::ADD_ITEMS;
				tArg.pvObjectA = (void *)&cList;
				tArg.pvObjectB = (void *)&cnItems;

				// Run a worker thread to add the items:
				RunWorkerThread (&tArg);
				}

			// Else if number of items is small, add them directly:
			else
				pGroup->AddItems (cList, cnItems);
			
			// Notify all views that item has been added:
			UpdateAllViews (NULL, HINT_ADD_ITEM, &cList);

			// Set document modified flag:
			SetModified ();
			}
		}
	}

// **************************************************************************
// AddItem ()
//
// Description:
//	Allows for the addition of a group's item(s).
//
// Parameters:
//	CObArray	&cList		Object array of items to add.
//	DWORD		dwCount		Number of items in cList.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::AddItems (CObArray &cList, DWORD dwCount)
	{
	// Get the currently selected group:
	CKGroup *pGroup = GetSelectedGroup ();
	ASSERT (pGroup != NULL);

	// Add the items to the group:
	pGroup->AddItems (cList, dwCount);

	// Notify all views that items have been added:
	UpdateAllViews (NULL, HINT_ADD_ITEM, &cList);

	// Set the document modified flag:
	SetModified ();
	}

// **************************************************************************
// RemoveItems ()
//
// Description:
//	Allows for the removal of items on the selected group.
//
// Parameters:
//	CObArray	&cList			Object array of items to remove.
//	DWORD		dwCount			Number of items in cList.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::RemoveItems (CObArray &cList, DWORD dwCount)
	{
	// Check that we specify a non-zero number of items (for debug only):
	ASSERT (dwCount > 0);

	
	// Get the currently selected group:
	CKGroup *pGroup = GetSelectedGroup ();
	ASSERT (pGroup != NULL);

	// Remove the items from the group:
	pGroup->RemoveItems (cList, dwCount);

	// Update the group view by reselecting the group:
	UpdateAllViews (NULL, HINT_SELECT_GROUP, (CObject *)pGroup);

	// Set the document modified flag:
	SetModified ();
	}

// **********************************************************************
// Worker Threads
// **********************************************************************

// **************************************************************************
// RunWorkerThread ()
//
// Description:
//	Creates a worker thread to perform a specified task.  This function will
//	block until worker thread completes its task.
//
// Parameters:
//	WORKERTHREADARG	*pArg	Pointer to a structure that contains information
//							  about task to be performed.
//
// Returns:
//  void
// **************************************************************************
void CKDocument::RunWorkerThread (WORKERTHREADARG *pArg)
	{
	// Load standard arrow and small hourglass cursor reource:
	HCURSOR hCursor = ::LoadCursor (NULL, IDC_APPSTARTING);
	ASSERT (hCursor);

	// Set the hourglass cursor:
	::SetCursor (hCursor);

	// Lock document to prevent edits while worker thread is doing its thing:
	SetLocked (true);

	// Start the worker thread:
	unsigned int uAddress = 0;
	HANDLE hThread = (HANDLE) _beginthreadex (NULL, 0, WorkerThread, pArg, 0, &uAddress);
	
	// If thread was created successfully, we need to wait for it to complete
	// its task.  This will guarentee that pArg will remain valid while the
	// thread completes its task.  We will need to process the message queue
	// while the thread runs to keep everybody up to date.
	if (hThread)
		{
		MSG stMsg;

		// Proccess messages while waiting for the thread to stop.  
		// hThread event will be set when thread stops.  Timeout every
		// 10 ms to process message queue, and then resume wait.
		while (WaitForSingleObject (hThread, 10) == WAIT_TIMEOUT)
			{
			// Process message queue by removing the next message, and then
			// dispatching it.  This will insure that important Windows 
			// notifiactions get processed by their target windows while this
			// thread is running.
			if (PeekMessage (&stMsg, NULL, 0, 0, PM_REMOVE))
				DispatchMessage (&stMsg);

			// Make sure hourglass cursor stays:
			::SetCursor (hCursor);
			}

		// When we break out of look, we know worker thread has stopped.
		// We can now close the thread handle:
		CloseHandle (hThread);
		}

	// We can now unlock document for edits:
	SetLocked (false);

	// Replace normal cursor:
	::DestroyCursor (hCursor);
	}

// **************************************************************************
// WorkerThread ()
//
// Description:
//	Worker thread function.
//
// Parameters:
//	void		*pvArgs		Pointer to a WORKERTHREADARG structure which
//							  describes task to be performed.
//
// Returns:
//  unsigned int - 0
// **************************************************************************
unsigned _stdcall CKDocument::WorkerThread (void *pvArgs)
	{
	// Cast argument to proper type so we can extract the required data
	// about the task we are to perform:
	WORKERTHREADARG *pWTA = (WORKERTHREADARG *)pvArgs;
	ASSERT (pWTA);

	// Execute specified task:
	switch (pWTA->eTask)
		{
		case WORKERTHREADARG::START_SINGLE_SERVER:
		case WORKERTHREADARG::STOP_SINGLE_SERVER:
			{
			// Extract pointer to server we are to start or stop from
			// argument structure:
			CKServer *pServer = (CKServer *)pWTA->pvObjectA;
			ASSERT (pServer);

			// Start or stop the server as the case may be:
			if (pWTA->eTask == WORKERTHREADARG::START_SINGLE_SERVER)
				pServer->Start ();
			else
				pServer->Stop ();
			}
			break;

		case WORKERTHREADARG::START_MULTIPLE_SERVER:
		case WORKERTHREADARG::STOP_MULTIPLE_SERVER:
			{
			// Extract the list (object array) of servers to start or stop 
			// from argument structure:
			CObArray *pList = (CObArray *)pWTA->pvObjectA;
			ASSERT (pList);

			// Get the number of server in the list:
			int cnServers = pList->GetSize ();
			CKServer *pServer = NULL;

			// Loop over the servers in the list:
			while (--cnServers >= 0)
				{
				// Get pointer to next server in list:
				pServer = (CKServer *) pList->GetAt (cnServers);
				ASSERT (pServer);

				// Start or stop that server as the case may be:
				if (pWTA->eTask == WORKERTHREADARG::START_MULTIPLE_SERVER)
					pServer->Start ();
				else
					pServer->Stop ();
				}
			}
			break;

		case WORKERTHREADARG::ADD_ITEMS:
			{
			// Extract the list (object array) of items to add from
			// argument structure:
			CObArray *pList = (CObArray *)pWTA->pvObjectA;
			ASSERT (pList);

			// Extract the number of items from the argument structure:
			int cnItems = *(int *)pWTA->pvObjectB;

			// Get pointer to first item from list.  We will use it to
			// get the group object these items will be added to:
			CKItem *pItem = (CKItem *)pList->GetAt (0);
			ASSERT (pItem);

			// Get group that we are adding the items to:
			CKGroup *pGroup = pItem->GetParentGroup ();
			ASSERT (pGroup);

			// Add the items to this group:
			pGroup->AddItems (*pList, cnItems);
			}
			break;

		default:
			// unhandled task ?
			ASSERT (FALSE);
			break;
		}

	return (0);
	}
