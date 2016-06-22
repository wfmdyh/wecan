// **************************************************************************
// groupview.cpp
//
// Description:
//	Implements a CTreeView derived class.  This the top left pane of our GUI
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


#include "stdafx.h"
#include "opctestclient.h"
#include "groupview.h"
#include "document.h"
#include "server.h"
#include "group.h"
#include "item.h"
#include "servergeterrorstringdlg.h"
#include "servergroupbynamedlg.h"
#include "serverenumgroupsdlg.h"

// Define string constsants for CSV file import/export common dialog:
static LPCTSTR lpszPaths = _T("Paths");
static LPCTSTR lpszOpenFile = _T("Open File");
static LPCTSTR lpszCSV = _T("csv");


/////////////////////////////////////////////////////////////////////////////
// CKGroupView
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKGroupView, CTreeView)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKGroupView, CTreeView)
	//{{AFX_MSG_MAP(CKGroupView)
	ON_COMMAND (ID_EDIT_NEWSERVER, OnNewServer)
	ON_COMMAND (ID_EDIT_NEWGROUP, OnNewGroup)
	ON_NOTIFY_REFLECT (TVN_GETDISPINFO, OnGetDispInfo)
	ON_COMMAND (ID_EDIT_PROPERTIES, OnProperties)
	ON_NOTIFY_REFLECT (TVN_SELCHANGED, OnSelectionChanged)
	ON_COMMAND (ID_EDIT_DELETE, OnDelete)
	ON_WM_RBUTTONDOWN ()
	ON_WM_LBUTTONDBLCLK ()
	ON_COMMAND (ID_TOOLS_GET_ERROR_STRING, OnGetErrorString)
	ON_COMMAND (ID_TOOLS_CLONE_GROUP, OnCloneGroup)
	ON_COMMAND (ID_TOOLS_GET_GROUP_BY_NAME, OnGetGroupByName)
	ON_COMMAND (ID_TOOLS_ENUMERATE_GROUPS, OnEnumerateGroups)
	ON_COMMAND (ID_TOOLS_CONNECT, OnConnect)
	ON_COMMAND (ID_TOOLS_DISCONNECT, OnDisconnect)
	ON_COMMAND (ID_TOOLS_RECONNECT, OnReconnect)
	ON_COMMAND (ID_EDIT_COPY, OnCopy)
	ON_COMMAND (ID_EDIT_CUT, OnCut)
	ON_COMMAND (ID_EDIT_PASTE, OnPaste)
	ON_WM_CHAR ()
	ON_COMMAND (ID_TOOLS_EXPORT_CSV, OnExportCsv)
	ON_COMMAND (ID_TOOLS_IMPORT_CSV, OnImportCsv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKGroupView ()
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
CKGroupView::CKGroupView ()
	{
	}

// **************************************************************************
// ~CKGroupView ()
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
CKGroupView::~CKGroupView ()
	{
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupView creation/drawing
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Create ()
//
// Description:
//	Creates the view window.
//
// Parameters:
//  LPCTSTR			lpszClassName	Names the Windows class.
//	LPCTSTR			lpszWindowName	Window name.
//	DWORD			dwStyle			Window stype.
//	const RECT		&rect			The size and position of the window.
//	CWnd			*pParentWnd		Parent window.
//	UINT			nID				The ID of the child window.
//	CCreateContext	*pContext		The create context of window.
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKGroupView::Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext) 
	{
	// Perform default processing.  Return FALSE if failure:
	if (!CWnd::Create (lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
		return (FALSE);

	// Initialize the image list for the items. The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 16x16 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	// Image number		Use
	//	0				Connected Server
	//	1				not used
	//	2				Active Group (not selected)
	//	3				Inactive Group (not selected)
	//	4				Active Group (selected)
	//	5				Inactive Group (selected)
	//	6				Disconnected Server
	//	7				Invalid Group
	//	8				not used
	//	9				not used
	m_cImageList.Create (IDB_GROUPIMAGES, 16, 4, RGB (255, 0, 255));
	m_cImageList.SetBkColor (CLR_NONE);
	GetTreeCtrl ().SetImageList (&m_cImageList, LVSIL_NORMAL);

	// Set the style we want for the tree:  Draw lines connecting child and
	// parent, has buttons to expand or collapse tree, draw line to root and
	// let root have button.
	GetTreeCtrl ().ModifyStyle (0, // Don't remove any styles
		TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT); // Add these styles
	
	// Return TRUE to inidcate success:
	return (TRUE);
	}

// **************************************************************************
// OnDraw ()
//
// Description:
//	OnDraw event handler.  This function needs to be implemented since it is
//  declared as a pure virtual function in base class.  We don't need to do
//  anything with it in this case.
//
// Parameters:
//  CDC			*pDC		Device context
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnDraw (CDC *pDC)
	{
	// This is how we get our document object in case we need to do something
	// with this function at some point:
	CDocument* pDoc = GetDocument ();
	}

// **************************************************************************
// OnUpdate ()
//
// Description:
//	Called when we the document has changed and we should update our view.
//
// Parameters:
//  CView		*pSender	Points to the view that modified the document, 
//							  or NULL if all views are to be updated.
//	LPARAM		lHint		Contains information about the modifications.
//							  Hints used here are defined in globals.h.
//	CObject		*pHint		Points to an object storing information about the 
//							  modifications.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnUpdate (CView *pSender, LPARAM lHint, CObject *pHint) 
	{
	switch (lHint)
		{
		case HINT_ADD_SERVER:
			ASSERT (pHint != NULL);		// pHint contains CKServer *
			Insert (TVI_ROOT, pHint);
			break;

		case HINT_REMOVE_SERVER:
			ASSERT (pHint != NULL);		// pHint contains CKServer *
			Delete (((CKServer *)pHint)->GetGUIHandle ());
			break;
			
		case HINT_ADD_GROUP:
			{
			ASSERT (pHint != NULL);		// pHint contains CKGroup *
			CKGroup *pGroup = (CKGroup *)pHint;

			CKServer *pParent = pGroup->GetParentServer ();
			ASSERT (pParent != NULL);

			Insert (pParent->GetGUIHandle (), pGroup);
			}
			break;

		case HINT_REMOVE_GROUP:
			ASSERT (pHint != NULL);		// pHint contains CKGroup *
			Delete (((CKGroup *)pHint)->GetGUIHandle ());
			break;

		case HINT_SELECT_GROUP:
			{
			// pHint contains CKGroup *

			// Get reference to link control:
			CTreeCtrl &cTree = GetTreeCtrl ();
			
			// Get handle of group:
			HTREEITEM hItem;
			if (pHint == NULL)	
				{
				// No group passed in, get the first one in the tree
				hItem = cTree.GetChildItem (TVI_ROOT);	// Get first child of root (i.e., server)
				hItem = cTree.GetChildItem (hItem);		// Get first child of parent (i.e., group)
				}

			else
				hItem = ((CKGroup*)pHint)->GetGUIHandle ();

			// Select the group:
			if (hItem != NULL)
				{
				// Select the group:
				cTree.SelectItem (hItem);

				// Force a repaint:
				cTree.Invalidate (false);
				cTree.UpdateWindow ();
				}
			}
			break;

		case HINT_ADD_SERVER_AND_GROUPS:
			{
			// pHint contains CKServer *
			ASSERT (pHint != NULL);
			CKServer *pServer = (CKServer *) pHint;

			// Insert the server:
			Insert (TVI_ROOT, pServer);

			// Now insert its groups:

			// We'll need the handle of parent server,
			HTREEITEM hParent = pServer->GetGUIHandle ();

			// and the head of its linked list of groups:
			CKGroup *pGroup = pServer->GetGroupHead ();

			// Loop over groups in linked list.  End of list will be indicated
			// by a NULL pointer.
			while (pGroup)
				{
				// Insert group item into the server branch of the tree:
				Insert (hParent, pGroup, false);

				// Move onto next group in linked list:
				pGroup = pGroup->GetNext ();
				}

			// Force a repaint:
			GetTreeCtrl ().Invalidate (false);
			GetTreeCtrl ().UpdateWindow ();
			}
			break;

		case HINT_LOAD_PROJECT:
			{
			// pHint contains CKServer * (head of linked list of servers)
			ASSERT (pHint != NULL);		
			CKServer *pServer = (CKServer *)pHint;

			// Loop over all servers in linked list.  End of list will be
			// indicated by a NULL pointer:
			while (pServer)
				{
				// Insert server item into the root of the tree:
				Insert (TVI_ROOT, pServer, false);

				// Insert server's groups:

				// We'll need the handle of the parent server,
				HTREEITEM hParent = pServer->GetGUIHandle ();

				// and the head of its linked list of groups:
				CKGroup *pGroup = pServer->GetGroupHead ();

				// Loop over all groups in linked list.  End of list will be
				// indicated by a NULL pointer:
				while (pGroup)
					{
					// Insert group item into the server branch of the tree:
					Insert (hParent, pGroup, false);

					// Move onto next group in linked list:
					pGroup = pGroup->GetNext ();
					}

				// Move onto next server connection in linked list:
				pServer = pServer->GetNext ();
				}

			// Select first item in the tree:
			CTreeCtrl &cTree = GetTreeCtrl ();
			HTREEITEM hFirstItem = cTree.GetFirstVisibleItem ();

			if (hFirstItem != NULL)
				cTree.SelectItem (hFirstItem);
			}
			break;

		case HINT_REFRESH_GROUPVIEW:
			// Force a repaint:
			GetTreeCtrl ().Invalidate (true);
			GetTreeCtrl ().UpdateWindow ();
			break;

		default:
			// Default processing:
			CView::OnUpdate (pSender, lHint, pHint);
			break;
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupView mouse handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnRButtonDown ()
//
// Description:
//	Process right button click messages for this view.  Build and display an
//  appropriate popup menu.
//
// Parameters:
//  UINT		uFlags		Indicates whether various virtual keys are down.
//	CPoint		point		Specifies the x and y coordinates of the cursor. 
//							  These coordinates are always relative to the
//							  upper-left corner of the window.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnRButtonDown (UINT uFlags, CPoint point) 
	{
	// Get reference to tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get pointer to our main menu.  We will use it to get text for
	// the popup menu:
	CMenu *pMainMenu = AfxGetMainWnd ()->GetMenu ();

	// Define flags to so that popup menu items will get text from pointer
	// to null terminated string.
	int nFlags = MF_ENABLED | MF_STRING;
	
	CMenu cMenu;
	UINT uHitFlags;
	HTREEITEM hItem;
	CString strMenuText;

	// Get a handle to the item that was clicked on (handle
	// will be NULL if click was not on a item):
	hItem = cTree.HitTest (point, &uHitFlags);

	// Select item if clicked on:
	if (hItem && (uHitFlags & TVHT_ONITEM)) 
		cTree.Select (hItem, TVGN_CARET);
		
	// Convert point to screen coordinates:
	ClientToScreen (&point);	

	// Build and display our context menu:
	if (cMenu.CreatePopupMenu ())
		{
		// If click was on an item:
		if (hItem)
			{
			// If click was on a server connection:
			if (cTree.GetParentItem (hItem) == NULL)
				{
				// Add Add Group menu item:
				pMainMenu->GetMenuString (ID_EDIT_NEWGROUP, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_EDIT_NEWGROUP, strMenuText);

				// Add separator menu item:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Connect menu item:
				pMainMenu->GetMenuString (ID_TOOLS_CONNECT, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_CONNECT, strMenuText);

				// Add Disconnect menu item:
				pMainMenu->GetMenuString (ID_TOOLS_DISCONNECT, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_DISCONNECT, strMenuText);

				// Add Reconnect menu item:
				pMainMenu->GetMenuString (ID_TOOLS_RECONNECT, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_RECONNECT, strMenuText);

				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Get Error String menu item:
				pMainMenu->GetMenuString (ID_TOOLS_GET_ERROR_STRING, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_GET_ERROR_STRING, strMenuText);

				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Enumerate Groups menu item:
				pMainMenu->GetMenuString (ID_TOOLS_ENUMERATE_GROUPS, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_ENUMERATE_GROUPS, strMenuText);
				
				// Add Get Group By Name menu item:
				pMainMenu->GetMenuString (ID_TOOLS_GET_GROUP_BY_NAME, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_GET_GROUP_BY_NAME, strMenuText);
				}

			// Else click was on a group:
			else 
				{
				// Add Add Item menu item:
				pMainMenu->GetMenuString (ID_EDIT_NEWITEM, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_EDIT_NEWITEM, strMenuText);

				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Clone Group menu item:
				pMainMenu->GetMenuString (ID_TOOLS_CLONE_GROUP, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_CLONE_GROUP, strMenuText);

				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Export CSV menu item:
				pMainMenu->GetMenuString (ID_TOOLS_EXPORT_CSV, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_EXPORT_CSV, strMenuText);

				// Add Import CSV menu item:
				pMainMenu->GetMenuString (ID_TOOLS_IMPORT_CSV, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_TOOLS_IMPORT_CSV, strMenuText);
				}
			
			// Add separator:
			cMenu.AppendMenu (MF_SEPARATOR);
	
			// Add Cut menu item:
			pMainMenu->GetMenuString (ID_EDIT_CUT, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_CUT, strMenuText);

			// Add Copy menu item:
			pMainMenu->GetMenuString (ID_EDIT_COPY, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_COPY, strMenuText);

			// Add Paste menu item:
			pMainMenu->GetMenuString (ID_EDIT_PASTE, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_PASTE, strMenuText);

			// Add Delete menu item:
			pMainMenu->GetMenuString (ID_EDIT_DELETE, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_DELETE, strMenuText);

			// Add separator:
			cMenu.AppendMenu (MF_SEPARATOR);

			// Add Properties menu item:
			pMainMenu->GetMenuString (ID_EDIT_PROPERTIES, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_PROPERTIES, strMenuText);
			}
		
		// Else click was on the background:
		else
			{
			// Add Add Server Connection menu item:
			pMainMenu->GetMenuString (ID_EDIT_NEWSERVER, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_NEWSERVER, strMenuText);

			// Check format of clipboard contents to see if it contains
			// a server connection.  If so, add a paste menu item.
			if (IsClipboardFormatAvailable (CF_SERVER))
				{
				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Paste menu item:
				pMainMenu->GetMenuString (ID_EDIT_PASTE, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_EDIT_PASTE, strMenuText);
				}
			}

		// Place the popup menu at the point of right mouse click, and route
		// all WM_COMMAND messages though the frame:
		if (cMenu.GetMenuItemCount () > 0)
			cMenu.TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd ());
		}
	}

// **************************************************************************
// OnLButtonDblClk ()
//
// Description:
//	Process left button double click messages for this view.  Display the 
//	properties for the item selected.
//
// Parameters:
//  UINT		uFlags		Indicates whether various virtual keys are down.
//	CPoint		point		Specifies the x and y coordinates of the cursor. 
//							  These coordinates are always relative to the
//							  upper-left corner of the window.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnLButtonDblClk (UINT nFlags, CPoint point) 
{
	// Get pointer to our document.  We will need it to display a property
	// sheet for selected item.
	CKDocument *pDoc = (CKDocument *) GetDocument ();
	ASSERT (pDoc != NULL);

	// Get reference to tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();
	HTREEITEM hItem;
	UINT uHitFlags;

	// Get a handle to the item that was clicked on.  (Handle will
	// be NULL if click was not on an item.)
	hItem = cTree.HitTest (point, &uHitFlags);

	// If there was an item we clicked select it:
	if (hItem && (uHitFlags & TVHT_ONITEMLABEL)) 
		cTree.Select (hItem, TVGN_CARET);
		
	// Convert point to screen coordinates:
	ClientToScreen (&point);	

	// Show property sheet for selected item:
	// Note, tree items are created with pointers to associated CKServer
	// or CKGroup objects saved in item data parameter.  We can retrieve
	// these parameters, case appropriately, and give to edit functions.
	if (hItem)
	{
		// If handle to parent item is NULL, then we know we selected a server:
		if (cTree.GetParentItem (hItem) == NULL)
			pDoc->EditServer ((CKServer *) cTree.GetItemData (hItem));

		// Otherwise we must have selected a group:
		else
			pDoc->EditGroup ((CKGroup *) cTree.GetItemData (hItem));
	}
}


/////////////////////////////////////////////////////////////////////////////
// CKGroupView tree handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnSelectionChanged ()
//
// Description:
//	Called when a item change in selection occurs.  We take this opportunity 
//	to set the newly selected item to a bold font to indicate selection.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnSelectionChanged (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	*pResult = 0;

	// Cast notification message to proper tree view notification message:
	NM_TREEVIEW *ptv = (NM_TREEVIEW *)pNMHDR;

	// Get reference to tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	ASSERT (ptv->itemNew.mask & TVIF_HANDLE);
	ASSERT (ptv->itemOld.mask & TVIF_HANDLE);

	// Unbold the previous selection:
	if (ptv->itemOld.hItem)
		cTree.SetItemState (ptv->itemOld.hItem, 0, TVIS_BOLD);

	// Make new selection bold:
	if (ptv->itemNew.hItem)
		cTree.SetItemState (ptv->itemNew.hItem, TVIS_BOLD, TVIS_BOLD);

	// Get pointer to our document so we can inform it of our
	// new selection:
	CKDocument *pDoc = (CKDocument *) GetDocument ();

	if (pDoc)
		{
		// Get handle of selected item's parent.  We will use it to
		// see if we selected a server of group.
		HTREEITEM hParent = cTree.GetParentItem (ptv->itemNew.hItem);

		// Note, tree items are created with pointers to associated CKServer
		// or CKGroup objects saved in item data parameter.  We can retrieve
		// these parameters, case appropriately, and give to edit functions.

		// If handle of parent is NULL, then we know we selected a server:
		if (hParent == NULL)
			{
			// Give document selected server (no selected group):
			pDoc->SetSelectedServerGroup (
				(CKServer *)cTree.GetItemData (ptv->itemNew.hItem), NULL);
			}

		// Otherwise we must have selected a group:
		else
			{
			// Give document selected group (and parent server):
			pDoc->SetSelectedServerGroup (
				NULL, (CKGroup *)cTree.GetItemData (ptv->itemNew.hItem));
			}
		}
	}

// **************************************************************************
// Insert ()
//
// Description:
//	Insert a new object into the view.
//
// Parameters:
//  HTREEITEM	hParent		Handle to parent item in tree control.
//	CObject		*pObject	Pointer to object to insert.
//	bool		bSelect		Set to true to select item after inserted (default).
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::Insert (HTREEITEM hParent, CObject *pObject, bool bSelect /*= true */)
	{
	ASSERT (pObject != NULL);
	
	TV_INSERTSTRUCT tvis;
	HTREEITEM hItem;

	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();
		
	// Fill up the tree view insert structure:
	tvis.hParent = hParent;						// Handle to parent item
	tvis.hInsertAfter = TVI_SORT;				// Sort items in tree
												// item props we will fill	
	tvis.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
	
	tvis.item.lParam = (LPARAM)pObject;			// item data: pointer to object
	tvis.item.pszText = LPSTR_TEXTCALLBACK;		// item text: defer to OnGetDispInfo ()
	tvis.item.iImage = I_IMAGECALLBACK;			// item image: defer to OnGetDispInfo ()
	tvis.item.iSelectedImage = I_IMAGECALLBACK;	// item selimage: defer to OnGetDispInfo ()

	// Insert the item:
	hItem = cTree.InsertItem (&tvis);
	
	// Select the item if asked:
	if (bSelect)
		cTree.SelectItem (hItem);

	// Give item object the handle of the tree control item it is associated with:

	// If parent is tree control root, then we know item is a server:
	if (hParent == TVI_ROOT)
		((CKServer *)pObject)->SetGUIHandle (hItem);

	// Otherwise item must be a group:
	else
		((CKGroup *)pObject)->SetGUIHandle (hItem);
	}

// **************************************************************************
// Delete ()
//
// Description:
//	Delete an object from the view.
//
// Parameters:
//  HTREEITEM	hItem		Handle of item to delete.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::Delete (HTREEITEM hItem)
	{
	// Delete tree item:
	GetTreeCtrl ().DeleteItem (hItem);
	}

// **************************************************************************
// OnGetDispInfo ()
//
// Description:
//	Handle notification to fill item information for display.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Cast notification message structure appropriately:
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	// Extract tree view item:
	TV_ITEM *pItem = &pTVDispInfo->item;

	// If handle to parent item is NULL, we know we are working with a server:
	bool bServerItem = (GetTreeCtrl ().GetParentItem (pItem->hItem) == NULL);
	
	// Pointer to object from tree item parameter.  We can cast this to a 
	// CKServer or CKGroup as the case may be.
	CObject *pObject = (CObject *) pItem->lParam;

	// The item's mask text bit will be set if we are being asked to supply
	// the item's text.
	if (pItem->mask & TVIF_TEXT)
		{
		if (bServerItem)
			lstrcpyn (pItem->pszText, ((CKServer *)pObject)->GetProgID (), pItem->cchTextMax);
		else
			lstrcpyn (pItem->pszText, ((CKGroup *)pObject)->GetName (), pItem->cchTextMax);
		}

	// The item's mask (not selected) image bit will be set if we are being
	// asked to supply the items image (index into image control).
	if (pItem->mask & TVIF_IMAGE)
		{
		// Give server image if server is specified:
		if (bServerItem)
			{
			// Give server connected image if connected:
			if (((CKServer *)pObject)->IsConnected ())
				pItem->iImage = 0;

			// Else give server not connected image:
			else
				pItem->iImage = 6;
			}

		// Else give group image:
		else
			{
			// Cast pObject to CKGroup:
			CKGroup *pGroup = (CKGroup *)pObject;
			
			// If group is valid, give active or inactive image as case may be:
			if (pGroup->IsValid ())
				pItem->iImage = pGroup->IsActive () ? 2 : 3;
			
			// Else give invalid group image:
			else
				pItem->iImage = 7;
			}
		}

	// The item's mask selected image bit will be set if we are being
	// asked to supply the items image (index into image control).
	if (pItem->mask & TVIF_SELECTEDIMAGE)
		{
		// Give server image is server is specified:
		if (bServerItem)
			{
			// Give server connected image if connected:
			if (((CKServer *)pObject)->IsConnected ())
				pItem->iSelectedImage = 0;

			// Else give server not connected image:
			else
				pItem->iSelectedImage = 6;
			}

		// Else give group image:
		else
			{
			// Cast pObject to CKGroup:
			CKGroup *pGroup = (CKGroup *)pObject;
			
			// If group is valid, give active of inactive image as case may be:
			if (pGroup->IsValid ())
				pItem->iSelectedImage = pGroup->IsActive () ? 4 : 5;

			// Else give invalid group image:
			else
				pItem->iSelectedImage = 7;
			}
		}

	*pResult = 0;
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroupView diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CKGroupView::AssertValid () const
	{
	CTreeView::AssertValid ();
	}

void CKGroupView::Dump (CDumpContext &dc) const
	{
	CTreeView::Dump (dc);
	}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKGroupView message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnNewServer ()
//
// Description:
//	New server connection menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnNewServer () 
	{
	// Get pointer to our document object.  (Pointer will be NULL if we
	// did not get properly attached to document):
	CKDocument *pDoc = (CKDocument *) GetDocument ();

	// If poiner looks good, tell document to add a server:
	if (pDoc)
		pDoc->AddServer ();
	}

// **************************************************************************
// OnNewGroup ()
//
// Description:
//	New group menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnNewGroup () 
	{
	// Get pointer to our document object.  (Pointer will be NULL if we
	// did not get properly attached to document):
	CKDocument *pDoc = (CKDocument *) GetDocument ();

	// If pointer looks good, tell document to add a group:
	if (pDoc)
		pDoc->AddGroup ();
	}

// **************************************************************************
// OnProperties ()
//
// Description:
//	Properties menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnProperties () 
	{
	// Get reference to out tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  (Handle will be NULL if no item 
	// is selected.  We shouldn't get called if that is the case.)
	HTREEITEM hItem = cTree.GetSelectedItem ();
	
	if (hItem)
		{
		// Get pointer to our document:
		CKDocument *pDoc = (CKDocument *) GetDocument ();
	
		if (pDoc)
			{
			// Note: pointer to CKServer or CKGroup object is saved in
			// assoicated tree item's item data parameter.

			// If handle to parent item is NULL, we know a server was selected:
			if (cTree.GetParentItem (hItem) == NULL)
				pDoc->EditServer ((CKServer *) cTree.GetItemData (hItem));

			// Else a group must have been selected:
			else
				pDoc->EditGroup ((CKGroup *) cTree.GetItemData (hItem));
			}
		}
	}

// **************************************************************************
// OnCopy ()
//
// Description:
//	Copy menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnCopy () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item:
	HTREEITEM hItem = cTree.GetSelectedItem ();
		
	// Create a fixed shared memory file to copy data to:
	CFixedSharedFile sf;

	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create some scratch pointers:
	CKServer *pServer = NULL;
	CKGroup *pGroup = NULL;
	

	// Get the object and perform the copy:
	try
		{
		// If handle to parent item is NULL, we know a server is selected:
		if (cTree.GetParentItem (hItem) == NULL)
			{
			// Pointer to CKServer object is stored in tree item's item data parameter:
			pServer = (CKServer *) cTree.GetItemData (hItem);
			ASSERT (pServer != NULL);

			// Call server's copy function to place its properties in shared mem file:
			pServer->Copy (sf);
			}

		// Else a group must be selected:
		else
			{
			// Pointer to CKGroup object is stored in tree item's item data parameter:
			pGroup = (CKGroup *) cTree.GetItemData (hItem);
			ASSERT (pGroup != NULL);

			// Call group's copy functin to place its properties in shared mem file:
			pGroup->Copy (sf);
			}
		}

	catch (...)
		{
		ASSERT (FALSE);
		return;
		}

	// Copy the object data, contained in shared mem file, to clipboard:
	sf.CopyToClipboard (pServer != NULL ? CF_SERVER : CF_GROUP);
	}

// **************************************************************************
// OnCut ()
//
// Description:
//	Cut menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnCut () 
	{
	// Perform a copy first,
	OnCopy ();

	// then a delete:
	OnDelete ();
	}

// **************************************************************************
// OnPaste ()
//
// Description:
//	Paste menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnPaste () 
	{
	// Open the clipboard.  Proceed if succesful:
	if (OpenClipboard ())
		{
		// Get reference to our tree control:
		CTreeCtrl &cTree = GetTreeCtrl ();

		// Get handle of selected item:
		HTREEITEM hItem = cTree.GetSelectedItem ();

		// Get pointer to our document object:
		CKDocument *pDoc = (CKDocument *) GetDocument ();

		// Create a fixed shared memory file to copy clipboard data to:
		CFixedSharedFile sf;

		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;

		try
			{
			// If server in the clipboard:
			if (IsClipboardFormatAvailable (CF_SERVER))
				{
				// Instantiate a new server object:
				CKServer *pServer = new CKServer;

				// Set shared memory file handle to clipboard data so we
				// can extract clipboard data using the memory file:
				sf.SetHandle (GetClipboardData (CF_SERVER), FALSE);

				// Call the server's paste function to load settings from
				// clipboard:
				pServer->Paste (sf);

				// Add server to project and connect:
				pDoc->AddServer (pServer, true /* connect */);
				}

			// Else if group is in the clipboard
			else if (IsClipboardFormatAvailable (CF_GROUP))
				{
				// We must be pasting to a server connection:
				ASSERT (cTree.GetParentItem (hItem) == NULL);
				
				// Instantiate a new group object, give it pointer to its
				// parent server object:
				CKGroup *pGroup = new CKGroup ((CKServer *) cTree.GetItemData (hItem));
				
				// Set shared memory file handle to clipboard data so we
				// can extract clipboard data using the memory file:
				sf.SetHandle (GetClipboardData (CF_GROUP), FALSE);

				// Call group's paste function to load settings from 
				// clipboard:
				pGroup->Paste (sf);

				// Add group to project:
				pDoc->AddGroup (pGroup);
				}

			// Else if item(s) are in the clipboard:
			else if (IsClipboardFormatAvailable (CF_ITEM))
				{
				// Create an object array to contain a list of items to be
				// added to a group:
				CObArray cItemList;
				DWORD cdwItems;
				CKItem *pItem;

				// Get pointer to group:
				CKGroup *pGroup = (CKGroup *) cTree.GetItemData (hItem);
				ASSERT (pGroup != NULL);

				// Set shared memory file handle to clipboard data so we
				// can extract clipboard data using the memory file:
				sf.SetHandle (GetClipboardData (CF_ITEM), FALSE);

				// Retrieve number of items:
				sf.Read (&cdwItems, sizeof (cdwItems));
				ASSERT (cdwItems > 0);

				// Allocate memory for items:
				cItemList.SetSize (cdwItems + 1);
				DWORD dwIndex;
				// Now retrieve the items:
				for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
					{
					// Instantiate a new item object:
					pItem = new CKItem (pGroup);

					// Call the item's paste function to load settings from 
					// clipboard:
					pItem->Paste (sf);

					// Add the item to object array:
					cItemList [dwIndex] = pItem;
					}

				// Terminate the list.  Will allow us to process list using
				// "while (element)" loop if we want:
				cItemList [dwIndex] = NULL;

				// Add the items:
				pDoc->AddItems (cItemList, cdwItems);
				}

			// Unidentified clipboard contents.  Throw exception.
			else
				throw (-1);
			}

		// If problem, empty clipboard contents:
		catch (...)
			{
			ASSERT (FALSE);
			EmptyClipboard (); 
			}

		// Free the shared memory handle:
		sf.Detach ();

		//  Close the clipboard:
		CloseClipboard ();
		}
	}

// **************************************************************************
// OnDelete ()
//
// Description:
//	Delete menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnDelete () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  (Handle will be NULL if no item is
	// selected):
	HTREEITEM hItem = cTree.GetSelectedItem ();

	// If an item was slected, delete it:
	if (hItem)
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;

		// Get pointer to our document.  (Pointer will be NULL if we did
		// not get properly attached to document).
		CKDocument *pDoc = (CKDocument *) GetDocument ();
	
		// Tell document to remove server or group as case may be:
		if (pDoc)
			{
			// If handle of parent item is NULL, we know a server is selected:
			if (cTree.GetParentItem (hItem) == NULL)
				pDoc->RemoveServer ((CKServer *) cTree.GetItemData (hItem));
			
			// Else a group must be selected:
			else
				pDoc->RemoveGroup ((CKGroup *) cTree.GetItemData (hItem));
			}
		}
	}

