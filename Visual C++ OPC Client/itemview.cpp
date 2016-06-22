// **************************************************************************
// itemview.cpp
//
// Description:
//	Implements a CListView derived class.  This the top right pane of our GUI
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


#include "stdafx.h"
#include "opctestclient.h"
#include "itemview.h"
#include "document.h"
#include "item.h"
#include "group.h"
#include "server.h"
#include "itempropertiesdlg.h"
#include "itemwritedlg.h"
#include "updateintervaldlg.h"

// Define some list control constants:
#define NUMCOLUMNS			6
#define DEFAULTCOLUMNWIDTH	120
#define ASCENDING			0
#define DESCENDING			1

// Define amount to grow item list by:
#define GROWLIST			1024

// Define column headers:
static LPCTSTR lpszRegSection = _T("Item View");
static LPCTSTR lpszSortColumn = _T("Sort Column");
static LPCTSTR lpszSortOrder = _T("Sort Order");
static LPCTSTR lpszUpdateInterval = _T("Update Interval");

// Item pane update timer evetn type:
#define UPDATE_ITEMPANE_EVENT	1

// Declare member statics:
WORD CKItemView::sm_wSortOrder = ASCENDING;
WORD CKItemView::sm_wSortColumn = 0;
DWORD* CKItemView::sm_pSortedItemList = NULL;


