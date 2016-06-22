// **************************************************************************
// eventview.cpp
//
// Description:
//	Implements a CListView derived class.  This the bottom pane of our GUI 
//	which shows event messages.
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
#include "eventview.h"

#define NUMCOLUMNS			3
#define DEFAULTCOLUMNWIDTH	120
#define GROWEVENTS			1024
#define MAXEVENTS			GROWEVENTS

static LPCTSTR lpszRegSection = _T("Event View");
static LPCTSTR lpszLogErrorsOnly = _T("Log Errors Only");

// event pane update timer
#define UPDATE_EVENTPANE_EVENT	1


/////////////////////////////////////////////////////////////////////////////
// CKEventView
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKEventView, CListView)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKEventView, CListView)
	//{{AFX_MSG_MAP(CKEventView)
	ON_WM_DESTROY ()
	ON_NOTIFY_REFLECT (LVN_GETDISPINFO, OnGetDispInfo)
	ON_COMMAND (ID_VIEW_CLEAR, OnClear)
	ON_WM_RBUTTONDOWN ()
	ON_WM_TIMER ()
	ON_WM_CHAR ()
	ON_COMMAND (ID_VIEW_ERRORONLY, OnLogErrorsOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKEventView ()
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
CKEventView::CKEventView ()
	{
	// Initialize member variables:
	m_cnEvents = 0;
	m_cnAllocEvents = 0;
	m_pEventList = NULL;

	m_bAutoScroll = true;
	m_bLogErrorsOnly = false;

	m_cnPendingEvents = 0;
	m_cnPendingAllocEvents = 0;
	m_pPendingEventList = NULL;
	}

// **************************************************************************
// ~CKEventView ()
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
CKEventView::~CKEventView ()
	{
	int i;

	// Free events:
	for (i = 0; i < m_cnEvents; i++)
		{
		delete m_pEventList [i];
		m_pEventList [i] = NULL;
		}

	// Free event list:
	if (m_pEventList)
		delete [] m_pEventList;	

	// Free pending events:
	for (i = 0; i < m_cnPendingEvents; i++)
		{
		delete m_pPendingEventList [i];
		m_pPendingEventList [i] = NULL;
		}

	// Free pending event list:
	if (m_pPendingEventList)
		delete [] m_pPendingEventList;
	}


/////////////////////////////////////////////////////////////////////////////
// CKEventView drawing
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnDraw ()
//
// Description:
//	OnDraw event handler.  This function needs to be implemented since it is
//  declared as a pure virtual function in base class.  We don't need to do
//  anything with it in this case.
//
// Parameters:
//  CDC			*pDC		Pointer to view's device context.
//
// Returns:
//  void
// **************************************************************************
void CKEventView::OnDraw (CDC *pDC)
	{
	// This is how we get our document object in case we need to do something
	// with this function at some point:
	CDocument* pDoc = GetDocument ();
	}


/////////////////////////////////////////////////////////////////////////////
// CKEventView diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CKEventView::AssertValid () const
	{
	CListView::AssertValid ();
	}

void CKEventView::Dump (CDumpContext &dc) const
	{
	CListView::Dump (dc);
	}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKEventView message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// PreCreateWindow ()
//
// Description:
//	PreCreateWindow event handler.
//
// Parameters:
//  CREATESTRUCT	&cs		Defines the initialization parameters for window.
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKEventView::PreCreateWindow (CREATESTRUCT &cs) 
	{
	// Perform default processing.  Return FALSE if failure:
	if (!CListView::PreCreateWindow (cs))
		return (FALSE);

	// Modify the window style so that list view has report format and
	// column headers that do not sort rows:
	cs.style |= LVS_REPORT | LVS_NOSORTHEADER; 

	// Return TRUE to indicate success:
	return (TRUE);
	}

// **************************************************************************
// Create ()
//
// Description:
//	Creates the CKEventView window and attaches it to parent window.
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
BOOL CKEventView::Create (LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext) 
	{
	// Perform default processing.  Return FALSE if failure:
	if (!CWnd::Create (lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
		return (FALSE);

	// Initialize the image list for the items.  These are the images that
	// show up to the left of each item in list.  The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 16x16 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	//
	// Image number		Use
	//	0				Information event
	//	1				Error event
	m_cImageList.Create (IDB_EVENTIMAGES, 16, 4, RGB (255, 0, 255));
	m_cImageList.SetBkColor (CLR_NONE);
	GetListCtrl ().SetImageList (&m_cImageList, LVSIL_SMALL);

	// Create columns.  Column width used in last session were saved in
	// registry. Use these widths for the present session.
	CString strColumnTitle;
	for (int i = 0; i < NUMCOLUMNS; i++)
		{
		// Create a string that will identify the registry entry for each column:
		TCHAR szNum [8];
		wsprintf (szNum, _T("%d"), i);

		// Read the column width from the registry:
		int nWidth = AfxGetApp ()->GetProfileInt (lpszRegSection, szNum, DEFAULTCOLUMNWIDTH);

		// Make sure it is valid:
		if (nWidth < 0)
			nWidth = DEFAULTCOLUMNWIDTH;

		// Load the column title string resources:
		switch (i)
			{
			case 0: // Date
				strColumnTitle.LoadString (IDS_DATE);
				break;

			case 1: // Time
				strColumnTitle.LoadString (IDS_TIME);
				break;

			case 2:	// Event
				strColumnTitle.LoadString (IDS_EVENT);
				break;

			default:
				ASSERT (FALSE);
				break;
			}

		// Insert the column:
		GetListCtrl ().InsertColumn (i, strColumnTitle, LVCFMT_LEFT, nWidth);
		}

	// Get the "log errors only" flag saved in registry last time:
	m_bLogErrorsOnly = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszLogErrorsOnly, FALSE) ? true : false;

	// Start update view timer:
	SetTimer (UPDATE_EVENTPANE_EVENT, 100, NULL);

	// Return TRUE to indicate success:
	return (TRUE);
	}

// **************************************************************************
// OnDestroy ()
//
// Description:
//	Called when window is destroyed.  Take opportunity to kill timer and save
//	view settings to registry.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKEventView::OnDestroy () 
	{
	CListView::OnDestroy ();

	// Stop update view timer:
	KillTimer (UPDATE_EVENTPANE_EVENT);
	
	// Save column widths in registry so we can use them next time:
	for (int i = 0; i < NUMCOLUMNS; i++)
		{
		// Create a string that will identify the registry entry for each
		// column:
		TCHAR szNum [8];
		wsprintf (szNum, _T("%d"), i);

		// Save the width:
		AfxGetApp ()->WriteProfileInt (lpszRegSection, szNum, GetListCtrl ().GetColumnWidth (i));
		}

	// Save "log errors only" flag in registry too:
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszLogErrorsOnly, m_bLogErrorsOnly);
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
void CKEventView::OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Extract pointer to list view item and  associated CKEvent object
	// from pNMHDER.  The CKEvent object describes the event and is 
	// associated when a list view item is created through the item's lParam.
	LV_DISPINFO *plvdi = (LV_DISPINFO *)pNMHDR;
	LV_ITEM *pItem = &plvdi->item;
	CKEvent *pEvent = (CKEvent *)pItem->lParam;

	ASSERT (pEvent);

	// The item's mask text bit will be set if we are being asked to supply
	// the item's text.  The subitem will indicate what column the text is
	// needed for.
	if (pItem->mask & LVIF_TEXT)
		{
		switch (pItem->iSubItem)
			{
			case 0:	// Date
				pEvent->FormatDate (pItem->pszText, pItem->cchTextMax);
				break;

			case 1:	// Time
				pEvent->FormatTime (pItem->pszText, pItem->cchTextMax);
				break;

			case 2:	// Event
				lstrcpyn (pItem->pszText, pEvent->GetMessage (), pItem->cchTextMax);
				break;

			default:
				ASSERT (FALSE);
				break;
			}
		}

	// The item's mask image bit will be set if we are being asked to supply
	// the items image (index into image list.  It the event type is 
	// tEventInformation we will supply the first image in the image list,
	// else we will supply the second (error event).
	if (pItem->mask & LVIF_IMAGE)
		pItem->iImage = (pEvent->GetType () == tEventInformation) ? 0 : 1;
	}

// **************************************************************************
// OnClear ()
//
// Description:
//	ID_VIEW_CLEAR event handler.  Delete all items from list view and redraw.	
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKEventView::OnClear () 
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock sf (&m_csLog);

	// Postpone any redrawing until we are done.  This will make things 
	// go faster and look smoother.
	SetRedraw (false);

	// Delete all items in list control:
	GetListCtrl ().DeleteAllItems ();

	// Delete any CKEvent objects associated with deleted list control items:
	for (int i = 0; i < m_cnEvents; i++)
		{
		delete m_pEventList [i];
		m_pEventList [i] = NULL;
		}

	// Reset event count to zero:
	m_cnEvents = 0;

	// Now allow view to redraw:
	SetRedraw (true);
	}