// **************************************************************************
// OnConnect ()
//
// Description:
//	Connect menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnConnect () 
	{
		
			// Get reference to our tree control:
			CTreeCtrl &cTree = GetTreeCtrl ();

			// Get handle of selected item.  Handle will be NULL if no item is
			// selected.  We should only get called if a server is selected.
			HTREEITEM hItem = cTree.GetSelectedItem ();

			if (hItem)
			{
				// A server had better be selected.  This will be the case if handle
				// of parent is NULL.  Check this (debug only).
				ASSERT (cTree.GetParentItem (hItem) == NULL);

				// Get pointer to our document object.  (This could be NULL if
				// we did not get attached to document properly.)
				CKDocument *pDoc = (CKDocument *) GetDocument ();

				// Tell document to connect server.  Pointer to CKServer object is
				// saved in tree item data parameter.
				if (pDoc)
					pDoc->ConnectServer ((CKServer *) cTree.GetItemData (hItem));
			}
			Sleep(5000);
		
	
	}

// **************************************************************************
// OnDisconnect ()
//
// Description:
//	Disconnect menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnDisconnect () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  Handle will be NULL if no item is 
	// selected.  We should only get called if a server is selected.
	HTREEITEM hItem = cTree.GetSelectedItem ();

	if (hItem)
		{
		// A server had better be selected.  This will be the case if handle
		// of parent is NULL.  Check this (debug only).
		ASSERT (cTree.GetParentItem (hItem) == NULL);

		// Get pointer to our document object.  (This could be NULL if
		// we did not get attached to document properly.)
		CKDocument *pDoc = (CKDocument *) GetDocument ();
		
		// Tell document to disconnect server.  Pointer to CKServer object
		// is saved in tree item data parameter.
		if (pDoc)
			pDoc->DisconnectServer ((CKServer *) cTree.GetItemData (hItem));
		}
	}