/////////////////////////////////////////////////////////////////////////////
// CKItemView
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKItemView, CListView)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKItemView, CListView)
	//{{AFX_MSG_MAP(CKItemView)
	ON_WM_DESTROY ()
	ON_COMMAND (ID_EDIT_NEWITEM, OnNewItem)
	ON_NOTIFY_REFLECT (LVN_GETDISPINFO, OnGetDispInfo)
	ON_COMMAND (ID_EDIT_PROPERTIES, OnProperties)
	ON_NOTIFY_REFLECT (LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_TIMER ()
	ON_COMMAND (ID_EDIT_DELETE, OnDelete)
	ON_WM_RBUTTONDOWN ()
	ON_COMMAND (ID_VIEW_UPDATETIMER, OnUpdateTimer)
	ON_COMMAND (ID_TOOLS_SYNC_WRITE, OnSyncWrite)
	ON_COMMAND (ID_TOOLS_ASYNC10_WRITE, OnAsync10Write)
	ON_COMMAND (ID_TOOLS_ASYNC20_WRITE, OnAsync20Write)
	ON_COMMAND (ID_TOOLS_SET_ACTIVE, OnSetActive)
	ON_COMMAND (ID_TOOLS_SET_INACTIVE, OnSetInactive)
	ON_COMMAND (ID_TOOLS_SYNC_READ_CACHE, OnSyncReadCache)
	ON_COMMAND (ID_TOOLS_SYNC_READ_DEVICE, OnSyncReadDevice)
	ON_COMMAND (ID_TOOLS_ASYNC10_READ_CACHE, OnAsync10ReadCache)
	ON_COMMAND (ID_TOOLS_ASYNC10_READ_DEVICE, OnAsync10ReadDevice)
	ON_COMMAND (ID_TOOLS_ASYNC20_READ_DEVICE, OnAsync20ReadDevice)
	ON_COMMAND (ID_TOOLS_ASYNC10_REFRESH_CACHE, OnAsync10RefreshCache)
	ON_COMMAND (ID_TOOLS_ASYNC10_REFRESH_DEVICE, OnAsync10RefreshDevice)
	ON_COMMAND (ID_TOOLS_ASYNC20_REFRESH_CACHE, OnAsync20RefreshCache)
	ON_COMMAND (ID_TOOLS_ASYNC20_REFRESH_DEVICE, OnAsync20RefreshDevice)
	ON_COMMAND (ID_EDIT_COPY, OnCopy)
	ON_COMMAND (ID_EDIT_CUT, OnCut)
	ON_COMMAND (ID_EDIT_PASTE, OnPaste)
	ON_NOTIFY_REFLECT (NM_DBLCLK, OnDblclk)
	ON_WM_CHAR ()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE (TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE (TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP ()


// **************************************************************************
// CKItemView ()
//
// Description:
//	Constructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKItemView::CKItemView ()
	{
	// Initialize member variables:
	sm_wSortOrder = ASCENDING;
	sm_wSortColumn = 0;
	m_wUpdateInterval = VIEW_DEFAULT_INTERVAL;

	m_cnSortedItems = 0;
	m_nSortedListSize = 0;
	sm_pSortedItemList = NULL;
	}

// **************************************************************************
// ~CKItemView ()
//
// Description:
//	Destructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKItemView::~CKItemView ()
	{
	// Free any memory used for sorted item list:
	if (sm_pSortedItemList)
		delete [] sm_pSortedItemList;
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemView creation/drawing
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// PreCreateWindow ()
//
// Description:
//	Called by framework before the creation of this window.  Use opportunity
//	to modify style of window.
//
// Parameters:
//	CREATESTRUCT	&cs		Defines the window initialization parameters. 
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKItemView::PreCreateWindow (CREATESTRUCT &cs) 
	{
	// Perform default processing.  Return FALSE if failure:
	if (!CListView::PreCreateWindow (cs))
		return (FALSE);

	// Modify window sytle:
	// First clear icon view, small icon view, and list view bits.
	// Then set report view and owner data bits.  
	// (Owner data transfers the responsibility of managing data from the 
	// list control to the application programmer.  This allows very large
	// data sets to be handled efficiently.  See MSDN discussion of Virtual
	// List-View Style.)
	cs.style &= ~(LVS_ICON | LVS_SMALLICON | LVS_LIST);
	cs.style |= (LVS_REPORT | LVS_OWNERDATA);

	// Return TRUE to indicate success:
	return (TRUE);	
	}

// **************************************************************************
// PreSubclassWindow ()
//
// Description:
//	This method is called by the framework to allow other necessary 
//	subclassing to occur before the window is subclassed. 
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::PreSubclassWindow () 
	{
	// Perform default processing:
	CListView::PreSubclassWindow ();

	// Enable tooltips:
	EnableToolTips (TRUE);
	}

// **************************************************************************
// Create
//
// Description:
//	Creates the CKItemView window and attaches it to parent window.  Use
//	opportunity to set list control's column properties from data saved
//	in registry and to start an update timer.
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
BOOL CKItemView::Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext) 
	{
	// Perform default processing.  Return FALSE if failure:
	if (!CWnd::Create (lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
		return (FALSE);

	// Initialize the image list for the items. The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 14x14 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	// Image number		Use
	//	0				Active tag
	//	1				Inactive tag
	//	2				Invalid tag
	//	3				not used
	m_cImageList.Create (IDB_ITEMIMAGES, 14, 2, RGB (255, 0, 255));
	m_cImageList.SetBkColor (CLR_NONE);
	GetListCtrl ().SetImageList (&m_cImageList, LVSIL_SMALL);

	// Create a string to hold column tile:
	CString strColumnTitle;

	// Define list control columns:
	for (int i = 0; i < NUMCOLUMNS; i++)
		{
		// Create a string to contain column width registry entry:
		TCHAR szNum [8];
		wsprintf (szNum, _T("%d"), i);

		// Read the column width from the registry.  This is the width of the column
		// saved at the end of the last session.
		int nWidth = AfxGetApp ()->GetProfileInt (lpszRegSection, szNum, DEFAULTCOLUMNWIDTH);

		// Make sure it is valid:
		if (nWidth < 0)
			nWidth = DEFAULTCOLUMNWIDTH;

		// Load the column title string resource:
		switch (i)
			{
			case 0:	// Item ID
				strColumnTitle.LoadString (IDS_ITEMID);
				break;

			case 1: // Data Type
				strColumnTitle.LoadString (IDS_DATATYPE);
				break;

			case 2: // Value
				strColumnTitle.LoadString (IDS_VALUE);
				break;

			case 3:	// Timestamp
				strColumnTitle.LoadString (IDS_TIMESTAMP);
				break;

			case 4: // Quality
				strColumnTitle.LoadString (IDS_QUALITY);
				break;

			case 5:	// Update Count
				strColumnTitle.LoadString (IDS_UPDATE_COUNT);
				break;

			default: // Unexpected column index
				ASSERT (FALSE);
				break;
			}

		// Insert the column:
		GetListCtrl ().InsertColumn (i, strColumnTitle, LVCFMT_LEFT, nWidth);
		}

	// Load the sort settings from registry.  (Settings saved at end of last session.)
	sm_wSortColumn = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszSortColumn, sm_wSortColumn);
	sm_wSortOrder = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszSortOrder, sm_wSortOrder);

	// Load update interval setting from registry.  (Setting saved at end of last session.)
	m_wUpdateInterval = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszUpdateInterval, m_wUpdateInterval);

	// Start update view timer.  This will give us a chance to do periodic 
	// maintenance of the view.
	SetTimer (UPDATE_ITEMPANE_EVENT, m_wUpdateInterval, NULL);

	// Add the extended full row selection style (This causes all subitems
	// to be selected at once - i.e. full row is selected.)
	SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	// Return TRUE to inicate success:
	return (TRUE);
	}

// **************************************************************************
// OnDestroy ()
//
// Description:
//	Called by framework just before window is destroyed.  Use opportunity to
//	save list control's column settings to registry and kill update timer.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnDestroy () 
	{
	// Perform default processing:
	CListView::OnDestroy ();
	
	// Save current column information to registry so we can use the same
	// settings next time:
	for (int i = 0; i < NUMCOLUMNS; i++)
		{
		// Create a string to identify column width registry entry:
		TCHAR szNum [8];
		wsprintf (szNum, _T("%d"), i);

		// Save width:
		AfxGetApp ()->WriteProfileInt (lpszRegSection, szNum, GetListCtrl ().GetColumnWidth (i));
		}

	// Save sorting preferences to registry:
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszSortColumn, sm_wSortColumn);
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszSortOrder, sm_wSortOrder);

	// Save update interval to registry:
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszUpdateInterval, m_wUpdateInterval);

	// Stop update view timer:
	KillTimer (UPDATE_ITEMPANE_EVENT);
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
void CKItemView::OnUpdate (CView *pSender, LPARAM lHint, CObject *pHint) 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Process according to hint type:
	switch (lHint)
		{
		// Need to update view due to added items:
		case HINT_ADD_ITEM:	
			{
			// pHint points to a CObArray containing a list of CKItem 
			// objects to add.  
			
			// Check to see if pHint was set (debug only).
			ASSERT (pHint != NULL);
			
			// Get pointer to the item list:
			CObArray *pList = (CObArray *)pHint;

			// Initialize some variables:
			CKItem *pItem = NULL;
			DWORD dwIndex = 0;

			// Postpone redrawing until we are done.  This will make things
			// go faster and look smoother.
			SetRedraw (false);

			// Insert all the items in array in.  Get pointer to next item
			// in object array, increment array index, and cast item pointer
			// to CKItem pointer.  A null pointer will have been placed in
			// array after last item to mark end of data.  This will cause
			// the loop to terminate.
			while (pItem = (CKItem *) pList->GetAt (dwIndex++))
				Insert (pItem);

			// Update the item count.  (This is one of the responsibilities
			// we took on by using a virtual list view.  See comments in
			// PreCreateWindow().)  Set flags to prevent the scroll position
			// from changing and to prevent the list control from repainting
			// unless affected items are in view. 
			GetListCtrl ().SetItemCountEx (m_cnSortedItems, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

			// Sort the list:
			SortList ();

			// Now that we have added all of the items, we can let the 
			// view repaint itself:
			SetRedraw (true);
			}
			break;

		// Need to update view due to re-added items:
		case HINT_READD_ITEM:
			{
			// This will be accomplished by simply causing a repaint of the
			// item icons currently in view.

			// Get reference to our list control:
			CListCtrl &cList = GetListCtrl ();

			int nTopIndex;
			CRect rc;
			CRect rcitem;
			
			// Get view rectangle:
			cList.GetClientRect (&rc);

			// Get index of first visible item:
			nTopIndex = cList.GetTopIndex ();

			// Get rectangle bounding the first visible item's icon:
			cList.GetItemRect (nTopIndex, &rcitem, LVIR_ICON);

			// Adjust the view rectangle to invalidate all item icons areas:
			rc.right = rcitem.right;
			rc.left = rcitem.left;
						
			// Invalidate the area to force a repaint of item icons:
			cList.InvalidateRect (&rc);
			}
			break;

		// Need to update view due to closed project or new server selection:
		case HINT_CLOSE_PROJECT:
		case HINT_SELECT_SERVER:
			// pHint will be NULL if HINT_CLOSE_PROJECT and will contain a
			// pointer to a CKServer object if HINT_SELECT_SERVER.  (Not
			// used in either case.)

			// No items should be displayed after project is closed of if
			// a server is selected.  (Items are displayed only when a group
			// is selected.)  Delete any items in list control.
			if (GetListCtrl ().GetItemCount () > 0)
				DeleteAllItems ();
			break;

		// Need to update view due to new group selection:
		case HINT_SELECT_GROUP:
			{
			// pHint points to the newly selected CKGroup object.

			// Hint pointer must have been set for us to proceed:
			if (pHint != NULL)
				{
				// Get pointer to newly selected CKGroup:
				CKGroup *pGroup = (CKGroup *) pHint;

				// Get reference to our list control:
				CListCtrl &cList = GetListCtrl ();

				// Postpone redrawing until we are done.  This will make
				// things go faster and look smoother.
				SetRedraw (false);

				// Delete any items that are currently in the list:
				if (cList.GetItemCount () > 0)
					DeleteAllItems ();
				
				// Insert all items that belong to the selected group:

				// Start with head of linked list of items in group and
				// work our way to the end.
				CKItem *pItem = pGroup->GetItemHead ();

				// Add each item in linked list until we hit the end 
				// (indicated by a NULL pointer).
				while (pItem)
					{
					// Insert item:
					Insert (pItem);

					// Get pointer to next item in linked list:
					pItem = pItem->GetNext ();
					}

				// Update the item count.  (This is one of the responsibilities
				// we took on by using a virtual list view.  See comments in
				// PreCreateWindow().)  Set flags to prevent the scroll position
				// from changing and to prevent the list control from repainting
				// unless affected items are in view.
				cList.SetItemCountEx (m_cnSortedItems, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

				// Sort the list:
				SortList ();

				// Now that we are done adding items, we can let the view
				// repaint itself:
				SetRedraw (true);
				}
			}
			break;

		// Need to update view due to refresh view request:
		case HINT_REFRESH_ITEMVIEW:
			// Force a repaint of whole view:
			GetListCtrl ().Invalidate (false);
			GetListCtrl ().UpdateWindow ();
			break;

		// Perform default processing:
		default:
			CView::OnUpdate (pSender, lHint, pHint);
			break;
		}	
	}

// **************************************************************************
// OnDraw
//
// Description:
//	OnDraw event handler.  This function needs to be implemented since it is
//  declared as a pure virtual function in base class.  We don't need to do
//  anything with it in this case.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnDraw (CDC *pDC)
	{
	// This is how we get our document object in case we need to do something
	// with this function at some point:
	CDocument* pDoc = GetDocument ();
	}

// **************************************************************************
// OnToolHitTest ()
//
// Description:
//	Called to determine if the point is over a tool.  Return the ID of the
//	tool found (Documentaion says return 1 if found, otherwise -1.  After
//	researching this, we found that this is not the case.  We need to return
//	a unique ID for each tool found or -1 if no tool is found.)  Tool in our 
//	case refers to an item/subitem cell.
//
// Parameters:
//  CPoint		cPoint		Location of cursor.
//	TOOLINFO	*pTI		Structure containing information about a 
//							  tool in a ToolTip control. 
//
// Returns:
//  int - ID of the tool found or -1 if no tool found.
// **************************************************************************
int CKItemView::OnToolHitTest (CPoint cPoint, TOOLINFO *pTI) const
	{
	int nRow = 0;
	int nCol = 0;

	// Get the (sub)item cell that the point resides in:
	CRect rc;
	nRow = GetCellRectFromPoint (cPoint, rc, &nCol);

	// If no cell was hit (indicated by nRow = -1), or if the cell does
	// not require a tool tip (entire text can fit in cell), then
	// return -1 (no tool found).
	if ((nRow == -1) || !(RequireCellToolTip (nRow, nCol, rc)))
		return (-1);

	// If we make it here, then the hit was in a cell that requires a 
	// tool tip to display entire text.

	// Fill the toolinfo structure (function's out parameter):

	// Handle to the window that contains the tool (i.e. this view):
	pTI->hwnd = m_hWnd;
	
	// Compute a unique tool ID from row and cell.  Ten least significant
	// bits will be the column number (up to 1023 columns), remaining 22
	// bits will be the row number (up 4194303 to rows).  Add one to make
	// ID non-zero since the list view will automatically create a tip
	// with ID zero.
	pTI->uId = (UINT)((nRow << 10) + (nCol & 0x3FF) + 1);

	// Get tool text through callback (TTN_NEEDTEXT notification)
	pTI->lpszText = LPSTR_TEXTCALLBACK;

	// Tool boundary:
	pTI->rect = rc;						// tool rect boundary

	// Return the ID of the tool:
	return (pTI->uId);
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemView mouse handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnRButtonDown ()
//
// Description:
//	Process right button click messages for this view.  Select item and
//  display an appropriate popup menu.
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
void CKItemView::OnRButtonDown (UINT nFlags, CPoint point) 
	{
	// Create a menu object, to be filled in shortly:
	CMenu cMenu;

	// Get reference to our list control:
	CListCtrl &cList = GetListCtrl ();

	// See if the click was on an item.  Return value will be item index hit
	// was on, or -1 if hit was not on an item.  Additional information will
	// also be loaded into flags parameter.
	UINT uHitFlags;
	int nItem = cList.HitTest (point, &uHitFlags);

	// If hit was on an item (index not equal to -1, and LVHT_ONITEM flag set),
	// then select item and unselect all others:
	if (nItem >= 0 && (uHitFlags & LVHT_ONITEM)) 
		{
		// If item is not selected, we have work to do:
		if (!cList.GetItemState (nItem, LVIS_SELECTED))
			{
			// Get index of first selected item.  (-1 means start search from
			// beginning of list, flags mean search through list by index
			// for selected items).
			int nSelIndex = cList.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

			// If we found a selected item (nSelIndex not -1), then unselect
			// it and look for another selected item.  Continue until all
			// items have been examined:
			while (nSelIndex >= 0)
				{
				// Unselect item:
				cList.SetItemState (nSelIndex, ~LVIS_SELECTED, LVIS_SELECTED);

				// Get index of next selected item:
				nSelIndex = cList.GetNextItem (nSelIndex, LVNI_ALL | LVNI_SELECTED);
				}

			// Select and give focus to the item hit on:
			cList.SetItemState (nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}

	// Else just unselect all items:
	else
		{
		// Get index of first selected item:
		int nSelIndex = cList.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

		// If we found a selected item, unselect it and continue searching for
		// and unselecting items until all have been unselected:
		while (nSelIndex >= 0)
			{
			// Unselect item:
			cList.SetItemState (nSelIndex, ~LVIS_SELECTED, LVIS_SELECTED);

			// Get next selected item:
			nSelIndex = cList.GetNextItem (nSelIndex, LVNI_ALL | LVNI_SELECTED);
			}
		}

	// Convert hit point to screen coordinates:
	ClientToScreen (&point);	

	// Create a popup menu:
	if (cMenu.CreatePopupMenu ())
		{
		// Get pointer to main menu.  We will use it to get text for popup:
		CMenu *pMainMenu = AfxGetMainWnd ()->GetMenu ();

		// Create a string to contain menu item text:
		CString strMenuText;

		// Define flags to so that popup menu items will get text from pointer
		// to null terminated string.
		int nFlags = MF_ENABLED | MF_STRING;

		// Get pointer to selected group object:
		CKGroup *pSelGroup = GetSelectedGroup ();

		// If we get a valid group pointer, add menu items that allow for 
		// items to be added or pasted
		if (pSelGroup != NULL)
			{
			// Add Add New Item menu item:
			pMainMenu->GetMenuString (ID_EDIT_NEWITEM, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_EDIT_NEWITEM, strMenuText);

			// If item data is in clipboard, and no items are selected, 
			// allow paste:
			if (IsClipboardFormatAvailable (CF_ITEM) && !cList.GetSelectedCount ()) 
				{
				// Add separator:
				cMenu.AppendMenu (MF_SEPARATOR);

				// Add Paste menu item:
				pMainMenu->GetMenuString (ID_EDIT_PASTE, strMenuText, MF_BYCOMMAND);
				cMenu.AppendMenu (nFlags, ID_EDIT_PASTE, strMenuText);
				}
			}

		// If at least one item is selected, allow for item management 
		// and item read and write:
		if (cList.GetSelectedCount () > 0)
			{
			// Check that selected group pointer is set (debug only):
			ASSERT (pSelGroup != NULL);

			// Add separator:
			cMenu.AppendMenu (MF_SEPARATOR);

			// Add these menu items only if selected group is valid:
			if (pSelGroup->IsValid ())
				{
				// If item management is supported by selected group:
				if (pSelGroup->IsIItemMgtSupported ())
					{
					// Add Set Items Active menu item:
					pMainMenu->GetMenuString (ID_TOOLS_SET_ACTIVE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_SET_ACTIVE, strMenuText);
					
					// Add Set Items Inactive menu item:
					pMainMenu->GetMenuString (ID_TOOLS_SET_INACTIVE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_SET_INACTIVE, strMenuText);
					}

				// If synchronous i/o is supported by selected group:
				if (pSelGroup->IsISyncIOSupported ())
					{
					// Add separator:
					cMenu.AppendMenu (MF_SEPARATOR);

					// Add Sync Read From Cache menu item:
					pMainMenu->GetMenuString (ID_TOOLS_SYNC_READ_CACHE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_SYNC_READ_CACHE, strMenuText);
					
					// Add Sync Read From Device menu item:
					pMainMenu->GetMenuString (ID_TOOLS_SYNC_READ_DEVICE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_SYNC_READ_DEVICE, strMenuText);

					// Add Sync Write menu item:
					pMainMenu->GetMenuString (ID_TOOLS_SYNC_WRITE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_SYNC_WRITE, strMenuText);
					}

				// If asynchronous i/o 1.0 is supported by selected group:
				// (Make sure update method was not set for 2.0)
				if (pSelGroup->IsIAsyncIOSupported () && (pSelGroup->GetUpdateMethod () != OPC_20_DATACHANGE))
					{
					// Add separator:
					cMenu.AppendMenu (MF_SEPARATOR);

					// Add Async 1.0 Read From Cache menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC10_READ_CACHE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC10_READ_CACHE, strMenuText);

					// Add Async 1.0 Read From Device menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC10_READ_DEVICE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC10_READ_DEVICE, strMenuText);

					// Add Async 1.0 Refresh From Cache menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC10_REFRESH_CACHE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC10_REFRESH_CACHE, strMenuText);

					// Add Async 1.0 Refresh From Device menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC10_REFRESH_DEVICE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC10_REFRESH_DEVICE, strMenuText);

					// Add Async 1.0 Write menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC10_WRITE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC10_WRITE, strMenuText);
					}

				// If asynchronous i/o 2.0 is supported and is being used:
				if (pSelGroup->IsIAsyncIO2Supported () && (pSelGroup->GetUpdateMethod () == OPC_20_DATACHANGE))
					{
					// Add separator:
					cMenu.AppendMenu (MF_SEPARATOR);

					// Add Async 2.0 Read (only from device) menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC20_READ_DEVICE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC20_READ_DEVICE, strMenuText);

					// Add Async 2.0 Refresh From Cache menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC20_REFRESH_CACHE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC20_REFRESH_CACHE, strMenuText);

					// Add Async 2.0 Refresh From Device menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC20_REFRESH_DEVICE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC20_REFRESH_DEVICE, strMenuText);

					// Add Async 2.0 Write menu item:
					pMainMenu->GetMenuString (ID_TOOLS_ASYNC20_WRITE, strMenuText, MF_BYCOMMAND);
					cMenu.AppendMenu (nFlags, ID_TOOLS_ASYNC20_WRITE, strMenuText);
					}
				}

			// Add a separator if we have already added menu items:
			if (cMenu.GetMenuItemCount () > 0)
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

		// If popup menu is not empty, display it at hit point:
		if (cMenu.GetMenuItemCount () > 0)
			cMenu.TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd ());
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemView list handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Insert ()
//
// Description:
//	Insert an item into view.
//
// Parameters:
//  CKItem		*pItem		Pointer to item object to add.
//
// Returns:
//  void
// **************************************************************************
void CKItemView::Insert (CKItem *pItem)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csSortedList);

	// Make sure item pointer was set (debuf only):
	ASSERT (pItem != NULL);

	// If sorted item list is full, we need to allocate another block of
	// memory big enough for current list plus room for more items and
	// transfer current list to it:
	if (m_cnSortedItems == m_nSortedListSize)
		{
		DWORD *p = NULL;

		// Be prepared to deal with allocation problems (p will be NULL in
		// this event):
		try
			{
			// Allocate the next largest size:
			p = new DWORD [m_nSortedListSize + GROWLIST];

			// Initialize new memory:
			ZeroMemory (p, sizeof (sm_pSortedItemList [0]) * (m_nSortedListSize + GROWLIST));

			// Transfer existing items to the new memory:
			if (m_cnSortedItems)
				{
				// Copy old to new:
				memcpy (p, sm_pSortedItemList, m_cnSortedItems * sizeof (sm_pSortedItemList [0]));

				// Free old memory:
				delete [] sm_pSortedItemList;
				}

			// Point to new list:
			sm_pSortedItemList = p;

			// Increment allocated list size:
			m_nSortedListSize += GROWLIST;
			}
		
		catch (...)
			{
			p = NULL;
			}
		}

	// Add the item to the sorted list:
	sm_pSortedItemList [m_cnSortedItems++] = (DWORD)pItem;
	}

// **************************************************************************
// DeleteAllItems ()
//
// Description:
//	Delete all items.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::DeleteAllItems ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csSortedList);

	// Get reference to our list control:
	CListCtrl &cList = GetListCtrl ();

	// Reset sorted item list:
	ZeroMemory (sm_pSortedItemList,
		sizeof (sm_pSortedItemList[0]) * (m_nSortedListSize));

	// Reset sorted item counter:
	m_cnSortedItems = 0;

	// Set the list control item count to zero.  (This is how it must
	// be done for virtual list view style.  See comments in PreCreateWindow ()).
	cList.SetItemCountEx (0, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

	// Force a repaint of the view:
	cList.Invalidate (true);
	cList.UpdateWindow ();
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
void CKItemView::OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csSortedList);

	// Cast generic notification message pointer to list view notification
	// message pointer:
	LV_DISPINFO *plvdi = (LV_DISPINFO *)pNMHDR;

	// Get pointer to list view item information structure from message
	// notification structure:
	LV_ITEM *plvItem = &plvdi->item;

	// If there are no items in the sorted item list, then there is no
	// work to be done here.
	if (sm_pSortedItemList == NULL || m_cnSortedItems == 0)
		return;

	// If index of item we are being ased to supply information for is 
	// greater than the number of items in our sorted item list, then
	// there is something wrong.  Return immediately.
	if (plvItem->iItem >= m_cnSortedItems)
		return;

	// Get pointer to CKItem object from sorted item list:
	CKItem *pItem = (CKItem *)sm_pSortedItemList [plvItem->iItem];
	ASSERT (pItem != NULL);

	// If we are being asked to supply item text:
	if (plvItem->mask & LVIF_TEXT)
		{
		// See what subitem (column) text is needed for.  Copy requested
		// text to output structure:
		switch (plvItem->iSubItem)
			{
			case 0:	// Item ID
				lstrcpyn (plvItem->pszText, pItem->GetItemID (), plvItem->cchTextMax);
				break;

			case 1:	// Data Type
				{
				// Convert data type to string:
				static CString strDataType;
				StringFromVartype (pItem->GetDataType (), strDataType);

				lstrcpyn (plvItem->pszText, strDataType, plvItem->cchTextMax);
				}
				break;

			case 2:	// Value
				{
				static CString strValue;
				pItem->GetValue (strValue);

				lstrcpyn (plvItem->pszText, strValue, plvItem->cchTextMax);
				}
				break;

			case 3:	// Timestamp
				{
				static CString strTimestamp;
				pItem->GetTimeStamp (strTimestamp);

				lstrcpyn (plvItem->pszText, strTimestamp, plvItem->cchTextMax);
				}
				break;

			case 4: // Quality
				lstrcpyn (plvItem->pszText, pItem->GetQuality (), plvItem->cchTextMax);
				break;

			case 5:	// Update Count
				{
				// Format numerical value as string:
				TCHAR szNum [64];
				wsprintf (szNum, _T("%u"), pItem->GetUpdateCount ());
				
				lstrcpyn (plvItem->pszText, szNum, plvItem->cchTextMax);
				}
				break;

			default:
				break;
			}
		}

	// If we are being asked to supply item image.  Return index into
	// list control's image list:
	if (plvItem->mask & LVIF_IMAGE)
		{
		// If item is valid, specify active or inactive item image
		// as the case may be.
		if (pItem->IsValid ())
			plvItem->iImage = pItem->IsActive () ? 0 : 1;

		// Else specify invalid item image:
		else
			plvItem->iImage = 2;
		}
	}

// **************************************************************************
// OnToolTipText
//
// Description:
//	Called by framework when tool tip text needs to be defined.
//
// Parameters:
//	UINT		uID				Identifier of the control that sent the notification.
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  BOOL - FALSE.
// **************************************************************************
BOOL CKItemView::OnToolTipText (UINT uID, NMHDR *pNMHDR, LRESULT *pResult)
	{
	// Get pointers for output text:
	TOOLTIPTEXTA *pTTTA = (TOOLTIPTEXTA *) pNMHDR;
	TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *) pNMHDR;

	CString strText;
	int nRow = 0;
	int nCol = 0;

	// Get ID of cell tool tip is needed for:
	UINT nID = pNMHDR->idFrom;

	// Get reference to our list control:
	CListCtrl &cList = GetListCtrl ();

	// Filter automatically created tip with ID 0:
	if (nID == 0)
		return (FALSE);

	// Decode the row and column from the ID we created earlier in OnToolHitTest():
	nRow = ((nID - 1) >> 10) & 0x3FFFFFF;
	nCol = (nID - 1) & 0x3FF;

	// Get the text from this cell:
	strText = cList.GetItemText (nRow, nCol);

	// Whether or not this application has been built for unicode or ansi is
	// irrelevant.  We need to pass the tootip text based on the request from
	// the OS.

	// Set the text:
#ifndef _UNICODE
	// This is an ANSI build.

	// If request is for ANSI, just copy string:
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn (pTTTA->szText, strText, sizeof (pTTTA->szText) / sizeof (char));

	// Else request is for UNICODE, so convert from ANSI:
	else
		_mbstowcsz (pTTTW->szText, strText, sizeof (pTTTW->szText) / sizeof (WCHAR));
#else
	// This is a UNICODE build.

	// If request is for ANSI, convert from UNICODE:
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz (pTTTA->szText, strText, sizeof (pTTTA->szText) / sizeof (char));

	// Else request is for UNICODE, so just copy string:
	else
		lstrcpyn (pTTTW->szText, strText, sizeof (pTTTW->szText) / sizeof (WCHAR));
#endif

	*pResult = 0;
	return (FALSE);
	}

// **************************************************************************
// OnColumnClick ()
//
// Description:
//	List control column header click event handler.  Sort the items based
//	on values in column.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnColumnClick (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get column index from message notification structure:
	WORD wColumn = ((NM_LISTVIEW *)pNMHDR)->iSubItem;

	// Record change in sorting parameters:

	// If same sort column selected, toggle the sort order:
	if (wColumn == sm_wSortColumn)
		sm_wSortOrder = !sm_wSortOrder;

	// Else sort using data in new sort column:
	else
		sm_wSortColumn = wColumn;

	// Sort the list using new column and order settings:
	SortList ();
	}

// **************************************************************************
// CompareItems ()
//
// Description:
//	Compares the order of items when sorting by values in one of the list
//	control columns.  Used by the qsort function, so we need to declare
//	parameters as void pointers.
//
// Parameters:
//  const void	*arg1		Pointer to the first items.
//	const void	*arg2		Pointer to the second item.
//
// Returns:
//  int - Returns a negative number if the arg1 comes first, 0 if arg1 and
//	arg2 are the same, or a positive value if arg2 comes first.
// **************************************************************************
int CKItemView::CompareItems (const void *arg1, const void *arg2)
	{
	// Initialize a variable to contain comparison result:
	int nRC = 0;

	// Cast arguments to CKItem pointers (this what the calling routine
	// is really comparing).
	CKItem *pItem1 = (CKItem *)(*(DWORD *)arg1);
	CKItem *pItem2 = (CKItem *)(*(DWORD *)arg2);

	// Sort based on the column parameter passed to SortItems():
	switch (sm_wSortColumn)
		{
		case 0: // Item ID
			// Item ID is a string, so do a simple string compare:
			nRC = lstrcmp (pItem1->GetItemID (), pItem2->GetItemID ());
			break;

		case 1:	// Data Type
			{
			// Must get data types and convert them to strings:
			static CString strType1;
			static CString strType2;
			
			StringFromVartype (pItem1->GetDataType (), strType1);
			StringFromVartype (pItem2->GetDataType (), strType2);

			// Now compare strings:
			nRC = lstrcmp (strType1, strType2);
			}
			break;

		case 2:	// Value
			{
			// Get string representation of values:
			static CString strValue1;
			static CString strValue2;
			
			pItem1->GetValue (strValue1);
			pItem2->GetValue (strValue2);

			// Compare strings:
			nRC = lstrcmp (strValue1, strValue2);
			}
			break;

		case 3:	// Timestamp
			{
			// Get string representation of timestamp:
			static CString strTimeStamp1;
			static CString strTimeStamp2;
			
			pItem1->GetTimeStamp (strTimeStamp1);
			pItem2->GetTimeStamp (strTimeStamp2);

			// Compare strings:
			nRC = lstrcmp (strTimeStamp1, strTimeStamp2);
			}
			break;

		case 4: // Quality
			// Quality is a string, so do string compare:
			nRC = lstrcmp (pItem1->GetQuality (), pItem2->GetQuality ());
			break;

		case 5:	// Update Count
			{
			// Get update counts:
			DWORD dwUpdates1 = pItem1->GetUpdateCount ();
			DWORD dwUpdates2 = pItem2->GetUpdateCount ();

			// Compare numerical update counts:
			if (dwUpdates1 == dwUpdates2)
				nRC = 0;
			else if (dwUpdates1 > dwUpdates2)
				nRC = -1;
			else
				nRC = 1;
			}
			break;

		// Unexpected column index:
		default:
			ASSERT (FALSE);
			break;
		}

	// Equal items should be sorted by item ID:
	if (sm_wSortColumn && nRC == 0)
		nRC = lstrcmp (pItem1->GetItemID (), pItem2->GetItemID ());

	// If the order is descending, reverse the result:
	if (LOWORD (sm_wSortOrder) == DESCENDING)
		nRC = -nRC;

	// Return the result:
	return (nRC);
	}

// **************************************************************************
// SortList ()
//
// Description:
//	Sort items in the list control.  Registers a CompareItems function with
//	qsort for sort comparisons.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::SortList ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csSortedList);

	// Get reference to out list control:
	CListCtrl &cList = GetListCtrl ();

	// If there are no items in the list, then there is nothing to do:
	if (!m_cnSortedItems)
		return;

	// Get number of selected items:
	int cnSelections = cList.GetSelectedCount ();

	// Initialize a variable to contain the index of item with focus:
	int nFocus = -1;

	// Save off selected items so we can restore them after sort:
	CMemFile cMem (cnSelections * sizeof (DWORD));
	
	// Don't bother looking for selected items if there are none:
	if (cnSelections)
		{
		// Find the first selected item:
		int nSel = cList.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

		// Save the items index if it has the focus:
		if (cList.GetItemState (nSel, LVIS_FOCUSED) == LVIS_FOCUSED)
			nFocus = sm_pSortedItemList [nSel];

		// Clear the selection and store off the array element (pointer
		// to CKItem object):
		cList.SetItemState (nSel, 0, LVIS_SELECTED | LVIS_FOCUSED);
		cMem.Write (&sm_pSortedItemList [nSel], sizeof (DWORD));
		
		// Store off remaining selections:
		int n = cnSelections;
		
		while (--n)
			{
			// Get index of next selected item:
			nSel = cList.GetNextItem (nSel, LVNI_BELOW | LVNI_SELECTED);
			ASSERT (nSel != -1);
	
			// Store it's index if it has the focus:
			if (nFocus == -1 && cList.GetItemState (nSel, LVIS_FOCUSED) == LVIS_FOCUSED)
				nFocus = sm_pSortedItemList [nSel];

			// Clear the selection and store off the index:
			cList.SetItemState (nSel, 0, LVIS_SELECTED | LVIS_FOCUSED);
			cMem.Write (&sm_pSortedItemList [nSel], sizeof (DWORD));
			}
		}

	// Sort the list.  qsort function requires us to define a function to
	// compare items.
	qsort ((void *)sm_pSortedItemList, m_cnSortedItems, 
		sizeof (sm_pSortedItemList [0]), CompareItems);

	// Now restore original selections:
	if (cnSelections)
		{
		// Reset file pointer to beginning:
		cMem.SeekToBegin ();

		// There is one unsorted index in the memory file for each 
		// selection detected above:
		for (int i = 0; i < cnSelections; i++)
			{
			// Read the index (actually pointer to CKItem object cast as DWORD)
			// of first selected item:
			DWORD dwIndex;
			cMem.Read (&dwIndex, sizeof (DWORD));

			// Find the item's location in the sorted array.  Look at each
			// element in array until we find the item:
			for (int j = 0; j < m_cnSortedItems; j++)
				{
				// We have found the item if the array element is the original index:
				if (sm_pSortedItemList [j] == dwIndex)
					{
					// Make sure the focused item is visible:
					if (dwIndex == (DWORD)nFocus)
						{
						cList.EnsureVisible (j, false);
						cList.SetItemState (j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						}
					
					// Restore selection:
					else
						cList.SetItemState (j, LVIS_SELECTED, LVIS_SELECTED);
					
					// If we are here, we found the item.  No need to continue
					// searching so break out of loop.
					break;
					}
				}
			}
		}

	// Force the list control to repaint itself:
	GetListCtrl ().Invalidate (true);
	}

// **************************************************************************
// GetSelectedCount ()
//
// Description:
//	Get number of items currently selected.
//
// Parameters:
//  none
//
// Returns:
//  int	-  Number of selected items.
// **************************************************************************
int CKItemView::GetSelectedCount ()
	{
	// Call the list control's GetSelectedCount function and return result:
	return (GetListCtrl ().GetSelectedCount ());
	}

// **************************************************************************
// GetSelectedItems ()
//
// Description:
//	Load an object array with item objects currently selected in list control.
//
// Parameters:
//  CObArray	&cItemList		Output array of items.
//
// Returns:
//  int - Number of items loaded into array.
// **************************************************************************
int CKItemView::GetSelectedItems (CObArray &cItemList)
	{
	// Get number of selected items:
	DWORD dwSelectedCount = GetSelectedCount ();

	// If there are selected items, fill the object array with them:
	if (dwSelectedCount)
		{
		// Get reference to our list control:
		CListCtrl &cListCtrl = GetListCtrl ();

		// Initialize some variables use in search for selected items:
		int nSelIndex = -1;
		CKItem *pItem = NULL;
		DWORD dwCount = 0;

		// Be prepared to handle exceptions that might be thrown by
		// object array:
		try
			{
			// Allocate enough array memory to hold all the selected items:
			cItemList.SetSize (dwSelectedCount);
			
			// Get the first selected item:
			nSelIndex = cListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

			// Continue loading selected items until we get them all:
			while ((nSelIndex >= 0) && (dwCount <= dwSelectedCount))
				{
				// Insert the item this selection represents into the list:
				pItem = (CKItem *) sm_pSortedItemList [nSelIndex];
				ASSERT (pItem != NULL);

				// Add the item to the array:
				cItemList.SetAt (dwCount++, pItem);

				// See if there is another selected item below the current one:
				nSelIndex = cListCtrl.GetNextItem (nSelIndex, LVNI_ALL | LVNI_SELECTED);
				}
			}
		
		catch (...)
			{
			ASSERT (FALSE);
			dwSelectedCount = 0;
			}
		}

	// Return the number of selected items:
	return (dwSelectedCount);
	}

// **************************************************************************
// GetSelectedGroup ()
//
// Description:
//	Get pointer to currently selected group.
//
// Parameters:
//  none
//
// Returns:
//  CKGroup* - Pointer to selected group object.
// **************************************************************************
CKGroup* CKItemView::GetSelectedGroup ()
	{
	// Get pointer to out document object:
	CKDocument *pDoc = (CKDocument *) GetDocument ();
	ASSERT (pDoc != NULL);

	// Ask document for pointer to selected group object.  Document should
	// have been advised of current selection.
	CKGroup *pGroup = pDoc->GetSelectedGroup ();

	// Return pointer:
	return (pGroup);
	}

// **************************************************************************
// GetSelectedServer ()
//
// Description:
//	Get pointer to currently selected server.
//
// Parameters:
//  none
//
// Returns:
//  CKServer* - Pointer to selected server object.
// **************************************************************************
CKServer* CKItemView::GetSelectedServer ()
	{
	// Get pointer to out document object:
	CKDocument *pDoc = (CKDocument *) GetDocument ();
	ASSERT (pDoc != NULL);

	// Ask document for pointer to selected server object.  Document should
	// have been advised of current selection.
	CKServer *pServer = (pDoc->GetSelectedGroup ())->GetParentServer ();

	// Return pointer:
	return (pServer);
	}

// **************************************************************************
// GetCellRectFromPoint ()
//
// Description:
//	Return the cell boundaries that a point resides in, along with the 
//	associated list control row and column.
//
// Parameters:
//  CPoint		&cPoint		Point.
//	CRect		&rc			Rectangle that defines cell cPoint resides in.
//	int			*pCol		Column cPoint resides in.		
//
// Returns:
//  int	- The row cPoint resides in, or -1 if not in a cell.
// **************************************************************************
int CKItemView::GetCellRectFromPoint (CPoint &cPoint, CRect &rc, int *pCol) const
	{
	int nCol = 0;	
	int nRow = 0;
	int nBottom = 0;
	int cnColumns = 0;

	// Get reference to our list control:
	CListCtrl &cList = GetListCtrl ();

	// Get the row number of top item:
	nRow = cList.GetTopIndex ();

	// Compute the row number of bottom item:
	nBottom = nRow + cList.GetCountPerPage ();

	// Make sure bottom row number is valid:
	if (nBottom > cList.GetItemCount ())
		nBottom = cList.GetItemCount ();
	
	// Get the number of columns (better be NUMCOLUMNS):
	CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem (0);
	cnColumns = pHeader->GetItemCount ();	
	
	// Determine which row the hit occurred.  Loop over visible rows:
	for (; nRow <= nBottom; nRow++)	
		{
		// Get bounding rect of item:		
		cList.GetItemRect (nRow, &rc, LVIR_BOUNDS);		
		
		// If the point falls in bounds, we found the row:
		if (rc.PtInRect (cPoint))		
			{
			// Now find the column.  Loop over columns:
			for (nCol = 0; nCol < cnColumns; nCol++)
				{
				// Get the width of column:
				int nColWidth = cList.GetColumnWidth (nCol);				
				
				// If the within the column boundaries, we found the column:
				if (cPoint.x >= rc.left && cPoint.x <= (rc.left + nColWidth))				
					{
					// At this point, rc will describe all but the right
					// boundary of the cell.  The top and bottom we set
					// when we found the row.  The left boundary was set
					// when we checked this column.

					// Now get client area.  We will use it later:
					CRect rcClient;
					GetClientRect (&rcClient);

					// Set the column number for output, provided pointer
					// was set by colling function:
					if (pCol) 
						*pCol = nCol;

					// Adjust right boundary so that rc now full describes
					// the cell:
					rc.right = rc.left + nColWidth;

					// Adjust the right boundary again to ensure that it does
					// not exceed client area:
					if (rc.right > rcClient.right)
						rc.right = rcClient.right;

					// We have everything we need now, so return:
					return (nRow);				
					}				
				
				// Adjust the left boundary so we can check the next column:
				rc.left += nColWidth;			
				}		
			}	
		}

	// If we make it here, then hit was not over a cell.  Return
	// -1 to indicate this:
	return (-1);
	}

// **************************************************************************
// RequireCellToolTip ()
//
// Description:
//	Determins if a cell tool tip is required.  Tool tip is required if cell
//	is too narrow to display its full text.
//
// Parameters:
//  int			nRow		Row number of cell.
//	int			nCol		Column number of cell.
//	CRect		rc			Rectangle that bounds the cell.
//
// Returns:
//  bool - true if tool tip is requred.
// **************************************************************************
bool CKItemView::RequireCellToolTip (int nRow, int nCol, CRect rc) const
	{
	// Get reference to our list control:
	CListCtrl &cList = GetListCtrl ();

	// Get cell text:
	CString strText;
	strText = cList.GetItemText (nRow, nCol);	

	// Create a device context so we can get the cell text extent:
	CWindowDC dc (&cList);

	// Set the current font:
	dc.SelectObject (cList.GetFont ());

	// Get the size of the text (given device context and font):
	CSize size = dc.GetTextExtent (strText);

	// If the string is longer then the column width, then we will return
	// true (need tool tip).  Otherwise we will return false.  Subtract
	// twenty from cell's width for column buffering.
	return (size.cx >= (rc.right - rc.left - 20));
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemView diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CKItemView::AssertValid () const
	{
	CListView::AssertValid ();
	}

void CKItemView::Dump (CDumpContext &dc) const
	{
	CListView::Dump (dc);
	}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKItemView message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnTimer ()
//
// Description:
//	Timer event handler.  Do periodic maintenance of view.
//
// Parameters:
//	UINT		nIDEvent		Timer event type.
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnTimer (UINT nIDEvent) 
	{
	// Process according to timer event type:
	switch (nIDEvent)
		{
		// Update the item pane.  This will cause the items' value,
		// quality, timestamp, etc. to get refreshed.
		case UPDATE_ITEMPANE_EVENT:
			{
			// Get reference to our list control:
			CListCtrl &cList = GetListCtrl ();

			// If there are items in the list control, update the view:
			if (cList.GetItemCount () > 0)
				{
				int nTopIndex;
				CRect rc;
				CRect rcitem;
				
				// Get index of the first visible item:
				nTopIndex = cList.GetTopIndex ();

				// Get client area rectangle.  We will modify this rectangle to 
				// only include the area of the view we wish to refresh.
				cList.GetClientRect (&rc);

				// Get rectangel that bounds top visible item:
				cList.GetItemRect (nTopIndex, &rcitem, LVIR_BOUNDS);

				// Reset left bound of rc to exclude first column.  There 
				// is no need to refresh this (item ID and image).  We are 
				// interested in refreshing all other properties/columns.
				rc.left = cList.GetColumnWidth (0);

				// No need to refresh anything above top of uppermost item:
				rc.top = rcitem.top;
							
				// Force a repaint of the area:
				cList.InvalidateRect (&rc, FALSE);
				}
			}
			break;

		// Use default processing of all other event types:
		default:
			CListView::OnTimer (nIDEvent);
			break;
		}
	}

// **************************************************************************
// OnUpdateTimer ()
//
// Description:
//	Item Update Interval menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnUpdateTimer () 
	{
	// Create an update interval dialog.  Pass it the current update
	// interval.
	CKUpdateIntervalDlg dlg (m_wUpdateInterval);
	
	// Show as a modal dialog.  If user hits "OK", we will apply changes:
	if (dlg.DoModal () == IDOK)
		{
		// If the user specified an update interval that is different than
		// the current one, reset the timer and store new value.
		if (m_wUpdateInterval != (WORD) dlg.m_nInterval)
			{
			// Stop the update view timer:
			KillTimer (UPDATE_ITEMPANE_EVENT);

			// Store the new update interval:
			m_wUpdateInterval = (WORD) dlg.m_nInterval;

			// Restart update view timer with new interval:
			SetTimer (UPDATE_ITEMPANE_EVENT, m_wUpdateInterval, NULL);
			}
		}
	}

// **************************************************************************
// OnNewItem ()
//
// Description:
//	New item menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnNewItem () 
	{
	// Get pointer to our document object.  (This could be NULL if the 
	// document was not properly attached.)
	CKDocument *pDoc = (CKDocument *) GetDocument();

	// Tell document to add a new item:
	if (pDoc)
		pDoc->AddItem ();
	}

// **************************************************************************
// OnProperties ()
//
// Description:
//	Item Properties menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnProperties () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get a list of selected items:
	CObArray cItemList;
	DWORD cdwItems;
	cdwItems = GetSelectedItems (cItemList);

	// If there are selected items, display the properties dialog:
	if (cdwItems > 0)
		{
		// Get pointer to selected server:
		CKServer *pServer = GetSelectedServer ();
		ASSERT (pServer != NULL);

		// Get pointer to selected group:
		CKGroup *pGroup = GetSelectedGroup ();
		ASSERT (pGroup != NULL);

		// Construct an item properties dialog.  Pass it the array of 
		// selected items, the selected group's item management COM
		// interface pointer, and a pointer to the selected server.
		CKItemPropertiesDlg dlg (cItemList, 
			cdwItems, 
			pGroup->GetIItemMgt (),
			pServer);

		// So as modal dialog.  Any changes to properties will be made
		// in the dialog object.
		dlg.DoModal ();

		// If changes were made, set document's modified flag:
		if (dlg.IsModified ())
			((CKDocument *) GetDocument ())->SetModified (true);
		}
	}

// **************************************************************************
// OnCopy ()
//
// Description:
//	Item copy menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnCopy () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create a memory file object to copy selected item settings to:
	CFixedSharedFile sf;

	// Create an object array to contain selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill array with selected items:
	cdwItems = GetSelectedItems (cItemList);

	// If there are selected items, copy them to the shared memory file:
	if (cdwItems > 0)
		{
		try
			{
			// First write the number of items copied:
			sf.Write (&cdwItems, sizeof (cdwItems));
			
			// Copy the items:
			for (DWORD dwIndex = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to CKItem object from list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// Call item's copy function to write its settings to 
				// the shared memory file:
				pItem->Copy (sf);
				}
			}
		
		catch (...)
			{
			ASSERT (FALSE);
			return;
			}

		// Place the object data in the clipboard:
		sf.CopyToClipboard (CF_ITEM);
		}
	}