// **************************************************************************
// OnLogErrorsOnly ()
//
// Description:
//	ID_VIEW_ERRORONLY event handler.  Toggles the log errors only flag.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKEventView::OnLogErrorsOnly ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock sf (&m_csLog);

	// Toggle the log errors only flag:
	m_bLogErrorsOnly = m_bLogErrorsOnly ? false : true;
	}

// **************************************************************************
// OnRButtonDown ()
//
// Description:
//	Right mouse button down event handler.  Used to display context menu.
//
// Parameters:
//	UINT		nFlags		Flags
//	CPoint		point		Mouse cursor location.
//
// Returns:
//  void
// **************************************************************************
void CKEventView::OnRButtonDown (UINT nFlags, CPoint point) 
	{
	CMenu cMenu;

	// Convert point to screen coordinates:
	ClientToScreen (&point);	

	// Create a popup menu:
	if (cMenu.CreatePopupMenu ())
		{
		// Get pointer to our main menu.  We will use it to get text for 
		// the popup menu:
		CMenu *pMainMenu = AfxGetMainWnd ()->GetMenu ();

		// Declare a container for menu text:
		CString strMenuText;

		// Define flags to so that popup menu items will get text from pointer
		// to null terminated string.
		int nFlags = MF_ENABLED | MF_STRING;

		// If we have event, then add a "clear view" menu item:
		if (m_cnEvents)
			{
			pMainMenu->GetMenuString (ID_VIEW_CLEAR, strMenuText, MF_BYCOMMAND);
			cMenu.AppendMenu (nFlags, ID_VIEW_CLEAR, strMenuText);
			}

		// Add a "log errors only" menu item:
		pMainMenu->GetMenuString (ID_VIEW_ERRORONLY, strMenuText, MF_BYCOMMAND);
		cMenu.AppendMenu (nFlags, ID_VIEW_ERRORONLY, strMenuText);

		// Place the popup menu at the point of right mouse click, and route
		// all WM_COMMAND messages though the frame:
		cMenu.TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd ());
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKEventView log message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// LogMsg ()
//
// Description:
//	Log a message to the event log.  Creates an CKEvent object and adds it to 
//	the pending event list.  Event will be taken from pending event list
//	and placed in list control in OnTimer().
//
// Parameters:
//  EVENTTYPE	eType			Event type (tEventError, tEventWarning, etc)
//	LPCTSTR		lpszMessage		Pointer to message string.
//
// Returns:
//  void
// **************************************************************************
void CKEventView::LogMsg (EVENTTYPE eType, LPCTSTR lpszMessage)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock sf (&m_csLog);

	// No need to add the event if it is not an error and we are
	// logging errors only:
	if (m_bLogErrorsOnly && eType != tEventError)
		return;

	// Create an event and add to pending event list.  Wrap with exception
	// handler in case of memory allocation propbles (not too likely).
	try
		{
		// Instantiate a new CKEvent object to contain information about
		// this event:
		CKEvent *pEvent = new CKEvent (lpszMessage, eType);

		// Check for room in the pending events list.  If we are out of
		// room, allocate larger block of memory and transfer existing
		// events to it.
		if (m_cnPendingEvents == m_cnPendingAllocEvents)
			{
			// Create a pointer to a new pending events array:
			CKEvent **p;

			// Allocate memory for the new array/list large enough for 
			// existing events plus room for new ones. (This is an array
			// of pointers to CKEvent objects.)
			p = new CKEvent * [m_cnPendingAllocEvents + GROWEVENTS];

			// Transfer existing events to the new memory and free memory
			// used by old list:
			if (m_cnPendingEvents)
				{
				memcpy (p, m_pPendingEventList, m_cnPendingEvents * sizeof (m_pPendingEventList[0]));
				delete [] m_pPendingEventList;
				}

			// Reset pointer to pending event list to give new memory area:
			m_pPendingEventList = p;

			// Increment size of memory allocated for new pending event list:
			m_cnPendingAllocEvents += GROWEVENTS;
			}

		// Add the item to the event list and increment the pending events
		// counter:
		m_pPendingEventList [m_cnPendingEvents++] = pEvent;
		}

	catch (...)
		{
		TRACE (_T("OTC: Exception handler invoked adding an event to the log\n"));
		}
	}

