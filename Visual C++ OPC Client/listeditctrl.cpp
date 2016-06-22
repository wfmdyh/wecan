// **************************************************************************
// listeditctrl.cpp
//
// Description:
//	Implements a special combination list/edit control class used by the 
//	CKItemWriteDlg class.
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
#include "listeditctrl.h"

#define IDC_LISTEDIT	1


/////////////////////////////////////////////////////////////////////////////
// CKListEditCtrl
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKListEditCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CKListEditCtrl)
	ON_NOTIFY_REFLECT (LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_WM_LBUTTONDOWN ()
	//}}AFX_MSG_MAP
	ON_MESSAGE (UM_ONUPARROW, OnPrevSubLabel)
	ON_MESSAGE (UM_ONDNARROW, OnNextSubLabel)
END_MESSAGE_MAP ()


// **************************************************************************
// CKListEditCtrl ()
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
CKListEditCtrl::CKListEditCtrl ()
	{
	// Initialize member variables:
	m_nSelItem = -1;
	}

// **************************************************************************
// ~CKListEditCtrl ()
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
CKListEditCtrl::~CKListEditCtrl ()
	{
	}

// **************************************************************************
// HitTestEx ()
//
// Description:
//	Determine which item or subitem was hit.
//
// Parameters:
//  CPoint		&cPoint		Current locaction of mouse cursor
//	int			*pCol		Gets set to column hit was in.
//
// Returns:
//  int - Row hit was in, or -1 if hit was not on a cell.
// **************************************************************************
int CKListEditCtrl::HitTestEx (CPoint &cPoint, int *pCol)
	{	
	int nCol = 0;
	int nRow = HitTest (cPoint, NULL);		
	int nBottom = 0;
	int cnColumns = 0;
	
	// Initialize returned column number to zero:
	if (pCol) 
		*pCol = 0;

	// Make sure that the ListView is in LVS_REPORT format:
	if ((GetWindowLong (m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT)
		return (nRow);	
	
	// Get the top and bottom visible rows:
	nRow = GetTopIndex ();
	nBottom = nRow + GetCountPerPage ();	
	
	// Make sure bottom index is valid:
	if (nBottom > GetItemCount ())
		nBottom = GetItemCount ();		
	
	// Get the number of columns:
	CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem (0);
	cnColumns = pHeader->GetItemCount ();	

	// Determine which row the hit occurred:
	for (; nRow <= nBottom; nRow++)	
		{
		// Get bounding rect of item:
		CRect rect;
		GetItemRect (nRow, &rect, LVIR_BOUNDS);		
		
		// Check to see if the point falls in bounds of that row:
		if (rect.PtInRect (cPoint))		
			{
			// Hit was in this row.  Now find the column:
			for (nCol = 0; nCol < cnColumns; nCol++)
				{				
				int nColWidth = GetColumnWidth (nCol);				
				
				// If hit is within this column, return:
				if (cPoint.x >= rect.left && cPoint.x <= (rect.left + nColWidth))				
					{					
					if (pCol) 
						*pCol = nCol;

					return (nRow);				
					}				
				
				// Adjust left side of row's rectangle so we will look at 
				// next column ntext time around loop:
				rect.left += nColWidth;			
				}		
			}	
		}

	// If we make it here, then the hit was not on a cell.  Return -1
	// to indicate failure:
	return (-1);
	}

// **************************************************************************
// EditSubLabel ()
//
// Description:
//	Edit the subitem value.  Creates a CEdit object sized and placed to cover
//	list control cell.  This edit box is needed to receive user input, and
//	will delete itself when its parent window is destroyed.
//
// Parameters:
//  int			nItem	Index of item
//	int			nCol	Column of item
//
// Returns:
//  CEdit* - Pointer to a CEdit object to be used to edit cell.
// **************************************************************************
CEdit* CKListEditCtrl::EditSubLabel (int nItem, int nCol)
	{
	int cnColumns = 0;
	int nColOffset = 0;

	// Make sure that the item is visible:
	if (!EnsureVisible (nItem, TRUE)) 
		return (NULL);

	// Get the number of columns:
	CHeaderCtrl* pHeader = (CHeaderCtrl*) GetDlgItem (0);
	cnColumns = pHeader->GetItemCount ();

	// Make sure that column is valid:
	if (nCol >= cnColumns || GetColumnWidth (nCol) < 5)
		return (NULL);

	// Get the column offset (from left boundary):
	nColOffset = 0;
	for (int i = 0; i < nCol; i++)
		nColOffset += GetColumnWidth (i);

	// Get selected item's rectangle:
	CRect rect;
	GetItemRect (nItem, &rect, LVIR_BOUNDS);

	// Scroll if we need to expose the column:
	CRect rcClient;
	GetClientRect (&rcClient);
	
	if (nColOffset + rect.left < 0 || nColOffset + rect.left > rcClient.right)
		{
		CSize size;
		size.cx = nColOffset + rect.left;
		size.cy = 0;
		Scroll (size);
		rect.left -= size.cx;
		}

	// Get column alignment:
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn (nCol, &lvcol);
	DWORD dwStyle;

	switch (lvcol.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_LEFT:
			dwStyle = ES_LEFT;
			break;

		case LVCFMT_RIGHT:
			dwStyle = ES_RIGHT;
			break;

		default:
			dwStyle = ES_CENTER;
			break;
		}

	rect.left += (nColOffset + 4);
	rect.right = rect.left + GetColumnWidth (nCol) - 10;

	if (rect.right > rcClient.right) 
		rect.right = rcClient.right;

	// Defile edit control (base class) style:
	dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	// Create a new CKListEdit box (not to be confused with CKListEditCtrl):
	CEdit *pEdit = new CKListEdit (nItem, nCol, GetItemText (nItem, nCol));

	// Set style, size and position using base class:
	pEdit->Create (dwStyle, rect, this, IDC_LISTEDIT);

	// Update currently selected item:
	m_nSelItem = nItem;

	// Return pointer to new CKListEdit control:
	return (pEdit);
	}


/////////////////////////////////////////////////////////////////////////////
// CKListEditCtrl message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnEndLabelEdit ()
//
// Description:
//	Handles notification message that notifies a list view control parent
//	window about the end of label editing for an item.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKListEditCtrl::OnEndLabelEdit (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	LV_DISPINFO *plvDispInfo = (LV_DISPINFO *)pNMHDR;
	LV_ITEM	*plvItem = &plvDispInfo->item;	
	
	// We are being asked for item's text:
	if (plvItem->pszText != NULL)	
		SetItemText (plvItem->iItem, plvItem->iSubItem, plvItem->pszText);	
	}

// **************************************************************************
// OnLButtonDown ()
//
// Description:
//	Left mouse button down event handler.
//
// Parameters:
//  UINT		nFlags		Flags.
//	CPoint		cPoint		Current location of mouse cursor.
//
// Returns:
//  void
// **************************************************************************
void CKListEditCtrl::OnLButtonDown (UINT nFlags, CPoint cPoint) 
	{
	int nIndex;	
	int nCol;

	// Perform default processing:
	CListCtrl::OnLButtonDown (nFlags, cPoint);	
	
	// Determine which item (i.e, row was hit):
	if ((nIndex = HitTestEx (cPoint, &nCol)) != -1)	
		{		
		// If the item column clicked on is editable, display an edit control:
		if (nCol > 1)		// Only last column is editable
			EditSubLabel (nIndex, nCol);		
		
		// otherwise select the item:
		else
			SetItemState (nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); 	
		}	
	}

// **************************************************************************
// OnPrevSubLabel ()
//
// Description:
//	On Up Arrow message handler.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Not used.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKListEditCtrl::OnPrevSubLabel (WPARAM wParam, LPARAM lParam)
	{
	// Previous item:
	--m_nSelItem;
	
	// Wrap to end if previous is invalid:
	if (m_nSelItem < 0)
		m_nSelItem = GetItemCount () - 1;

	// Edit new item:
	EditSubLabel (m_nSelItem, 2);
	
	return (0);
	}

// **************************************************************************
// OnNextSubLabel ()
//
// Description:
//	On Down Arrow message handler.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Not used.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKListEditCtrl::OnNextSubLabel (WPARAM wParam, LPARAM lParam)
	{
	// Next item:
	++m_nSelItem;
	
	// Wrap to start if next is invalid:
	if (m_nSelItem >= GetItemCount ())	
		m_nSelItem = 0;

	// Edit new item:
	EditSubLabel (m_nSelItem, 2);
	
	return (0);
	}


/////////////////////////////////////////////////////////////////////////////
// CKListEdit
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKListEdit, CEdit)
	//{{AFX_MSG_MAP(CKListEdit)
	ON_WM_CREATE ()
	ON_WM_CHAR ()
	ON_WM_NCDESTROY ()
	ON_WM_KILLFOCUS ()
	ON_MESSAGE (WM_PASTE, OnPaste)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKListEdit ()
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
CKListEdit::CKListEdit (int iItem, int iSubItem, CString sInitText)
	: m_sInitText (sInitText)
	{
	// Initialize member variables:
	m_iItem = iItem;	
	m_iSubItem = iSubItem;	
	m_bESC = FALSE;
	}

// **************************************************************************
// ~CKListEdit ()
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
CKListEdit::~CKListEdit ()
	{
	}


/////////////////////////////////////////////////////////////////////////////
// CKListEdit message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnCreate ()
//
// Description:
//	The framework calls this member function when an application requests 
//	that the Windows window be created.
//
// Parameters:
//  LPCREATESTRUCT	lpCreateStruct	Contains information about window to be
//									  created.
//
// Returns:
//  int - 0 if success, -1 if fail
// **************************************************************************
int CKListEdit::OnCreate (LPCREATESTRUCT lpCreateStruct) 
	{
	// Perform default processing:
	if (CEdit::OnCreate (lpCreateStruct) == -1)		
		return (-1);	
	
	// Set the font to our parents current:
	CFont* font = GetParent ()->GetFont ();	
	SetFont (font);

	// Initialize edit text:
	SetWindowText (m_sInitText);	

	// Give the control the focus and select the current text for edit:
	SetFocus ();	
	SetSel (0, -1);	

	// If we make it here, then return 0 to indicate success:
	return (0);
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
void CKListEdit::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
	// Filter escape and return:
	if (nChar == VK_ESCAPE || nChar == VK_RETURN)	
		{		
		if (nChar == VK_ESCAPE)
			m_bESC = TRUE;		
		
		GetParent ()->SetFocus ();		
		return;	
		}

	// Perform default processing:
	CEdit::OnChar (nChar, nRepCnt, nFlags);	

	// Get text extent:
	CString strText;
	GetWindowText (strText);	

	CWindowDC dc (this);
	CFont *pFont = GetParent ()->GetFont ();
	CFont *pFontDC = dc.SelectObject (pFont);
	CSize size = dc.GetTextExtent (strText);	
	dc.SelectObject (pFontDC);

	// add some extra buffer	
	size.cx += 5;
	
	// Get client rectangle:
	CRect rect; 
	CRect parentrect;	

	GetClientRect (&rect);
	GetParent ()->GetClientRect (&parentrect);

	// Transform rectangle to parent coordinates:
	ClientToScreen (&rect);
	GetParent ()->ScreenToClient (&rect);

	// Check whether control needs to be resized and whether there is space to grow:	
	if (size.cx > rect.Width ())	
		{
		if (size.cx + rect.left < parentrect.right)
			rect.right = rect.left + size.cx;		
		else			
			rect.right = parentrect.right;

		MoveWindow (&rect);	
		}

	// Send notification to parent of the list control that edit was made:
	GetParent ()->GetParent ()->SendMessage (UM_LISTEDIT_ITEMCHANGE, m_iSubItem, m_iItem);
	}

// **************************************************************************
// OnNcDestroy ()
//
// Description:
//	Called by the framework when the nonclient area is being destroyed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKListEdit::OnNcDestroy () 
	{
	// Perform default processing:
	CEdit::OnNcDestroy ();
	
	// Self-delete:
	delete this;	
	}

// **************************************************************************
// OnKillFocus ()
//
// Description:
//	This method is called by the framework immediately before losing the 
//	input focus.
//
// Parameters:
//  CWnd		*pNewWnd	Specifies a pointer to the window that receives
//							  the input focus, this may be NULL or it may be temporary.
//
// Returns:
//  void
// **************************************************************************
void CKListEdit::OnKillFocus (CWnd *pNewWnd) 
	{
	// Perform default processing:
	CEdit::OnKillFocus (pNewWnd);	
	
	// Get current text:
	CString strText;	
	GetWindowText (strText);

	// Send LVN_ENDLABELEDIT notification to parent of list control:
	LV_DISPINFO dispinfo;

	dispinfo.hdr.hwndFrom = GetParent ()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID ();	
	dispinfo.hdr.code = LVN_ENDLABELEDIT;
	dispinfo.item.mask = LVIF_TEXT;	
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR (LPCTSTR (strText));
	dispinfo.item.cchTextMax = strText.GetLength ();

	GetParent ()->GetParent ()->SendMessage (WM_NOTIFY, GetParent ()->GetDlgCtrlID (), (LPARAM) &dispinfo);	

	// Destroy our window:
	DestroyWindow ();
	}

// **************************************************************************
// PreTranslateMessage ()
//
// Description:
//	Used by class CWinApp to translate window messages before they are 
//	dispatched to theTranslateMessage andDispatchMessage Windows functions.
//	Use opportunity to process up and down arrow messages.
//
// Parameters:
//  MSG			*pMsg
//
// Returns:
//	BOOL - Nonzero if the message was translated and should not be dispatched,
//	0 if the message was not translated and should be dispatched.
// **************************************************************************
BOOL CKListEdit::PreTranslateMessage (MSG *pMsg) 
	{
	// Process key down events:
	if (pMsg->message == WM_KEYDOWN)	
		{
		// Send parent window notification of up arrow message:
		if (pMsg->wParam == VK_UP)
			{
			GetParent ()->SendMessage (UM_ONUPARROW, 0, 0);
			return (TRUE);
			}

		// Send parent window notification of down arrow event:
		if (pMsg->wParam == VK_DOWN)
			{
			GetParent ()->SendMessage (UM_ONDNARROW, 0, 0);
			return (TRUE);
			}

		// Do not dispatch return, delete, escape, and control presses:
		if (pMsg->wParam == VK_RETURN || 
			pMsg->wParam == VK_DELETE || 
			pMsg->wParam == VK_ESCAPE || 
			GetKeyState (VK_CONTROL))		
			{			
			::TranslateMessage (pMsg);
			::DispatchMessage (pMsg);			

			return (TRUE);		    	
			}	
		}

	// Perform default processing and return result:
	return (CEdit::PreTranslateMessage (pMsg));
	}

// **************************************************************************
// OnPaste ()
//
// Description:
//	Handles pastes into the edit control by allowing default processing
//  to occur and then notifies the parent that a change has been made.
//
// Parameters:
//  WPARAM		wParam		Not used.
//	LPARAM		lParam		Not used.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKListEdit::OnPaste (WPARAM wParam, LPARAM lParam) 
	{
	// Perform default message processing:
	Default ();

	// Send notification to parent of the list control that edit was made:
	GetParent ()->GetParent ()->SendMessage (UM_LISTEDIT_ITEMCHANGE, m_iSubItem, m_iItem);
	return (0);
	}