// **************************************************************************
// OnCut ()
//
// Description:
//	Item cut menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnCut () 
	{
	// Perform a copy first:
	OnCopy ();

	// Then delete:
	OnDelete ();
	}

// **************************************************************************
// OnPaste ()
//
// Description:
//	Item Paste menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnPaste () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Open the clipboard.  Return if we can't open it for some reason.
	if (OpenClipboard ())
		{
		// Get pointer to our document object.
		CKDocument *pDoc = (CKDocument *) GetDocument ();

		// Create a shared memory file to use to extract clipboard
		// contents from:
		CFixedSharedFile sf;

		// Create an object array to contain pointers to the 
		// items we will be adding shortly:
		CObArray cItemList;
		DWORD cdwItems;
		CKItem *pItem;

		// Get pointer to selected group.  This is where the items
		// will get added:
		CKGroup *pGroup = GetSelectedGroup ();
		ASSERT (pGroup != NULL);

		// Be prepared to handle execptions:
		try
			{
			// Is there item data in the clipboard?
			if (IsClipboardFormatAvailable (CF_ITEM))
				{
				// Set shared memory file handle to clipboard data:
				sf.SetHandle (GetClipboardData (CF_ITEM), FALSE);

				// Retrieve number of items and create and allocate enough
				// array memory to hold all of the item pointers:
				sf.Read (&cdwItems, sizeof (cdwItems));
				ASSERT (cdwItems > 0);

				// Reset the list control's size to accomodate new items:
				cItemList.SetSize (cdwItems + 1);
				DWORD dwIndex;
				// Now retrieve the items from the clipboard:
				for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
					{
					// Instantiate a new item object.  Pass it a pointer
					// to its parent group:
					pItem = new CKItem (pGroup);

					// Call the item's paste functin to read its properties
					// from the clipboard via the shared memory file:
					pItem->Paste (sf);

					// Add the new item to the object array:
					cItemList [dwIndex] = pItem;
					}

				// Terminate the list.  Will allow us to process list using
				// "while (element)" loop if we want:
				cItemList [dwIndex] = NULL;

				// Tell the document to add the items:
				pDoc->AddItems (cItemList, cdwItems);
				}

			// If item data is not in clipboard, then there is a problem.
			// throw exception to be caught below:
			else
				throw (-1);
			}
		
		catch (...)
			{
			ASSERT (FALSE);
			EmptyClipboard (); 
			}

		// Free the shared memory handle and close the clipboard:
		sf.Detach ();
		CloseClipboard ();
		}
	}