// **************************************************************************
// OnReconnect ()
//
// Description:
//	Reconnect menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnReconnect ()
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  Handle will be NULL if no item is 
	// selected.  We should only get called if a server is selected.
	HTREEITEM hItem = cTree.GetSelectedItem ();

	if (hItem)
		{
		// A server had better be selected.  This will be the case if handle
		// of parent is NULL.  Check this (debug only).
		ASSERT (cTree.GetParentItem (hItem) == NULL);

		// Get pointer to our document object.  (This could be NULL if
		// we did not get attached to document properly.)
		CKDocument *pDoc = (CKDocument *) GetDocument ();
		
		if (pDoc)
			{
			// Pointer to CKServer object is saved in tree item data
			// parameter:
			CKServer *pServer = (CKServer *) cTree.GetItemData (hItem);

			// Tell document to disconnect server first,
			pDoc->DisconnectServer (pServer);

			// then connect again:
			pDoc->ConnectServer (pServer);
			}
		}
	}

// **************************************************************************
// OnGetErrorString ()
//
// Description:
//	Get Error String menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnGetErrorString () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  Handle will be NULL if no item is 
	// selected.  We should only get called if a server is selected.
	HTREEITEM hItem = cTree.GetSelectedItem ();

	// A server had better be selected.  This will be the case if handle
	// of parent is NULL.  Check this (debug only).
	ASSERT (cTree.GetParentItem (hItem) == NULL);

	// Create a "Get error string" dialog.  Give it a pointer to the server
	// object (saved as tree item data parameter).
	CKServerGetErrorStringDlg dlg ((CKServer *)cTree.GetItemData (hItem));

	// Show as a modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnGetGroupByName ()
