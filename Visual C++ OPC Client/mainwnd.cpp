// **************************************************************************
// mainwnd.cpp
//
// Description:
//	Implements a CFrameWnd derived class.  This the main window of our GUI
//	which contains the group view, item view, and event view panes - the view
//	parts of our MFC SDI document/view architecture.
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
#include "mainwnd.h"
#include "document.h"
#include "groupview.h"
#include "itemview.h"
#include "eventview.h"
#include "server.h"
#include "group.h"

// Registry entry names:
static LPCTSTR lpszMainWindow	= _T("Main Window");
static LPCTSTR lpszX			= _T("x");
static LPCTSTR lpszY			= _T("y");
static LPCTSTR lpszCX			= _T("cx");
static LPCTSTR lpszCY			= _T("cy");
static LPCTSTR lpszState		= _T("State");
static LPCTSTR lpszGroups		= _T("Groups");
static LPCTSTR lpszEvents		= _T("Events");
static LPCTSTR lpszToolbar		= _T("Toolbar");
static LPCTSTR lpszStatusbar	= _T("Status Bar");

// Status bar string resources:
#define ID_HELPTEXT		0
#define ID_ITEMCOUNT	1

static UINT auIDs[] = 
	{
	ID_HELPTEXT,
	ID_ITEMCOUNT,
	};

// For window update timer:
#define STATUSUPDATETIMER		1
#define STATUSUPDATEINTERVAL	250


/////////////////////////////////////////////////////////////////////////////
// CKMainWnd
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CKMainWnd, CFrameWnd)

// **************************************************************************
BEGIN_MESSAGE_MAP (CKMainWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CKMainWnd)
	ON_WM_CLOSE ()
	ON_WM_ENDSESSION ()
	ON_UPDATE_COMMAND_UI (ID_APP_ABOUT, OnUpdateCmdUI)
	ON_WM_TIMER ()
	ON_UPDATE_COMMAND_UI (ID_APP_EXIT, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_EDIT_COPY, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_EDIT_CUT, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_EDIT_PASTE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_EDIT_DELETE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_NEW, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_OPEN, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_SAVE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_SAVE_AS, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE1, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE2, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE3, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE4, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE5, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE6, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE7, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE8, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE9, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE10, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE11, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE12, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE13, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE14, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE15, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_FILE_MRU_FILE16, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_VIEW_CLEAR, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_VIEW_ERRORONLY, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_SYNC_WRITE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC10_WRITE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC20_WRITE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_SET_ACTIVE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_SET_INACTIVE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_SYNC_READ_CACHE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_SYNC_READ_DEVICE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC10_READ_CACHE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC10_READ_DEVICE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC20_READ_DEVICE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC10_REFRESH_CACHE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC10_REFRESH_DEVICE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC20_REFRESH_CACHE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ASYNC20_REFRESH_DEVICE, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_GET_ERROR_STRING, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_CLONE_GROUP, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_GET_GROUP_BY_NAME, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_ENUMERATE_GROUPS, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_CONNECT, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_DISCONNECT, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_RECONNECT, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_EXPORT_CSV, OnUpdateCmdUI)
	ON_UPDATE_COMMAND_UI (ID_TOOLS_IMPORT_CSV, OnUpdateCmdUI)
	ON_WM_DESTROY ()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE (ID_EDIT_NEWSERVER, ID_EDIT_DELETE, OnUpdateCmdUI)
	
	ON_MESSAGE (UM_SERVER_SHUTDOWN, OnServerShutdownRequest)
	ON_MESSAGE (UM_SELECT_GROUP, OnSelectGroup)
	ON_MESSAGE (UM_REFRESH_ITEMVIEW, OnRefreshItemView)
	ON_MESSAGE (UM_ITEM_READD, OnItemReAdd)
	ON_MESSAGE (UM_CHANGEVIEW, OnChangeView)
END_MESSAGE_MAP ()


/////////////////////////////////////////////////////////////////////////////
// CKMainWnd construction/destruction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKMainWnd ()
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
CKMainWnd::CKMainWnd ()
	{
	// Initialize member variables:
	m_bLoaded = false;
	m_pEventLog = NULL;
	m_cnItems = 0;

	m_strIdleStatusText.LoadString (AFX_IDS_IDLEMESSAGE); 
	m_strStatusText = m_strIdleStatusText;
	m_bForceStatusText = false;
	}

// **************************************************************************
// ~CKMainWnd ()
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
CKMainWnd::~CKMainWnd ()
	{
	}