// **************************************************************************
// OnDelete ()
//
// Description:
//	Item Delete menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnDelete () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the array with pointers to the selected items:
	cdwItems = GetSelectedItems (cItemList);

	// If there are any selected items, remove them from the project:
	if (cdwItems)
		{
		// Get pointer to our document object:
		CKDocument *pDoc = (CKDocument *) GetDocument ();

		// Tell the document to remove the items pointed to in
		// the object array:
		pDoc->RemoveItems (cItemList, cdwItems);
		}
	}

// **************************************************************************
// OnSetActive ()
//
// Description:
//	Item Set Active menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnSetActive () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to set the items in the list active.  If this
	// succeeds, set the document modified flag:
	if (pSelGroup->SetItemActiveState (cItemList, cdwItems, true))
		((CKDocument *) GetDocument ())->SetModified (true);
	}

// **************************************************************************
// OnSetInactive ()
//
// Description:
//	Item Set Inactive menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnSetInactive () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to set the items in the list inactive.  If this
	// succeeds, set the document modified flag:
	if (pSelGroup->SetItemActiveState (cItemList, cdwItems, false))
		((CKDocument *) GetDocument ())->SetModified (true);
	}

// **************************************************************************
// OnSyncReadCache ()
//
// Description:
//	Item Sync Cache Read Cache menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnSyncReadCache () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to sync read from cache all items in array:
	pSelGroup->ReadSync (cItemList, cdwItems, false);
	}