//
// Description:
//	Get Group By Name menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnGetGroupByName () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  Handle will be NULL if no item is 
	// selected.  We should only get called if a server is selected.
	HTREEITEM hItem = cTree.GetSelectedItem ();

	// A server had better be selected.  This will be the case if handle
	// of parent is NULL.  Check this (debug only).
	ASSERT (cTree.GetParentItem (hItem) == NULL);

	// Pointer to CKServer object is saved in tree item data
	// parameter:
	CKServer *pServer = (CKServer *)cTree.GetItemData (hItem);
	ASSERT (pServer != NULL);

	// Create a "Get group by name" dialog.  Give it a pointer to the
	// server's IOPCServer COM interface:
	CKServerGroupByNameDlg dlg (pServer->GetIServer ());

	// Show as a modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnEnumerateGroups ()
//
// Description:
//	Enumerate Groups menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnEnumerateGroups () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  Handle will be NULL if no item is 
	// selected.  We should only get called if a server is selected.
	HTREEITEM hItem = cTree.GetSelectedItem ();

	// A server had better be selected.  This will be the case if handle
	// of parent is NULL.  Check this (debug only).
	ASSERT (cTree.GetParentItem (hItem) == NULL);

	// Pointer to CKServer object is saved in tree item data
	// parameter:
	CKServer *pServer = (CKServer *)cTree.GetItemData (hItem);
	ASSERT (pServer != NULL);

	// Create an "Enumerate groups" dialog.  Give it a pointer to the
	// server's IOPCServer COM interface:
	CKServerEnumerateGroupsDlg dlg (pServer->GetIServer ());

	// Show as a modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnCloneGroup ()