// **************************************************************************
// LoadSettings ()
//
// Description:
//	Called when the application's window settings should be loaded from the
//	registry, or set to the defaults.  Settings saved at end of each session.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::LoadSettings ()
	{
	// Get pointer to application root object:
	CWinApp *pApp =  AfxGetApp ();
	ASSERT (pApp != NULL);

	// Define "Main Window" registry section string:
	CString strSection = lpszMainWindow;
	CString strKey;

	// Get height and witdh of screen (in pixels) to use as default if 
	// registry settings not found:
	int cx = GetSystemMetrics (SM_CXSCREEN);
	int cy = GetSystemMetrics (SM_CYSCREEN);

	// Get position of main window:
	int x = pApp->GetProfileInt (strSection, lpszX, 10);
	int y = pApp->GetProfileInt (strSection, lpszY, 10);

	// Get width and height of main window:
	int nWidth = pApp->GetProfileInt (strSection, lpszCX, cx / 2);
	int nHeight = pApp->GetProfileInt (strSection, lpszCY, cy / 2);

	// Get the state of main window when it was last visible (Default to normal):
	int nState = pApp->GetProfileInt (strSection, lpszState, 0);
	
	// Get the dimensions of the splitter panes:
	m_cDataView.SetColumnInfo (0, pApp->GetProfileInt (strSection, lpszGroups, nWidth / 3), 10);
	m_cEventView.SetRowInfo (0, pApp->GetProfileInt (strSection, lpszEvents, nHeight / 2), 10);

	// Get tool bar state and show if true:
	if (pApp->GetProfileInt (strSection, lpszToolbar, true))
		ShowControlBar (&m_cToolBar, TRUE, TRUE);

	// Load status bar state and show if true:
	if (pApp->GetProfileInt (strSection, lpszStatusbar, true))
		m_cStatusBar.ShowWindow (SW_SHOW);

	// Resize the main window using position, width and height jsut 
	// retrieved from registry.
	MoveWindow (x, y, nWidth, nHeight);

	// Minimize window if state retrieved from registry is -1,
	if (nState == -1)
		ShowWindow (SW_SHOWMINIMIZED);
	// or maximize if state is 1,
	else if (nState == 1)
		ShowWindow (SW_SHOWMAXIMIZED);
	// or show as normal otherwise.
	else
		ShowWindow (SW_SHOWNORMAL);
	}

// **************************************************************************
// SaveSettings ()
//
// Description:
//	Called when the application's window settings should be saved to the
//	registry.  These settings will be used the next time the application
//	starts up.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::SaveSettings ()
	{
	// Get pointer to application root object:
	CWinApp *pApp = AfxGetApp ();
	ASSERT (pApp != NULL);

	// Define "Main Window" registry section string:
	CString strSection = lpszMainWindow;
	CString strKey;

	// Save the size/position/state of the main window:
	if (IsIconic ())
		{
		// State is -1 if minimized.  Doesn't make sense to overwrite
		// any existing window size and position settings now.
		pApp->WriteProfileInt (strSection, lpszState, -1);
		}
	else if (IsZoomed ())
		{
		// State is 1 if maximized.  Doesn't make sense to overwrite
		// any existing window size and position settings now.
		pApp->WriteProfileInt (strSection, lpszState, 1);
		}
	else
		{
		// State is 0 otherwise.  Save window size and position settings
		// too.
		RECT rc;
		GetWindowRect (&rc);

		pApp->WriteProfileInt (strSection, lpszX, rc.left);
		pApp->WriteProfileInt (strSection, lpszY, rc.top);

		pApp->WriteProfileInt (strSection, lpszCX, rc.right - rc.left);
		pApp->WriteProfileInt (strSection, lpszCY, rc.bottom - rc.top);

		pApp->WriteProfileInt (strSection, lpszState, 0);
		}

	// Save the splitter pane widths:
	
	// Save group view pane width:
	int cxCol;
	int cxColMin;
	m_cDataView.GetColumnInfo (0, cxCol, cxColMin);
	pApp->WriteProfileInt (strSection, lpszGroups, cxCol);

	// Save event view pane width:
	int cyRow;
	int cyRowMin;
	m_cEventView.GetRowInfo (0, cyRow, cyRowMin);
	pApp->WriteProfileInt (strSection, lpszEvents, cyRow);

	// Save tool bar state (true shown, false hidden):
	pApp->WriteProfileInt (strSection, lpszToolbar, m_cToolBar.IsWindowVisible ());

	// Save status bar state (true shown, false hidden):
	pApp->WriteProfileInt (strSection, lpszStatusbar, m_cStatusBar.IsWindowVisible ());
	}

// **************************************************************************
// PreCreateWindow ()
//
// Description:
//	Called before the creation of the window. Override to modify the window 
//	style.
//
// Parameters:
//  none
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKMainWnd::PreCreateWindow (CREATESTRUCT &cs)
	{
	// Perform default processing.  Return FALSE if fail:
	if (!CFrameWnd::PreCreateWindow (cs))
		return (FALSE);

	// Modify main window style (do not show document name before application
	// name in title bar, and window not initially visible).
	cs.style &= ~(FWS_PREFIXTITLE | WS_VISIBLE);
	return (TRUE);
	}