// **************************************************************************
// OnSyncReadDevice ()
//
// Description:
//	Item Sync Device Read menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnSyncReadDevice () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to sync read from device all items in array:
	pSelGroup->ReadSync (cItemList, cdwItems, true);
	}

// **************************************************************************
// OnSyncWrite ()
//
// Description:
//	Item Sync Write menu evnet handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnSyncWrite () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Create a dialog to write values to these items:
	CKItemWriteDlg dlg (pSelGroup, cItemList, cdwItems, WRITE_SYNC);

	// Show as modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnAsync10ReadCache ()
//
// Description:
//	Item Async Cache Read 1.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync10ReadCache () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 10 read from cache all items in array:
	pSelGroup->ReadAsync10 (cItemList, cdwItems, false);
	}

// **************************************************************************
// OnAsync10ReadDevice ()
//
// Description:
//	Async Device Read 1.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync10ReadDevice () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 10 read from device all items in array:
	pSelGroup->ReadAsync10 (cItemList, cdwItems, true);
	}

// **************************************************************************
// OnAsync10RefreshCache ()
//
// Description:
//	Async Cache Refresh 1.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync10RefreshCache () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 10 refresh from cache all of its items:
	pSelGroup->RefreshAsync10 (false);
	}