//
// Description:
//	Clone Group menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnCloneGroup () 
	{
	// Get pointer to our document.  (Pointer could be NULL if we did not
	// get attached to document properly.)
	CKDocument *pDoc = (CKDocument *) GetDocument ();

	// Tell document to clone selected group.  Document should have been
	// advised of selected group before we get called.
	if (pDoc)
		pDoc->CloneGroup ();
	}

// **************************************************************************
// OnChar ()
//
// Description:
//	Handle keyboard input.  Switch view on tab, etc.
//
// Parameters:
//  UINT		nChar		Character code
//	UINT		nRepCnt		repeat count
//	UINT		nFlags		Flags
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
	// Switch views on TAB:
	if (nChar == VK_TAB)
		{
		// Post a change view message to main window:
		AfxGetMainWnd ()->PostMessage (UM_CHANGEVIEW, 0, (LPARAM)this);

		// Return now so default processing does not occur:
		return;
		}
	
	// Show context menu on SPACEBAR:
	else if (nChar == VK_SPACE)
		{
		// Get reference to our tree control:
		CTreeCtrl &cTree = GetTreeCtrl ();

		// Get handle of selected item.
		HTREEITEM hItem = cTree.GetSelectedItem ();
		
		// Get rectange of selected item:
		CRect rc;
		cTree.GetItemRect (hItem, &rc, true);

		// Compute the coordinates of rectangle center:
 		rc.left = (rc.left + rc.right) / 2;
		rc.top = (rc.top + rc.bottom) / 2;

		// Use code in OnRButtonDown() to construct the context menu.  Pass
		// it the coordinates of rectangle's center.  (That's where it will
		// place the context menu.)
		OnRButtonDown (0, CPoint (rc.left, rc.top)); 

		// Return now so default processing will not occur:
		return;
		}

	// Show properties on return:
	else if (nChar == VK_RETURN)
		{
		// Use code in OnProperties() to display properties of selected item:
		if (GetTreeCtrl ().GetSelectedItem ())
			OnProperties ();

		// Return now so default processing will not occur:
		return;
		}
	
	// Perform default processing for all other characters:
	CTreeView::OnChar (nChar, nRepCnt, nFlags);
	}