/////////////////////////////////////////////////////////////////////////////
// CKMainWnd diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CKMainWnd::AssertValid () const
	{
	CFrameWnd::AssertValid ();
	}

void CKMainWnd::Dump (CDumpContext &dc) const
	{
	CFrameWnd::Dump (dc);
	}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CKMainWnd message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnCreateClient ()
//
// Description:
//	Create our splitter window with views.
//
// Parameters:
//  LPCREATESTRUCT	lpcs		The CREATESTRUCT structure defines the 
//								  initialization parameters passed to the 
//								  window procedure of an application.	
//	CCreateContext	*pContext	A CCreateContext structure contains pointers
//								  to the document, the frame window, the view, 
//								  and the document template.
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CKMainWnd::OnCreateClient (LPCREATESTRUCT lpcs, CCreateContext *pContext) 
	{
	// Set up tool bar:

	// First determine whether or not to use flat toolbars.  Proper support 
	// for flat toolbars begins with the COMCTL32.DLL installed with 
	// IE4 - V4.71.
	CVersionInfo cVer (_T("COMCTL32.DLL"));
	DWORD dwVersion = MAKELONG (cVer.GetMinorVersion (), cVer.GetMajorVersion ());
	TRACE (_T("Common controls version %u.%02u detected\n"), cVer.GetMajorVersion (), cVer.GetMinorVersion ()); 

	// If DLL version supports flat tool bars,
	if (dwVersion >= MAKELONG (71, 4))
		{
		// Create tool bar with flat style.  (Other style properties set are:
		// child of this window, show on top of frame window, message text is
		// to be updated at same time as tool tips, tool tips are displayed.)
		// Return FALSE if fail:
		if (!m_cToolBar.CreateEx (this, TBSTYLE_FLAT, WS_CHILD | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, CRect (0, 0, 0, 0), ID_VIEW_TOOLBAR))
			{
			TRACE (_T("OTC: CKMainWnd::OnCreateClient () -> Flat tool bar creation failed\n"));
			return (FALSE);
			}
		}
	
	// else it does not:
	else
		{
		// Create tool bar with default style.  (Other style properties set are:
		// child of this window, show on top of frame window, message text is
		// to be updated at same time as tool tips, tool tips are displayed.)
		// Return FALSE if fail:
		if (!m_cToolBar.Create (this, WS_CHILD | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, ID_VIEW_TOOLBAR))
			{
			TRACE (_T("OTC: CKMainWnd::OnCreateClient () -> Tool bar creation failed\n"));
			return (FALSE);
			}
		}

	// Load the tool bar:
	m_cToolBar.LoadToolBar (IDR_MAINFRAME);

	// Set up status bar:

	// Create with "this" window as parent:
	m_cStatusBar.Create (this);

	// Set indicator IDs:
	m_cStatusBar.SetIndicators (auIDs, sizeof(auIDs) / sizeof(auIDs[0]));

	// Set indicator ID, style, and width for each index:
	m_cStatusBar.SetPaneInfo (ID_HELPTEXT, auIDs[ID_HELPTEXT], SBPS_NOBORDERS | SBPS_STRETCH, 0);
	m_cStatusBar.SetPaneInfo (ID_ITEMCOUNT, auIDs[ID_ITEMCOUNT], SBPS_NORMAL, 64);

	// Split this window into to two panes, one on top of the other.  Top
	// pane will be "data view", and bottowm will be "event view".  Return
	// FALSE if fail.
	if (!m_cEventView.CreateStatic (this, 2, 1))
		{
		TRACE (_T("OTC: CKMainWnd::OnCreateClient () -> CreateStatic () failed\n"));
		return (FALSE);
		}

	// Split top (data view) pane again into two panes, side by side.  Left
	// pane will be "group view", right pane will be the "item view".  Return
	// FALSE if fail.
	if (!m_cDataView.CreateStatic (&m_cEventView, 1, 2))
		{
		TRACE (_T("OTC: CKMainWnd::OnCreateClient () -> CreateStatic () failed\n"));
		return (FALSE);
		}

	// Get client area of main window.  We will use it to set initial size
	// of the 3 component panes.  Top panes will each be half the width of
	// the client area, and 3/4 its heigth.  Bottom pane will be its full
	// width and 1/4 its height.
	RECT rc;
	GetClientRect (&rc);

	// Create "group view" in top left pane:
	if (!m_cDataView.CreateView (
		0, 0, 
		RUNTIME_CLASS (CKGroupView), 
		CSize (rc.right / 2, 3 * rc.bottom / 4),
		pContext))
		{
		ASSERT (FALSE);
		return (FALSE);
		}

	// Create "item view" in top right pane:
	if (!m_cDataView.CreateView (
		0, 1, 
		RUNTIME_CLASS (CKItemView), 
		CSize (rc.right / 2, 3 * rc.bottom / 4),
		pContext))
		{
		ASSERT (FALSE);
		return (FALSE);
		}

	// Create "event view" in bottom pane:
	if (!m_cEventView.CreateView (
		1, 0,
		RUNTIME_CLASS (CKEventView),
		CSize (rc.right, rc.bottom / 4),
		pContext))
		{
		ASSERT (FALSE);
		return (FALSE);
		}

	// Save event view pointer:
	m_pEventLog = (CKEventView *) m_cEventView.GetPane (1, 0);

	// Start update status bar timer:
	SetTimer (STATUSUPDATETIMER, STATUSUPDATEINTERVAL, NULL);

	// If we make it here, everything went OK.  Return TRUE to indicate
	// success:
	return (TRUE);
	}