// **************************************************************************
// OnAsync10RefreshDevice ()
//
// Description:
//	Item Async Device Refresh 1.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync10RefreshDevice () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 10 refresh from device all of its items:
	pSelGroup->RefreshAsync10 (true);
	}

// **************************************************************************
// OnAsync10Write ()
//
// Description:
//	Item Async Write 1.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync10Write () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Create a dialog to write values to these items:
	CKItemWriteDlg dlg (pSelGroup, cItemList, cdwItems, WRITE_ASYNC10);

	// Show as modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnAsync20ReadDevice ()
//
// Description:
//	Item Async Device Read 2.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync20ReadDevice () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 20 read (from device always) all items in 
	// array:
	pSelGroup->ReadAsync20 (cItemList, cdwItems);
	}

// **************************************************************************
// OnAsync20RefreshCache ()
//
// Description:
//	Item Async Cache Refresh 2.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync20RefreshCache () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 20 refresh from cache all of its items:
	pSelGroup->RefreshAsync20 (false);
	}

// **************************************************************************
// OnAsync20RefreshDevice ()
//
// Description:
//	Item Async Device Refresh 2.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync20RefreshDevice () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Tell the group to async 20 refresh from device all of its items:
	pSelGroup->RefreshAsync20 (true);
	}