// **************************************************************************
// OnExportCsv ()
//
// Description:
//	Export CSV menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnExportCsv () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  (We shouldn't get called unless a group
	// is selected.)
	HTREEITEM hItem = cTree.GetSelectedItem ();
	ASSERT (hItem);

	// Pointer to group object is saved in tree item data parameter:
	CKGroup *pGroup = (CKGroup *) cTree.GetItemData (hItem);
	ASSERT (pGroup && (cTree.GetParentItem (hItem) != NULL));

	// Declare some strings we will need for common file save dialog:
	CString strInitialPath;
	CString strFilter;
	CString strTitle;

	// Load the common file save dialog filter string from string resources:
	strFilter.LoadString (IDS_CSVFILEFILTER);
	
	// Load the common file save dialog title from string resources:
	strTitle.LoadString (IDS_CSVEXPORT);

	// Define the initial path string (always to project directory):
	strInitialPath = AfxGetApp ()->GetProfileString (lpszPaths, lpszOpenFile, _T(""));

	// Create the common file save dialog:
	CFileDialog ofn (
		FALSE,									// Not an open dialog
		lpszCSV, 								// Default extension (CSV)
		pGroup->GetName (),						// Suggested file name is group name
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, // Flags (hide read only files, prompt if attemt to overwrite)
		strFilter);								// Filter set

	// Set initial path and title:
	ofn.m_ofn.lpstrInitialDir = strInitialPath;
	ofn.m_ofn.lpstrTitle = strTitle;

	// Show as modal dialog.  Unless user hits "OK", return:
	if (ofn.DoModal () != IDOK)
		return;

	// If we make it here, user specified all necessary data in common file save
	// dialog, and hit "OK".  Create a CStdioFile to make file IO easy for us.
	CStdioFile csv;

	// Create a file exception object, just in case:
	CFileException fe;

	// Open the file.  Get file name from common file save dialog.  Open file
	// as readable// writeable, create if necessary, as text.  If failure, 
	// report error and return:
	if (!csv.Open (ofn.GetPathName (), CFile::modeReadWrite | CFile::modeCreate | CFile::typeText, &fe))
		{
		// Let MFC report the open failure:
		fe.ReportError ();
		return;
		}

	// Perform the export:
	try
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;

		// Call the group's ExportCsv function to save properties to file:
		pGroup->ExportCsv (csv);
		}
	
	// If anything happens, delete the file:
	catch (...)
		{
		TRACE (_T("OTC: Exception thrown during CSV export -> deleting the output file\n"));
		csv.Close ();
		CFile::Remove (ofn.GetPathName ());
		}
	}

