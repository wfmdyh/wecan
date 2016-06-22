// **************************************************************************
// itemwritedlg.cpp
//
// Description:
//	Implements a dialog class for assigning values to write to selected OPC 
//	items.
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
#include "itemwritedlg.h"
#include "item.h"
#include "group.h"

#define DEFAULTCOLUMNWIDTH		150
#define UPDATE_CURRENT_VALUE	1


/////////////////////////////////////////////////////////////////////////////
// CKItemWriteDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKItemWriteDlg, CDialog)
	//{{AFX_MSG_MAP(CKItemWriteDlg)
	ON_NOTIFY (LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
	ON_WM_TIMER ()
	ON_WM_DESTROY ()
	ON_BN_CLICKED (IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
	ON_MESSAGE (UM_LISTEDIT_ITEMCHANGE, OnWriteValueChanged)
END_MESSAGE_MAP ()


// **************************************************************************
// CKItemWriteDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  CKGroup		*pGroup		Pointer to group object items belong to.
//	CObArray	&cItemList	Array of item object to display.
//	DWORD		cdwItems	Number of items in cItemList.
//	WORD		wType		Write type: WRITE_SYNC, WRITE_ASYNC10, WRITE_ASYNC20
//
// Returns:
//  none
// **************************************************************************
CKItemWriteDlg::CKItemWriteDlg (CKGroup *pGroup, CObArray &cItemList, DWORD cdwItems, WORD wType)
	: CDialog (CKItemWriteDlg::IDD, NULL)
	{
	// Initialize member variables:
	ASSERT (pGroup != NULL);
	m_pGroup = pGroup;

	ASSERT (cdwItems > 0);
	m_cdwItems = cdwItems;

	m_pItemList = &cItemList;

	ASSERT (wType >= WRITE_SYNC && wType <= WRITE_ASYNC20);
	m_wType = wType;

	//{{AFX_DATA_INIT(CKItemWriteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	}

// **************************************************************************
// DoDataExchange ()
//
// Description:
//	This method is called by the framework to exchange and validate dialog data.
//
// Parameters:
//  CDataExchange	*pDX	A pointer to a CDataExchange object.
//
// Returns:
//  void
// **************************************************************************
void CKItemWriteDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKItemWriteDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemWriteDlg message handlers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// OnInitDialog ()
//
// Description:
//	Called immediately before the dialog box is displayed.  Use opportunity
//	to initialize controls.
//
// Parameters:
//  none
//
// Returns:
//	BOOL - TRUE.
// **************************************************************************
BOOL CKItemWriteDlg::OnInitDialog () 
	{
	// Perform default processing:
	CDialog::OnInitDialog ();

	// Get a pointer to our list control:
	CListCtrl *pListCtrl = NULL;
	pListCtrl = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (pListCtrl != NULL);

	// Initialize the image list for the items:

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
	
	pListCtrl->SetImageList (&m_cImageList, LVSIL_SMALL);

	// Create list control columns:
	CString strColumnTitle;

	// Item ID column:
	strColumnTitle.LoadString (IDS_ITEMID);
	pListCtrl->InsertColumn (0, strColumnTitle, LVCFMT_LEFT, DEFAULTCOLUMNWIDTH);

	// Current value column:
	strColumnTitle.LoadString (IDS_CURRENTVALUE);
	pListCtrl->InsertColumn (1, strColumnTitle, LVCFMT_LEFT, DEFAULTCOLUMNWIDTH);

	// Write value column:
	strColumnTitle.LoadString (IDS_WRITEVALUE);
	pListCtrl->InsertColumn (2, strColumnTitle, LVCFMT_LEFT, DEFAULTCOLUMNWIDTH);

	// Declare some varibles we will use to add items to list control:
	CKItem *pItem = NULL;
	LV_ITEM lvi;

	// Insert items into the list control:
	for (DWORD dwIndex = 0; dwIndex < m_cdwItems; dwIndex++)
		{
		// Get pointer to next item in list:
		pItem = (CKItem *) m_pItemList->GetAt (dwIndex);
		ASSERT (pItem != NULL);

		// Define mask for list control properties we are defining (text, image,
		// and paramter):
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		
		// Fill the LV_ITEM structure:
		lvi.iSubItem = 0;
		lvi.iImage = I_IMAGECALLBACK;		// Use callback to get image data
		lvi.pszText = LPSTR_TEXTCALLBACK;	// Use callback to get text data
		lvi.iItem = dwIndex;				// Item index
		lvi.lParam = (LPARAM) pItem;		// Parameter used to store pointer to assocaiated CKItem object
		
		// Instert the item into the list control:
		pListCtrl->InsertItem (&lvi);
		}

	// Subclass the list control with our list edit control (This will allow
	// use to edit write values as if using a light weight grid control):
	m_cListEditCtrl.SubclassDlgItem (IDC_LIST, this);

	// Set a timer to update current item values:
	SetTimer (UPDATE_CURRENT_VALUE, 100, NULL);

	// Disable the apply button to start.  Will enable when changes are made:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (FALSE);

	// Set window title based on write type.  Load title string from string 
	// resources:
	CString strTitle;

	switch (m_wType)
		{
		case WRITE_SYNC:
			strTitle.LoadString (IDS_SYNCWRITE);
			break;

		case WRITE_ASYNC10:
			strTitle.LoadString (IDS_ASYNC10WRITE);
			break;

		case WRITE_ASYNC20:
			strTitle.LoadString (IDS_ASYNC20WRITE);
			break;
		}

	SetWindowText (strTitle);

	// Place the cursor on the first item's write value cell:

	// First get rectangle bounding top item in list control:
	CRect rc;
	pListCtrl->GetItemRect (pListCtrl->GetTopIndex (), &rc, LVIR_BOUNDS);

	// Adjust right bound to left side of write value column plus an offset of 10:
	rc.right = pListCtrl->GetColumnWidth (0) + pListCtrl->GetColumnWidth (1) + 10;

	// Adjust bottom bound upward by an offset of 10:
	rc.bottom -= 10;

	// Post a message to simulate a left button down event at coordinates
	// just computed:
	pListCtrl->PostMessage (WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM (rc.right, rc.bottom));

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
	}

// **************************************************************************
// OnDestroy ()
//
// Description:
//	Called by framework just before window is destroyed.  Use opportunity to 
//	stop update timer.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemWriteDlg::OnDestroy () 
	{
	// Stop our update value timer:
	KillTimer (UPDATE_CURRENT_VALUE);

	// Perform default processing:
	CDialog::OnDestroy ();
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
void CKItemWriteDlg::OnGetDispInfo (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Get list view item description structure from argument:
	LV_DISPINFO *plvdi = (LV_DISPINFO *)pNMHDR;
	LV_ITEM *plvItem = &plvdi->item;

	// Use structure to get pointer to CKItem object associated with the 
	// list view item we are being asked about:
	CKItem *pItem = (CKItem *)plvItem->lParam;
	ASSERT (pItem != NULL);

	// We are being ased to supply text for list view item:
	if (plvItem->mask & LVIF_TEXT)
		{
		// Determine what column data is needed for, and fill request
		// accordingly:
		switch (plvItem->iSubItem)
			{
			case 0:	// Item ID
				lstrcpyn (plvItem->pszText, pItem->GetItemID (), plvItem->cchTextMax);
				break;

			case 1:	// Current Value
				{
				CString strValue;
				pItem->GetValue (strValue);

				lstrcpyn (plvItem->pszText, strValue, plvItem->cchTextMax);
				}
				break;

			case 2:	// Write Value (notification handled by subclass):
				if (plvdi->hdr.code == LVN_ENDLABELEDIT)
					{
					TRACE (_T("WriteValue\r\n"));
					}
				break;
			}
		}

	// We are being asked to supply item image.  Specify index into image
	// list according to item's valid and active state:
	if (plvItem->mask & LVIF_IMAGE)
		{
		if (pItem->IsValid ())
			plvItem->iImage = pItem->IsActive () ? 0 : 1;
		else
			plvItem->iImage = 2;
		}
	
	*pResult = 0;
	}

// **************************************************************************
// OnOK ()
//
// Description:
//	OK button event handler.  Use opportunity to apply changes.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemWriteDlg::OnOK () 
	{
	// Apply changes:
	OnApply ();

	// Perform default processing:
	CDialog::OnOK ();
	}

// **************************************************************************
// OnApply ()
//
// Description:
//	Apply changes.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemWriteDlg::OnApply () 
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Get pointer to our list control:
	CListCtrl *pListCtrl = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (pListCtrl != NULL);

	// Build and issue write request:
	try
		{
		// Decalare object arrays to contain list of items to write to and
		// the values to be written, and related variables:
		CKItem *pItem;
		CObArray cItems;
		CStringArray cValues;
		DWORD cdwItems = 0;

		ASSERT (m_cdwItems > 0);

		// Construct an item and value list for this write operation:
		cItems.SetSize (m_cdwItems);
		cValues.SetSize (m_cdwItems);

		// Cycle through the list to see which item values have been set:
		for (DWORD dwItem = 0; dwItem < m_cdwItems; dwItem++)
			{
			// Get pointer to next item in list:
			pItem = (CKItem *) m_pItemList->GetAt (dwItem);
			ASSERT (pItem != NULL);

			// We use the WParam to flag write value applied:
			if (pItem->GetWParam ())
				{
				// Write value entered, so add item and value to list of
				// items to request writes for:
				cItems [cdwItems] = pItem;

				// Add corresponding write value to value array:
				cValues [cdwItems] = pListCtrl->GetItemText (dwItem, 2);

				// Clear write applied flag:
				pItem->SetWParam (0);

				// Increment the number of items to write to:
				++cdwItems;
				}
			}

		// Send the write request (as long as there is at least one write value):
		if (cdwItems > 0)
			{
			switch (m_wType)
				{
				case WRITE_SYNC:
					m_pGroup->WriteSync (cItems, cValues, cdwItems);
					break;

				case WRITE_ASYNC10:
					m_pGroup->WriteAsync10 (cItems, cValues, cdwItems);
					break;

				case WRITE_ASYNC20:
					m_pGroup->WriteAsync20 (cItems, cValues, cdwItems);
					break;
				}
			}
		}
	
	catch (...)
		{
		ASSERT (FALSE);
		TRACE (_T("OTC: CKItemWriteDlg::OnApply () - memory exception thrown\r\n"));
		}

	// Disable the apply button until next write value change:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (FALSE);
	}

// **************************************************************************
// OnWriteValueChanged ()
//
// Description:
//	Write value changed event handler.
//
// Parameters:
//  WPARAM		wSubItem		Not used
//	LPARAM		lItem			Index of item.
//
// Returns:
//  long - 0.
// **************************************************************************
long CKItemWriteDlg::OnWriteValueChanged (WPARAM wSubItem, LPARAM lItem)
	{
	// Use the CKItem param to show that a write value has been applied.
	// We will use this in OnApply to determine whether to send write 
	// data for that value.

	// Get pointer to our list control:
	CListCtrl *pList = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (pList != NULL);

	// Get pointer to CKItem object associated with specified list
	// control item (stored as item data value):
	CKItem *pItem = (CKItem *)pList->GetItemData ((DWORD)lItem);
	ASSERT (pItem != NULL);

	// Set item's write value applied flag:
	pItem->SetWParam (1);

	// Since we now have pending writes, enable the apply button:
	((CButton *) GetDlgItem (IDC_APPLY))->EnableWindow (TRUE);

	return (0);
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
void CKItemWriteDlg::OnTimer (UINT nIDEvent) 
	{
	// Perform task according to itmer event type:
	switch (nIDEvent)
		{
		// This is our view update event type:
		case UPDATE_CURRENT_VALUE:
			{
			// Invalidate current values column to force a repaint of the current
			// values.

			// Get pointer to our list control:
			CListCtrl *pList = (CListCtrl *) GetDlgItem (IDC_LIST);
			ASSERT (pList != NULL);
			ASSERT (pList->GetItemCount () > 0);

			int nTopIndex;
			CRect rc;
			CRect rcitem;
				
			// Get an index to the first visible item:
			nTopIndex = pList->GetTopIndex ();

			// Get top-most boundary:
			pList->GetItemRect (nTopIndex, &rcitem, LVIR_BOUNDS);

			// Get bottom-most boundary:
			pList->GetClientRect (&rc);

			rc.left = pList->GetColumnWidth (0);				// left-most
			rc.right = rc.left + pList->GetColumnWidth (1);		// right-most
			rc.top = rcitem.top;
						
			// Invalidate the region:
			pList->InvalidateRect (&rc, FALSE);					// invalidate this rectange
			}
			break;

		default:
			// Perform default processing for all other event types:
			CDialog::OnTimer (nIDEvent);
			break;
		}
	}