// **************************************************************************
// ActivateFrame ()
//
// Description:
//	Handles notification that the window frame should be activated and 
//	updated.  Override this function to load current window settings.
//
// Parameters:
//  int			nCmdShow	Specifies how the CWnd is to be shown.
//							  SW_HIDE, SW_MINIMIZE, etc.	
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::ActivateFrame (int nCmdShow) 
	{
	// Make sure window settings are loaded from registry just once per
	// application load by using the m_bLoaded flag:
	if (!m_bLoaded)
		{
		LoadSettings ();	
		m_bLoaded = true;
		}

	// Perform default processing:
	CFrameWnd::ActivateFrame (nCmdShow);
	}

// **************************************************************************
// OnClose ()
//
// Description:
//	Handles notification that the window is being destroyed.  Override this
//	function to save current window settings.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::OnClose () 
	{
	// Get pointer to our document object:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc);

	// Only allow a close if the document is not locked (i.e. some other
	// thread is using document and needs it to stay alive until they are 
	// done:
	if (!pDoc->IsLocked ())
		{
		// Save main window settings in registry so it will appear the
		// same next time application is started:
		SaveSettings ();

		// Perform default processing:
		CFrameWnd::OnClose ();
		}
	else
		{
		// Don't allow close.  Sound a beep to warn user:
		MessageBeep ((DWORD)-1);
		return;
		}
	}

// **************************************************************************
// OnEndSession ()
//
// Description:
//	Handles notification that the session is ending.
//
// Parameters:
//  BOOL		bEnding		If TRUE, Windows can terminate any time after 
//							  all applications have returned from processing
//							  this call.  Override this function to save
//							  current window settings.
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::OnEndSession (BOOL bEnding) 
	{
	// Save main window settings to registry so it will appear the same the
	// next time application is started:
	SaveSettings ();

	// Perfrom default processing:
	CFrameWnd::OnEndSession (bEnding);
	}

// **************************************************************************
// OnCmdMsg ()
//
// Description:
//	Called by the framework to route and dispatch command messages and to 
//	handle the update of command user-interface objects.  Override to pass
//	message onto appropriate view to process.
//
// Parameters:
//  UINT				nID				Contains the command ID.
//	int					nCode			Identifies the command notification code.
//	void				*pExtra			Used according to the value of nCode.
//	AFX_CMDHANDLERINFO	*pHandlerInfo	If not NULL, OnCmdMsg fills in the 
//										  pTarget and pmf members of the 
//										  pHandlerInfo structure instead of 
//										  dispatching the command. Typically,
//										  this parameter should be NULL.
//
// Returns:
//  BOOL - TRUE if message processed.
// **************************************************************************
BOOL CKMainWnd::OnCmdMsg (UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo) 
	{
	// See if the default framework can process the message.  Return TRUE
	// if it can:
	if (CFrameWnd::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo))
		return (TRUE);

	// Otherwise pass it onto the document for routing to all inactive views.
	// If one of these views processes the message, return TRUE:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	if (pDoc != NULL)
		{
		if (pDoc->RouteCmdMsg (GetActiveView (), nID, nCode, pExtra, pHandlerInfo))
			return (TRUE);
		}
	
	// If we make it here, message not processed.  Return FALSE to indicate
	// this:
	return (FALSE);
	}