// **************************************************************************
// OnImportCsv ()
//
// Description:
//	Import CSV menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroupView::OnImportCsv () 
	{
	// Get reference to our tree control:
	CTreeCtrl &cTree = GetTreeCtrl ();

	// Get handle of selected item.  (We shouldn't get called unless a group
	// is selected.)
	HTREEITEM hItem = cTree.GetSelectedItem ();
	ASSERT (hItem);

	// Pointer to group object is saved in tree item data parameter:
	CKGroup *pGroup = (CKGroup *) cTree.GetItemData (hItem);
	ASSERT (pGroup && (cTree.GetParentItem (hItem) != NULL));

	// Declare some strings we will need for common file save dialog:
	CString strInitialPath;
	CString strFilter;
	CString strTitle;

	// Load the common file open dialog filter string from string resources:
	strFilter.LoadString (IDS_CSVFILEFILTER);
	
	// Load the common file open dialog title from string resources:
	strTitle.LoadString (IDS_CSVIMPORT);

	// Define the initial path string (always to project directory):
	strInitialPath = AfxGetApp ()->GetProfileString (lpszPaths, lpszOpenFile, _T(""));

	// Create the common file open dialog:
	CFileDialog ofn (
		TRUE,									// open dialog
		lpszCSV, 								// Default extension (CSV)
		pGroup->GetName (),						// Suggested file name is group name
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, // Flags (hide read only files, prompt if attemt to overwrite)
		strFilter);								// Filter set

	// Set initial path and title:
	ofn.m_ofn.lpstrInitialDir = strInitialPath;
	ofn.m_ofn.lpstrTitle = strTitle;

	// Show as modal dialog.  Unless user hits "OK", return:
	if (ofn.DoModal () != IDOK)
		return;

	// If we make it here, user specified all necessary data in common file open
	// dialog, and hit "OK".  Create a CStdioFile to make file IO easy for us.
	CStdioFile csv;

	// Create a file exception object, just in case:
	CFileException fe;

	// Open the file.  Get file name from common file open dialog.  Open file
	// as read only, text, and deny other applications simultaneous write 
	// access.  If failure, report error and return:
	if (!csv.Open (ofn.GetPathName (), CFile::modeRead | CFile::typeText | CFile::shareDenyWrite, &fe))
		{
		// Let MFC report the open failure:
		fe.ReportError ();
		return;
		}

	// Perform the import:
	try
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;

		// Declare an object array to hold a list of the group's items to
		// import:
		CObArray cItemList;
		DWORD cdwItems = 0;

		// Call the group's ImportCsv function to read the group and item
		// properties from the file.  The object array cItemList will be
		// loaded by this function.
		pGroup->ImportCsv (csv, cItemList, cdwItems);

		// If the group has items, indicated by a non-zero cdwItems, tell
		// the document to add these items:
		if (cdwItems)
			((CKDocument *) GetDocument ())->AddItems (cItemList, cdwItems);
		}
	
	catch (...)
		{
		TRACE (_T("OTC: Exception thrown during CSV import\n"));
		}
	}