// **************************************************************************
// OnAsync20Write ()
//
// Description:
//	Item Async Write 2.0 menu event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnAsync20Write () 
	{
	// Create an object array to contain pointers to the selected items:
	CObArray cItemList;
	DWORD cdwItems;

	// Fill the object array with pointers to selected items:
	cdwItems = GetSelectedItems (cItemList);

	// Get the selected group:
	CKGroup *pSelGroup = GetSelectedGroup ();
	ASSERT (pSelGroup != NULL);

	// Create a dialog to write values to these items:
	CKItemWriteDlg dlg (pSelGroup, cItemList, cdwItems, WRITE_ASYNC20);

	// Show as modal dialog:
	dlg.DoModal ();
	}

// **************************************************************************
// OnDblclk ()
//
// Description:
//	Mouse double click event handler.  Show item properties dialog.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKItemView::OnDblclk (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Use existing code in OnProperties() to show item properties dialog:
	OnProperties ();

	*pResult = 0;
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
void CKItemView::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags) 
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
		// Initialize variable to contain first selected item index:
		int nItem = -1;

		// Get reference to our list control:
		CListCtrl &cList = GetListCtrl ();

		// Get the first selected item:
		nItem = cList.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

		// Show properties of selected item, if there is one:
		if (nItem >= 0)
			{
			// Make sure item is visible:
			cList.EnsureVisible (nItem, false);

			// Get a point located in this item's rectangle:
			CRect rc;
			cList.GetItemRect (nItem, &rc, LVIR_LABEL);

			// Compute the coordinates of rectangle center:
			rc.left = (rc.left + rc.right) / 2;
			rc.top = (rc.top + rc.bottom) / 2;

			// Use existing code in OnRButtonDown() to display context menu
			OnRButtonDown (0, CPoint (rc.left, rc.top));
			}

		// Return now so default processing does not occur:
		return;
		}

	// Show properties on return:
	else if (nChar == VK_RETURN)
		{
		// If there are selected items, use exisitng code in OnProperties() to
		// display properties dialog:
		if (GetSelectedCount ())
			OnProperties ();

		// Return now so default processing does not occur:
		return;
		}
	
	// Perform default processing:
	CListView::OnChar (nChar, nRepCnt, nFlags);
	}