// **************************************************************************
// AddEvent ()
//
// Description:
//	Places an event in list control
//
// Parameters:
//  CKEvent		*pEvent		Pointer to event object
//
// Returns:
//  bool - true if success.
// **************************************************************************
bool CKEventView::AddEvent (CKEvent *pEvent)
	{
	// Create a list view item structure:
	LV_ITEM lvi;

	// Get reference to list view's list control:
	CListCtrl &cList = GetListCtrl ();

	// Fill the list view item structure:

	// Item props we will fill (text, parameter, image):
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	
	// No sub items:
	lvi.iSubItem = 0;

	// Defer to OnGetDispInfo () to get image:
	lvi.iImage = I_IMAGECALLBACK;

	// Defer to OnGetDispInfo () to get text:
	lvi.pszText = LPSTR_TEXTCALLBACK;

	// Set item number:
	lvi.iItem = cList.GetItemCount ();

	// We will use the list view item parameter to contain a pointer to
	// the associated CKEvent object.  The CKEvent object contains
	// contains information about the event.
	lvi.lParam = (LPARAM)pEvent;
	
	ASSERT (pEvent != NULL);

	// Insert the new item into list control:
	int nIndex = cList.InsertItem (&lvi);
	
	// If auto scroll flag is set, and new item's index is good, then force
	// list control to scroll if necessary to show new item:
	if (m_bAutoScroll && nIndex != -1)
		cList.EnsureVisible (nIndex, false);

	// If new item's index is -1 there was a problem, so return false
	// to indicate this.  Else return true to indicate success:
	return (nIndex != -1);
	}

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
void CKEventView::OnTimer (UINT nIDEvent) 
	{
	// Process according to timer event type:
	switch (nIDEvent)
		{
		// This is the view maitenance timer event type we defined:
		case UPDATE_EVENTPANE_EVENT:
			{
			// Create a CSafeLock to make this object thread safe.  Our critical
			// section gets locked here, and will automatically be unlocked when the
			// CSafeLock goes out of scope.
			CSafeLock sf (&m_csLog);

			// Get reference to list view's list control:
			CListCtrl &cList = GetListCtrl ();

			// Create a scratch pointer to event object:
			CKEvent *pEvent = NULL;
			int nIndex = 0;

			// If we have pending event we will be adding to view, postpone
			// redrawing until we are done.  This will make things go faster
			// and look smoother.
			if (m_cnPendingEvents)
				cList.SetRedraw (false);

			// Process all event in pending events list:
			while (nIndex < m_cnPendingEvents)
				{
				// Get next event in pending events list:
				pEvent = m_pPendingEventList [nIndex];
				ASSERT (pEvent != NULL);

				// Invalidate pending event:
				m_pPendingEventList [nIndex++] = NULL;

				// If we have reached the self-imposed limit of displayable
				// events then remove the first one in list control to make
				// room:
				if (m_cnEvents == MAXEVENTS)
					{
					// Delete first item in list (FIFO):
					cList.DeleteItem (0);

					// Delete the associated event object:
					delete m_pEventList [0];

					// Scroll events up one notch:
					memcpy (m_pEventList, &m_pEventList[1], (m_cnEvents - 1) * sizeof (m_pEventList[0]));
					
					// Decrement the number of events:
					--m_cnEvents;
					}

				// Check for room in the events list.  If we are out of
				// room, allocate larger block of memory and transfer
				// existing events to it.
				else if (m_cnEvents == m_cnAllocEvents)
					{
					// Create a pointer to new events array:
					CKEvent **p;

					// Allocate memory for the new array/list large enough
					// for existing events plus room for new ones. (This is
					// an array of pointers to CKEvent objects.)
					p = new CKEvent * [m_cnAllocEvents + GROWEVENTS];

					// Transfer existing events to the new memory and free
					// memory used by old list:
					if (m_cnEvents)
						{
						memcpy (p, m_pEventList, m_cnEvents * sizeof (m_pEventList[0]));
						delete [] m_pEventList;
						}
					
					// Reset pointer to event list to give new memory area:
					m_pEventList = p;

					// Increment size of memory allocated for new event list:
					m_cnAllocEvents += GROWEVENTS;
					}

				// Add the item to the event list and increment index for next
				// item in pending events list:
				m_pEventList[m_cnEvents++] = pEvent;

				// Add the item to the list control:
				AddEvent (pEvent);
				}

			// We should have processed all items in pending event list.
			// If there were pending items, we need to reset pending
			// event counter to zero, restore the list control redraw flag,
			// and invalidate the list control to force a redraw.
			if (m_cnPendingEvents)
				{
				m_cnPendingEvents = 0;
				cList.SetRedraw (true);
				cList.Invalidate ();
				}
			}
			break;

		// Perform default processing for all other timer event types:
		default:
			CListView::OnTimer (nIDEvent);
			break;
		}
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
void CKEventView::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags) 
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
		// Initialize item index to -1, to indicate no item selected:
		int nItem = -1;

		// Get reference to list control:
		CListCtrl &cList = GetListCtrl ();

		// Get the first selected item:
		nItem = cList.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

		// Get a sensible rectangular region to place the context menu in:
		CRect rc;

		// If there is a selected item, use its rectangle:
		if (nItem >= 0)
			{
			// Force the list control to scroll if necessary to show 
			// selected item:
			cList.EnsureVisible (nItem, false);

			// Get the item's rectangle:
			cList.GetItemRect (nItem, &rc, LVIR_LABEL);
			}

		// Otherwise use the window's rectangle:
		else
			{
			// Get the window's rectangle:
			GetWindowRect (&rc);

			// Must convert dimensions returned by GetWindowRect():
			ScreenToClient (&rc);
			}

		// Compute coordinates of the rectangle's center:
		rc.left = (rc.left + rc.right) / 2;
		rc.top = (rc.top + rc.bottom) / 2;

		// Display our context menu at rectangle's center.  No point in
		// reinventing the wheel, use the code already developed for right
		// mouse down events to do this:
		OnRButtonDown (0, CPoint (rc.left, rc.top));

		// Return now so default processing does not occur:
		return;
		}

	// Perform default processing for all other characters:
	CListView::OnChar (nChar, nRepCnt, nFlags);
	}