// **************************************************************************
// OnUpdateCmdUI ()
//
// Description:
//	Handles notification to update the enabled state of application 
//	functionality (i.e, menu items).
//
// Parameters:
//  CCmdUI		*pCmdUI		Encapsulates user interface data
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::OnUpdateCmdUI (CCmdUI *pCmdUI)
	{
	// Get pointer to our document:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc != NULL);

	// Set flag to indicate we sould disable all items related to project
	// edit functions if the docuemtn is presently locked (i.e. don't allow
	// other threads to access document data while another thread has
	// possession of it.)
	bool bDisableProjectEditOperations = pDoc->IsLocked ();

	// Processes according to the menu item we are being asked about:
	switch (pCmdUI->m_nID)
		{
		// Disable "File|New", "File|Open", and "File|Exit" if document
		// is locked:
		case ID_FILE_NEW:
		case ID_FILE_OPEN:
		case ID_APP_EXIT:
			pCmdUI->Enable (!pDoc->IsLocked ());
			break;

		// Always enable "Help|About":
		case ID_APP_ABOUT:
			pCmdUI->Enable (true);
			break;

		// Enable "File|Save" only if document is not locked, and is modified:
		case ID_FILE_SAVE:
			pCmdUI->Enable (!bDisableProjectEditOperations && ((CKDocument *) GetActiveDocument ())->IsModified ());
			break;

		// Enable "File|Save As" only if document is not locked and has
		// server objects defined:
		case ID_FILE_SAVE_AS:
			pCmdUI->Enable (!bDisableProjectEditOperations && ((CKDocument *) GetActiveDocument ())->GetServerCount () > 0);
			break;

		// Disable "File|Most Recently Used Files" selections if document
		// is locked:
		case ID_FILE_MRU_FILE1:
		case ID_FILE_MRU_FILE2:
		case ID_FILE_MRU_FILE3:
		case ID_FILE_MRU_FILE4:
		case ID_FILE_MRU_FILE5:
		case ID_FILE_MRU_FILE6:
		case ID_FILE_MRU_FILE7:
		case ID_FILE_MRU_FILE8:
		case ID_FILE_MRU_FILE9:
		case ID_FILE_MRU_FILE10:
		case ID_FILE_MRU_FILE11:
		case ID_FILE_MRU_FILE12:
		case ID_FILE_MRU_FILE13:
		case ID_FILE_MRU_FILE14:
		case ID_FILE_MRU_FILE15:
		case ID_FILE_MRU_FILE16:
			if (pCmdUI->m_pMenu)
				{
				int iMRU;
				CMenu *pMenu = pCmdUI->m_pMenu;

				TRACE (_T("Deleting MRU list\n"));

				// Pre-delete everything so that things don't get screwed up when
				// the user makes a change
				for (iMRU = ID_FILE_MRU_FILE1; iMRU <= ID_FILE_MRU_FILE16; iMRU++)
					pMenu->DeleteMenu (iMRU, MF_BYCOMMAND);

				cApp.OnUpdateMRUFile (pCmdUI);

				// If locked, set enabled state to "GRAYED and DISABLED":
				if (pDoc->IsLocked ())
					{
					for (iMRU = ID_FILE_MRU_FILE1; iMRU <= ID_FILE_MRU_FILE16; iMRU++)
						pMenu->EnableMenuItem (iMRU, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					}
				}

			break;

		// Enable "Edit|Copy" only if document is not locked, and appropriate
		// item is slected:
		case ID_EDIT_COPY:
			{
			// If document is not locked, look at what view is active and what
			// sort of view items are selected:
			if (!bDisableProjectEditOperations)
				{
				// Get pointer to the active view:
				CView *pView = GetActiveView ();

				// If the group view is active, enable if a server or group
				// is selected:
				if (pView == GetGroupView ())
					{
					pCmdUI->Enable ((pDoc->GetSelectedServer () != NULL) ||
									(pDoc->GetSelectedGroup () != NULL));
					}
				
				// Else if the item view is active, enable if one or more
				// items are selected:
				else if (pView == GetItemView ())
					{
					pCmdUI->Enable (((CKItemView *)pView)->GetSelectedCount ());
					}

				// Otherwise disable:
				else
					pCmdUI->Enable (false);
				}

			// Disable if document is locked:
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable "Edit|Cut" and "Edit|Delete" only if document is not locked
		// and a group or server is selected:
		case ID_EDIT_CUT:
		case ID_EDIT_DELETE:
			if (bDisableProjectEditOperations)
				pCmdUI->Enable (false);
			else
				pCmdUI->Enable ((pDoc->GetSelectedServer () != NULL) ||
								(pDoc->GetSelectedGroup () != NULL));
			break;

		// Disable "Edit|Paste" if document is locked.  If not, enable only if
		// it makes sence to paste the contents of the clipboard into the 
		// selected item:
		case ID_EDIT_PASTE:
			{
			if (bDisableProjectEditOperations)
				pCmdUI->Enable (false);
			else
				{
				// Get pointer to the active view:
				CView *pView = GetActiveView ();

				// Get pointer to the selected server and group:
				CKServer *pServer = pDoc->GetSelectedServer ();
				CKGroup *pGroup = pDoc->GetSelectedGroup ();

				// If group view is active:
				if (pView == GetGroupView ())
					{
					// Can paste server if no selected object:
					if (pServer == NULL && pGroup == NULL)
						pCmdUI->Enable (IsClipboardFormatAvailable (CF_SERVER));

					// Can paste group or server if a server is selected:
					else if (pServer != NULL)
						pCmdUI->Enable (IsClipboardFormatAvailable (CF_SERVER) ||
										IsClipboardFormatAvailable (CF_GROUP));

					// Can paste item if a group is selected:
					else if (pGroup != NULL)
						pCmdUI->Enable (IsClipboardFormatAvailable (CF_ITEM));
					}
				
				// Else if item view is active:
				else if (pView == GetItemView ())
					{
					// Can paste item if a group is selected:
					if (pGroup != NULL)
						pCmdUI->Enable (IsClipboardFormatAvailable (CF_ITEM));

					// Otherwise can't paste anything:
					else
						pCmdUI->Enable (false);
					}

				// Can't paste into any other view, so disable:
				else
					pCmdUI->Enable (false);
				}
			}
			break;

		// Enable "Edit|New Server Connection" if document is not locked:
		case ID_EDIT_NEWSERVER:
			pCmdUI->Enable (!bDisableProjectEditOperations);
			break;

		// Enable "Edit|New Group" if document is not locked and a server 
		// is selected:
		case ID_EDIT_NEWGROUP:
			pCmdUI->Enable (!bDisableProjectEditOperations && pDoc->GetSelectedServer () != NULL);
			break;

		// Enable "Edit|Properties" if document is not locked and a server or
		// or group is selected:
		case ID_EDIT_PROPERTIES:
			if (bDisableProjectEditOperations)
				pCmdUI->Enable (false);
			else
				pCmdUI->Enable ((pDoc->GetSelectedServer () != NULL) ||
								(pDoc->GetSelectedGroup () != NULL));
			break;

		// Enable "Edit|New Item" if document is not locked and a group
		// is selected:
		case ID_EDIT_NEWITEM:
			pCmdUI->Enable (!bDisableProjectEditOperations && pDoc->GetSelectedGroup () != NULL);
			break;

		// Enable "View|Clear Messages" if the event view has messages in it:
		case ID_VIEW_CLEAR:
			pCmdUI->Enable (GetEventView ()->GetEventCount ());
			break;

		// Always enable "View|Log Errors Only".  Check if event view's
		// "log errors only" flag has been set:
		case ID_VIEW_ERRORONLY:
			pCmdUI->Enable (true);
			pCmdUI->SetCheck (GetEventView ()->LogErrorsOnly ());
			break;

		// Enable "Tools|Server|Connect" if document is not locked, a
		// server is selected, and that server in not presently connected:
		case ID_TOOLS_CONNECT:
			{
			CKServer *pServer = pDoc->GetSelectedServer ();

			if (pServer && !bDisableProjectEditOperations)
				pCmdUI->Enable (!pServer->IsConnected ());
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable "Tools|Server|Disconnect", "Tools|Server|Reconnect", 
		// "Tools|Server|Get Error String", "Tools|Server|Get Group By Name",
		// and "Tools|Server|Enumerate Groups" if document is not locked,
		// a server is selected, and that server is presently connected:
		case ID_TOOLS_DISCONNECT:
		case ID_TOOLS_RECONNECT:
		case ID_TOOLS_GET_ERROR_STRING:
		case ID_TOOLS_GET_GROUP_BY_NAME:
		case ID_TOOLS_ENUMERATE_GROUPS:
			{
			CKServer *pServer = pDoc->GetSelectedServer ();

			if (pServer && !bDisableProjectEditOperations)
				pCmdUI->Enable (pServer->IsConnected ());
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable "Tools|Group|Clone Group" if document is not locked, a
		// group is selected, and that group supports the Group State
		// Management interface:
		case ID_TOOLS_CLONE_GROUP:
			{
			CKGroup *pGroup = pDoc->GetSelectedGroup ();
			
			if (pGroup && !bDisableProjectEditOperations)
				{
				pCmdUI->Enable (pGroup->IsIGroupStateMgtSupported ());
				}
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable "Tools|Group|Export CSV" and "Tools|Group|Import CSV" if
		// document is not locked, and a group is selected:
		case ID_TOOLS_EXPORT_CSV:
		case ID_TOOLS_IMPORT_CSV:
			pCmdUI->Enable (!bDisableProjectEditOperations && pDoc->GetSelectedGroup () != NULL);
			break;

		// Enable "Tools|Item|Set Active" and "Tools|Item|Set Inactive" if
		// docuement is not locked, a group is selected, if one or more
		// items in that group are currently selected, and the group
		// supports the Item Management interface:
		case ID_TOOLS_SET_ACTIVE:
		case ID_TOOLS_SET_INACTIVE:
			{
			CKGroup *pGroup = pDoc->GetSelectedGroup ();
			
			if (pGroup && !bDisableProjectEditOperations)
				{
				pCmdUI->Enable ((GetItemView ()->GetSelectedCount () > 0) && 
					(pGroup->IsIItemMgtSupported ()));
				}
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable item sync read and write if document is not locked, a
		// group is selected, if one or more items in that group are 
		// currently selected, and the group supports the sync IO
		// interface:
		case ID_TOOLS_SYNC_READ_CACHE:
		case ID_TOOLS_SYNC_READ_DEVICE:
		case ID_TOOLS_SYNC_WRITE:
			{
			CKGroup *pGroup = pDoc->GetSelectedGroup ();
			
			if (pGroup && !bDisableProjectEditOperations)
				{
				pCmdUI->Enable ((GetItemView ()->GetSelectedCount () > 0) && 
					(pGroup->IsISyncIOSupported ()));
				}
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable item async 1.0 read, write and refresh if document is not
		// locked, a group is selected, if one or more items in that group
		// are currently selected, the group supports the Async IO interface
		// and the OPC_20_DATACHANGE update method:
		case ID_TOOLS_ASYNC10_READ_CACHE:
		case ID_TOOLS_ASYNC10_READ_DEVICE:
		case ID_TOOLS_ASYNC10_REFRESH_CACHE:
		case ID_TOOLS_ASYNC10_REFRESH_DEVICE:
		case ID_TOOLS_ASYNC10_WRITE:
			{
			CKGroup *pGroup = pDoc->GetSelectedGroup ();
			
			if (pGroup && !bDisableProjectEditOperations)
				{
				pCmdUI->Enable ((GetItemView ()->GetSelectedCount () > 0) && 
								(pGroup->IsIAsyncIOSupported ()) &&
								(pGroup->GetUpdateMethod () != OPC_20_DATACHANGE));
				}
			else
				pCmdUI->Enable (false);
			}
			break;

		// Enable item async 2.0 read, write and refresh if document is not
		// locked, a group is selected, if one or more items in that group
		// are currently selected, the group supports the Async 2.0 IO interface
		// and the OPC_20_DATACHANGE update method
		case ID_TOOLS_ASYNC20_READ_DEVICE:
		case ID_TOOLS_ASYNC20_REFRESH_CACHE:
		case ID_TOOLS_ASYNC20_REFRESH_DEVICE:
		case ID_TOOLS_ASYNC20_WRITE:
			{
			CKGroup *pGroup = pDoc->GetSelectedGroup ();
			
			if (pGroup && !bDisableProjectEditOperations)
				{
				pCmdUI->Enable ((GetItemView ()->GetSelectedCount () > 0) && 
								(pGroup->IsIAsyncIO2Supported ()) &&
								(pGroup->GetUpdateMethod () == OPC_20_DATACHANGE));
				}
			else
				pCmdUI->Enable (false);
			}
			break;

		default:
			// Unexpected item ID - programmer error
			TRACE (_T("OTC: Unhandled OnUpdateCmdUI case (%08X)!\r\n"), pCmdUI->m_nID);
			break;
		}
	}

// **************************************************************************
// OnServerShutdownRequest ()
//
// Description:
//	Handles notification that a server has been shutdown.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Pointer to server object.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKMainWnd::OnServerShutdownRequest (WPARAM wParam, LPARAM lParam)
	{
	// Get pointer to our document:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc != NULL);

	// Notify the document of the shutdown request:
	pDoc->OnServerShutdown ((CKServer *) lParam);

	return (0);
	}

// **************************************************************************
// OnSelectGroup
//
// Description:
//	Handles notification that a group should be selected.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Pointer to group object.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKMainWnd::OnSelectGroup (WPARAM wParam, LPARAM lParam)
	{
	// Get pointer to our document:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc != NULL);

	// Notify document to update all views due to a new group selection.
	// Pointer to selected group object is passed along as lParam:
	pDoc->UpdateAllViews (NULL, HINT_SELECT_GROUP, (CObject *)lParam);
	
	return (0);
	}

// **************************************************************************
// OnRefreshItemView ()
//
// Description:
//	Handles notification that a item view should be repianted.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Not used.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKMainWnd::OnRefreshItemView (WPARAM wParam, LPARAM lParam)
	{
	// Get pointer to our document:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc != NULL);

	// Notify document to update item view:
	pDoc->UpdateAllViews (NULL, HINT_REFRESH_ITEMVIEW, NULL);
	
	return (0);
	}

// **************************************************************************
// OnItemReAdd ()
//
// Description:
//	Handles notification that a item should be re-added to the view.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Pointer to the item.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKMainWnd::OnItemReAdd (WPARAM wParam,	LPARAM lParam)
	{
	// Get pointer to our document:
	CKDocument *pDoc = (CKDocument *) GetActiveDocument ();
	ASSERT (pDoc != NULL);

	// Notify document to update all views due to a re-add item request.
	// Pointer to item object is passed along as lParam:
	pDoc->UpdateAllViews (NULL, HINT_READD_ITEM, (CObject *)lParam);
	
	// Set document modified flag:
	pDoc->SetModified ();

	return (0);
	}

// **************************************************************************
// OnChangeView ()
//
// Description:
//	Handles notification that the view should be changed (tab selection).
//
// Parameters:
//  WPARAM		wParam			Not used.
//	CView		*pActiveView	Pointer to the active view.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKMainWnd::OnChangeView (WPARAM wParam, LPARAM lParam)
	{
	// See what view pActiveView points to, and set that view active:
	CView *pActiveView = (CView *)lParam;
	// If group view,
	if (pActiveView == GetGroupView ())
		SetActiveView (GetItemView ());
	
	// else if item view,
	else if (pActiveView == GetItemView ())
		SetActiveView (GetEventView ());

	// else if event view (no other case expected),
	else if (pActiveView == GetEventView ())
		SetActiveView (GetGroupView ());

	return (0);
	}

// **************************************************************************
// LogMsg ()
//
// Description:
//	Called to place a message in the event log.
//
// Parameters:
//  EVENTTYPE	eType			Event type (enumerated in globals.h)
//	LPCTSTR		lpszMessage		Pointer to message string.
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::LogMsg (EVENTTYPE eType, LPCTSTR lpszMessage)
	{
	// Pass the message along to the event view:
	GetEventView ()->LogMsg (eType, lpszMessage);
	}

// **************************************************************************
// UpdateItemCount ()
//
// Description:
//	Called to update the item count member variable (m_cnItems).
//
// Parameters:
//  int			nDelta		Number to change the count by.
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::UpdateItemCount (int nDelta)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csStatus);

	// Update the item count:
	m_cnItems += nDelta;
	ASSERT (m_cnItems >= 0);
	}

// **************************************************************************
// SetStatusBarText ()
//
// Description:
//	Called to set the status bar text.
//
// Parameters:
//  LPCTSTR		lpszText	Pointer to text string.
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::SetStatusBarText (LPCTSTR lpszText)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csStatus);

	// Store the text:
	m_strStatusText = lpszText;

	// Set flag to force a status bar update (to be processed in OnTimer()):
	m_bForceStatusText = true;
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
void CKMainWnd::OnTimer (UINT nIDEvent) 
	{
	// Process our status update event:
	if (nIDEvent == STATUSUPDATETIMER)
		{
		// Create a CSafeLock to make this object thread safe.  Our critical
		// section gets locked here, and will automatically be unlocked when the
		// CSafeLock goes out of scope.
		CSafeLock cs (&m_csStatus);

		static TCHAR szBuff [128];
		static CString strItemCount;
		static int cnItems = -1;

		// Only update on item change:
		if (cnItems != m_cnItems)
			{
			cnItems = m_cnItems;

			// Initialize string once:
			if (strItemCount.IsEmpty ())
				strItemCount.LoadString (IDS_ITEMCOUNT);

			// Update item count string:
			wsprintf (szBuff, strItemCount, cnItems);

			// Calculate new size for the pane:
			CClientDC dc (NULL);
			
			HFONT hFont = (HFONT)m_cStatusBar.SendMessage (WM_GETFONT);
			HGDIOBJ hOldFont = NULL;
			
			if (hFont != NULL)
				hOldFont = dc.SelectObject (hFont);

			CSize sz = dc.GetTextExtent (szBuff, lstrlen (szBuff));
			
			if (hOldFont != NULL)
				dc.SelectObject (hOldFont);

			// Update the width:
			m_cStatusBar.SetPaneInfo (ID_ITEMCOUNT, auIDs[ID_ITEMCOUNT], SBPS_NORMAL, sz.cx);

			// Update pane text:
			m_cStatusBar.SetPaneText (ID_ITEMCOUNT, szBuff, true);
			}

		// Reset status bar text if force update flag is set or if
		// current text is not "Idle"
		if (m_bForceStatusText || m_strIdleStatusText != m_strStatusText)
			{
			m_cStatusBar.SetPaneText (ID_HELPTEXT, m_strStatusText, true);
			m_bForceStatusText = false;
			}
		}

	// Perform default processing for all other timer event types:
	CFrameWnd::OnTimer (nIDEvent);
	}

// **************************************************************************
// OnDestroy ()
//
// Description:
//	The framework calls this member function to inform the CWnd object that 
//	it is being destroyed. OnDestroy is called after the CWnd object is 
//	removed from the screen.  Use opportunity to kill timer.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKMainWnd::OnDestroy () 
	{
	KillTimer (STATUSUPDATETIMER);
	CFrameWnd::OnDestroy ();
	}
