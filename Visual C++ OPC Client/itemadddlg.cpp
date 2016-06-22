// **************************************************************************
// itemadddlg.cpp
//
// Description:
//	Implements a dialog class for adding OPC items.  Allows user to browse for
//	items and other handy things.
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
#include "itemadddlg.h"
#include "item.h"
#include "group.h"
#include "server.h"

// Registry sections;
static LPCTSTR lpszRegSection	= _T("Add Items");
static LPCTSTR lpszAutoValidate	= _T("Auto Validate");
static LPCTSTR lpszBrowseFlat	= _T("Browse Flat");

// Default add item list size:
#define DEF_ITEM_LIST_SIZE		16

// Branch dummy item data:
#define NULL_ITEM_NAME			_T("_QCNULL_DUMMY")
#define NULL_ITEM_DATA			0xFFFFFFFF

// Image list indices:
#define ILI_BRANCH				8
#define ILI_SELBRANCH			9
#define ILI_LEAF				3

// Access rights filter combo box indices:
#define CB_ACCESS_ANY			0
#define CB_ACCESS_READONLY		1
#define CB_ACCESS_WRITEONLY		2
#define CB_ACCESS_READWRITE		4


/////////////////////////////////////////////////////////////////////////////
// CKItemAddDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKItemAddDlg, CDialog)
	//{{AFX_MSG_MAP(CKItemAddDlg)
	ON_BN_CLICKED (IDC_NEW, OnNew)
	ON_NOTIFY (TVN_ITEMEXPANDING, IDC_BRANCHLIST, OnBranchExpanding)
	ON_NOTIFY (TVN_SELCHANGED, IDC_BRANCHLIST, OnBranchSelected)
	ON_CBN_SELCHANGE (IDC_FILTERACCESS, OnAccessFilterChange)
	ON_EN_CHANGE (IDC_FILTERLEAF, OnLeafFilterChange)
	ON_CBN_SELCHANGE (IDC_FILTERTYPE, OnVartypeFilterChange)
	ON_BN_CLICKED (IDC_NEXT, OnNext)
	ON_BN_CLICKED (IDC_PREVIOUS, OnPrevious)
	ON_BN_CLICKED (IDC_VALIDATEITEM, OnValidateItem)
	ON_EN_CHANGE (IDC_FILTERBRANCH, OnBranchFilterChange)
	ON_NOTIFY (NM_DBLCLK, IDC_LEAFLIST, OnClickLeafList)
	ON_EN_CHANGE (IDC_ITEMID, OnChange)
	ON_BN_CLICKED (IDC_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_DUPLICATE, OnDuplicate)
	ON_BN_CLICKED (IDC_ADD_LEAVES, OnAddLeaves)
	ON_EN_CHANGE (IDC_ACCESSPATH, OnChange)
	ON_BN_CLICKED (IDC_ACTIVE, OnChange)
	ON_CBN_SELCHANGE (IDC_DATATYPE, OnChange)
	ON_BN_CLICKED (IDC_AUTOVALIDATE, OnAutoValidate)
	ON_WM_SHOWWINDOW ()
	ON_BN_CLICKED (IDC_BROWSEFLAT, OnBrowseFlat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKItemAddDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  CKGroup							*pGroup		Group to add item to.
//	IOPCBrowseServerAddressSpace	*pIBrowse	Server browse interface.
//	CWnd*							pParent		Parent window.  If it is NULL,
//												the dialog object’s parent 
//												window is set to the main
//												application window.
//
// Returns:
//  none
// **************************************************************************
CKItemAddDlg::CKItemAddDlg (CKGroup *pGroup, IOPCBrowseServerAddressSpace *pIBrowse, CWnd *pParent /*=NULL*/)
	: CDialog (CKItemAddDlg::IDD, pParent)
	{
	// Initialize member variables:
	ASSERT (pGroup != NULL);
	m_pGroup = pGroup;
	m_pIBrowse = pIBrowse;

	m_nListIndex = 0;
	m_nSelIndex = 0;
	m_bModified = false;
	m_pIItemMgt = NULL;

	// Load registry settings.  These are settings we may want to use each time
	// we add some items:
	m_bAutoValidate = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszAutoValidate, FALSE);
	m_bBrowseFlat = AfxGetApp ()->GetProfileInt (lpszRegSection, lpszBrowseFlat, FALSE);

	// Allocate memory for add item list:
	try
		{
		m_cItemList.SetSize (DEF_ITEM_LIST_SIZE, DEF_ITEM_LIST_SIZE);
		}
	catch (...)
		{
		ASSERT (FALSE);
		}

	//{{AFX_DATA_INIT(CKItemAddDlg)
	m_strAccessPath = ITEM_DEFAULT_ACCESSPATH;
	m_bActive = ITEM_DEFAULT_ACTIVESTATE;
	m_strItemID = ITEM_DEFAULT_ITEMID;
	m_vtDataType = ITEM_DEFAULT_DATATYPE;
	m_strFilterLeaf = _T("");
	m_strFilterBranch = _T("");
	//}}AFX_DATA_INIT

	// Initialize bitmap buttons:
	m_cNext.Initialize (IDB_NEXT, IDB_NEXTGRAY);
	m_cPrev.Initialize (IDB_PREVIOUS, IDB_PREVIOUSGRAY);
	m_cNew.Initialize (IDB_NEWITEM, IDB_NEWITEMGRAY);
	m_cDuplicate.Initialize (IDB_DUPITEM, IDB_DUPITEMGRAY);
	m_cDelete.Initialize (IDB_DELETEITEM, IDB_DELETEITEMGRAY);
	m_cValidate.Initialize (IDB_VALIDATEITEM, IDB_VALIDATEITEMGRAY);
	}

// **************************************************************************
// ~CKItemAddDlg ()
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
CKItemAddDlg::~CKItemAddDlg ()
	{
	// Save registry settings:
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszAutoValidate, m_bAutoValidate);
	AfxGetApp ()->WriteProfileInt (lpszRegSection, lpszBrowseFlat, m_bBrowseFlat);
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
void CKItemAddDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKItemAddDlg)
	DDX_Text (pDX, IDC_ACCESSPATH, m_strAccessPath);
	DDX_Check (pDX, IDC_ACTIVE, m_bActive);
	DDX_Text (pDX, IDC_ITEMID, m_strItemID);
	DDX_Text (pDX, IDC_FILTERLEAF, m_strFilterLeaf);
	DDX_Text (pDX, IDC_FILTERBRANCH, m_strFilterBranch);
	//}}AFX_DATA_MAP

	DDX_Check (pDX, IDC_BROWSEFLAT, m_bBrowseFlat);
	DDX_Check (pDX, IDC_AUTOVALIDATE, m_bAutoValidate);

	// Transfer data type combo box index to vartype:
	CComboBox *pCombo = (CComboBox *)GetDlgItem (IDC_DATATYPE);

	// If save and validate, transfer data type combo box index to
	// vartype member variable:
	if (pDX->m_bSaveAndValidate)
		{
		CString strType;
		pCombo->GetLBText (pCombo->GetCurSel (), strType);
		m_vtDataType = VartypeFromString (strType);
		}

	// else use current member variable value to make combo box selection:
	else
		{
		CString strType;

		// Convert data type to combo box entry string:
		switch (m_vtDataType & ~VT_ARRAY)
			{
			case VT_BOOL:	strType = _T("Boolean");	break;
			case VT_UI1:	strType = _T("Byte");		break;
			case VT_I1:		strType = _T("Char");		break;
			case VT_UI2:	strType = _T("Word");		break;
			case VT_I2:		strType = _T("Short");		break;
			case VT_UI4:	strType = _T("DWord");		break;
			case VT_I4:		strType = _T("Long");		break;
			case VT_R4:		strType = _T("Float");		break;
			case VT_R8:		strType = _T("Double");		break;
			case VT_BSTR:	strType = _T("String");		break;
			default:		strType = _T("Native");		break;
			}

		// Update array flag data:
		if ((m_vtDataType & VT_ARRAY) != 0)
			strType += _T(" Array");
		
		// Select combo box entry:
		pCombo->SelectString (-1, strType);
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemAddDlg message handlers
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
//	BOOL -  TRUE.
// **************************************************************************
BOOL CKItemAddDlg::OnInitDialog () 
	{
	CDialog::OnInitDialog ();

	// Limit the access path to 255 chars:
	((CEdit *)GetDlgItem (IDC_ACCESSPATH))->LimitText (255);
	
	// Allow 10 levels of 32 character names plus a tag name of 31 characters:
	((CEdit *)GetDlgItem (IDC_ITEMID))->LimitText (10*32 + 31);

	// Subclass image buttons:
	m_cNext.SubclassDlgItem (IDC_NEXT, this);
	m_cPrev.SubclassDlgItem (IDC_PREVIOUS, this);
	m_cNew.SubclassDlgItem (IDC_NEW, this);
	m_cDuplicate.SubclassDlgItem (IDC_DUPLICATE, this);
	m_cDelete.SubclassDlgItem (IDC_DELETE, this);
	m_cValidate.SubclassDlgItem (IDC_VALIDATEITEM, this);

	// Create tool tips for the image buttons:
	m_cToolTip.Create (this);
	m_cToolTip.AddWindowTool (&m_cNext);
	m_cToolTip.AddWindowTool (&m_cPrev);
	m_cToolTip.AddWindowTool (&m_cNew);
	m_cToolTip.AddWindowTool (&m_cDuplicate);
	m_cToolTip.AddWindowTool (&m_cDelete);
	m_cToolTip.AddWindowTool (&m_cValidate);

	// Get our group's item management interface pointer for the group:
	m_pIItemMgt = m_pGroup->GetIItemMgt ();

	// Intialize our item browser:
	InitializeBrowser ();

	// Intialize control status:
	UpdateStatus ();

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (TRUE);  
	}

// **************************************************************************
// OnShowWindow ()
//
// Description:
//	This method is called by the framework when this dialog is about to be
//	hidden or shown.  Use opportunity to set focus.
//
// Parameters:
//  BOOL		bShow		TRUE if the window is being shown;  FALSE if 
//							 the window is being hidden.
//	UINT		nStatus		Specifies the status of the window being shown.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnShowWindow (BOOL bShow, UINT nStatus) 
	{
	// Perform default processing:
	CDialog::OnShowWindow (bShow, nStatus);
	
	// If showing dialog, set focus to item ID edit box:
	if (bShow)
		GetDlgItem (IDC_ITEMID)->SetFocus ();
	}

// **************************************************************************
// OnOK ()
//
// Description:
//	Override to perform the OK button action in a modal dialog box.  Use
//	opportunity to apply changes.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnOK () 
	{
	// Apply any outstanding changes:
	if (OnApplyChange ())
		{
		// Terminate the list.  Will allow us to process list using
		// "while (element)" loop if we want:
		if (m_nListIndex > 0)
			m_cItemList.SetAtGrow (m_nListIndex, NULL);

		// Perform default processing:
		CDialog::OnOK ();
		}
	}

// **************************************************************************
// OnCancel ()
//
// Description:
//	Override to perform the Cancel button or ESC key action.  Use opportunity
//	to free any items created.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnCancel () 
	{
	// Free any items we created:
	for (int nIndex = 0; nIndex < m_nListIndex; nIndex++)
		{
		// Get next element in list:
		CKItem *pItem = (CKItem *) m_cItemList.GetAt (nIndex);
		ASSERT (pItem != NULL);

		// Delete it:
		delete pItem;
		}
	
	// Perform default processing:
	CDialog::OnCancel ();
	}

// **************************************************************************
// OnNew ()
//
// Description:
//	New button event handler.  Select a new item for edits.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnNew () 
	{
	// No item currently in list should be selected on new:
	SelectItem (-1);
	}

// **************************************************************************
// OnDuplicate ()
//
// Description:
//	Duplicate button event handler.  Duplicate the current item.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnDuplicate () 
	{
	// Validate selected item first, and see if it has an address with
	// a dot bit format:
	bool bDotBitAddress = false;
	HRESULT hr = Validate (&bDotBitAddress);

	// If current item is not valid, don't duplicate it.  Instead,
	// tell user something is wrong with selected item:
	if (FAILED (hr))
		{
		// Define a string format for the validation return code:
		CString strHR;
		strHR.Format (_T("0x%08X"), hr);

		// Create an error string using above format for return code::
		CString strFailure;
		strFailure.FormatMessage (IDS_VALIDATE_ITEM_FAILED, m_strItemID, strHR);

		// Display a message box with error string:
		AfxMessageBox (strFailure);

		// Return now so we don't duplicate invalid item:
		return;
		}

	// Create a duplicate item dialog.  This will allow the user to specify
	// how many duplicates to make::
	CKDuplicateItemCountDlg dlg;

	// Show as modal dialog.  Duplicate item only if user hits "OK":
	if (dlg.DoModal () == IDOK)
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;

		// Postpone repaint until we are done.  This will make things go
		// faster and look smoother:
		SetRedraw (false);

		// Duplicate item, and select duplicate.  Repeat as many times
		// as user says:
		for (int i = 0; i < dlg.m_cnDuplicateItems; i++)
			SelectItem (-1, true, bDotBitAddress);

		// Now that we are done, allow a repaint:
		SetRedraw (true);

		// Force a repaint:
		Invalidate ();
		}
	}

// **************************************************************************
// OnDelete () 
//
// Description:
//	Delete button event handler.  Deletes the current item.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnDelete () 
	{
	// Delete the selected item (selected item index had better be
	// less than the number of items in list):
	if (m_nSelIndex < m_nListIndex)
		{
		// Get pointer to the selected item's object:
		CKItem *pItem = (CKItem *) m_cItemList.GetAt (m_nSelIndex);
		ASSERT (pItem != NULL);
	
		// Delete the item object:
		delete pItem;

		// Remove the item from the list:
		m_cItemList.RemoveAt (m_nSelIndex);
		m_nListIndex--;

		// Select the next item if there is one (i.e, the item that just 
		// took this positions place from the RemoveAt())
		if (m_nSelIndex < m_nListIndex)
			SelectItem (m_nSelIndex);
		
		// Otherwise select the previous one:
		else
			SelectItem (m_nSelIndex - 1);
		}

#ifdef _DEBUG
	// Delete button should be disabled in this case.  If not, assert
	// false to catch problem in debugger.
	else
		ASSERT (FALSE);
#endif
	}

// **************************************************************************
// OnNext ()
//
// Description:
//	Next button event handler.	Selects next item in list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnNext () 
	{
	// Select the next item (selected item index had better be
	// less than the number of items in list):
	if (m_nSelIndex < m_nListIndex)
		SelectItem (m_nSelIndex + 1);

#ifdef _DEBUG
	// Next button should be disabled in this case.  If not, assert
	// false to catch problem in debugger.
	else
		ASSERT (FALSE);
#endif
	}

// **************************************************************************
// OnPrevious ()
//
// Description:
//	Previous button event handler.  Selects previous item in list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnPrevious () 
	{
	// Select the previous item (selected item index had better be
	// less than the number of items in list):
	if (m_nSelIndex > 0)
		SelectItem (m_nSelIndex - 1);

#ifdef _DEBUG
	// Previous button should be disabled in this case.  If not, assert
	// false to catch problem in debugger.
	else
		ASSERT (FALSE);
#endif
	}

// **************************************************************************
// OnValidateItem ()
//
// Description:
//	Validate button event handler.  Validates selecte item's properties. 
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnValidateItem () 
	{
	// Validate the selected item:
	HRESULT hr = Validate ();

	// If validation fails, inform the user:
	if (FAILED (hr))
		{
		// Define a string format for the validation return code:
		CString strHR;
		strHR.Format (_T("0x%08X"), hr);

		// Define a string format for the validation return code:
		CString strFailure;
		strFailure.FormatMessage (IDS_VALIDATE_ITEM_FAILED, m_strItemID, strHR);

		// Show message box with error string:
		AfxMessageBox (strFailure);
		}
	}

// **************************************************************************
// Validate ()
//
// Description:
//	Validates selected item's properties.
//
// Parameters:
//  bool	*pbDotBitAddress	true if address has "dot bit address" format.
//
// Returns:
//  HRESULT - Use FAILED or SUCCEEDED macro to test.
// **************************************************************************
HRESULT CKItemAddDlg::Validate (bool *pbDotBitAddress /*= NULL */)
	{
	// Assume invalid until proven otherwisw:
	HRESULT hr = E_FAIL;

	// Make sure data in member variables represents current control settings:
	UpdateData (true);

	// Allocate memory for our item definition:
	OPCITEMDEF *pItems = (OPCITEMDEF *) CoTaskMemAlloc (sizeof (OPCITEMDEF));

	// If allocation succeeds, perform validation:
	if (pItems)
		{
		// Initialize some variables:
		HRESULT *pErrors = NULL;
		OPCITEMRESULT *pResults = NULL;
		WCHAR *pszItemID = NULL;
		WCHAR *pszAccessPath = NULL;

		// Check to make sure we have an item ID (debug only):
		ASSERT (!m_strItemID.IsEmpty ());

		// Allocate memory for item ID string:
		pszItemID = (WCHAR *) CoTaskMemAlloc ((m_strItemID.GetLength () + 1) * sizeof (WCHAR));

		// COM requires us to put strings in UNICODE format.
#ifdef _UNICODE
		// This is a UNICODE build so just copy item ID string "as is"
		// to allocated memory:
		lstrcpyn (pszItemID, m_strItemID, m_strItemID.GetLength () + 1);
#else
		// This is an ANSI build so convert item ID string to UNICODE and
		// place result in allocated memory:
		_mbstowcsz (pszItemID, m_strItemID, m_strItemID.GetLength () + 1);
#endif

		// If an access path is specified, then convert its string to UNICODE
		// and copy it to allocated memory:
		if (!m_strAccessPath.IsEmpty ())
			{
			// First allocate memory for the access path string:
			pszAccessPath = (WCHAR *) CoTaskMemAlloc ((m_strAccessPath.GetLength () + 1) * sizeof (WCHAR));

			// Convert if necessary and copy result to allocated memory:
#ifdef _UNICODE
			lstrcpyn (pszAccessPath, m_strAccessPath, m_strAccessPath.GetLength () + 1);
#else
			_mbstowcsz (pszAccessPath, m_strAccessPath, m_strAccessPath.GetLength () + 1);
#endif
			}

		// Fill in item def structure:
		pItems->bActive = m_bActive;
		pItems->dwBlobSize = 0;
		pItems->hClient = NULL;
		pItems->pBlob = NULL;
		pItems->szAccessPath = pszAccessPath;
		pItems->szItemID = pszItemID;
		pItems->vtRequestedDataType = m_vtDataType;
		
		// Validate the single item, (no blob requested):
		try
			{
			// Request server to validate item described initem def structure
			// through the group's item management interface:
			hr = m_pIItemMgt->ValidateItems (1, pItems, FALSE, &pResults, &pErrors);

			// If server reports that item is valid, we may still have to modify
			// the data type.  We will also validate "dot bit address" format.
			if (hr == S_OK)
				{
				// Modify the data type to what the server will use:
				m_vtDataType = pResults->vtCanonicalDataType;

				// Update the dialog controls to reflect data type change:
				UpdateData (false);

				// Flag as modified:
				m_bModified = true;

				// Verify dot bit address setting:
				if (pbDotBitAddress)
					{
					// Assume false until proven otherwise:
					*pbDotBitAddress = false;

					// Data type must be VT_BOOL for dot bit address,  It it's not,
					// fall through with flag set to false:
					if (m_vtDataType == VT_BOOL)
						{
						// To determine if truly a dot bit address, strip off
						// bit number and see if its still valid.

						// First free previous results,
						if (pResults)
							{
							CoTaskMemFree (pResults);
							pResults = NULL;
							}

						// and previous errors:
						if (pErrors)
							{
							CoTaskMemFree (pErrors);
							pErrors = NULL;
							}

						// Get position of "dot" character:
						WCHAR *pwch = wcsrchr (pItems->szItemID, L'.');

						if (pwch)
							{
							// Replace "dot" with NULL terminator, effectively stripping
							// off the whole "dot bit number" portion of the address:
							*pwch = L'\0';

							// Reset the requested data type to VT_EMPTY.  We are only
							// interested in seeing if the address is valid for some
							// unspecified data type.
							pItems->vtRequestedDataType = VT_EMPTY;

							// Request server to validate the "stripped" address:
							if (SUCCEEDED (m_pIItemMgt->ValidateItems (1, pItems, FALSE, &pResults, &pErrors)))
								{
								// If the server reports that the "stipped" address is valid, and
								// that it corresponds to some data type other than VT_BOOL, 
								// then the original address is truely a "dot bit" address:
								if (pResults->vtCanonicalDataType != VT_EMPTY && pResults->vtCanonicalDataType != VT_BOOL)
									*pbDotBitAddress = true;
								}
							}
						}
					}
				}

			// else if server reports that item is not valid.  Assign item
			// error for return:
			else
				{
				hr = pErrors [0];
				}
			}
		
		// Catch exceptions:
		catch (...)
			{
			// Probably something wrong with interface pointer, so invalidate it:
			m_pIItemMgt = NULL;

			// Update dialog control status:
			UpdateStatus ();
			}

		// Free allocations (COM requires us to do this):
		CoTaskMemFree (pItems);

		if (pszItemID)
			CoTaskMemFree (pszItemID);

		if (pszAccessPath)
			CoTaskMemFree (pszAccessPath);

		if (pResults)
			CoTaskMemFree (pResults);

		if (pErrors)
			CoTaskMemFree (pErrors);
		}

	// Return validation return code:
	return (hr);
	}

// **************************************************************************
// OnBrowseFlat ()
//
// Description:
//	Browse Flat check box event handler.  Display list of items according to
//	check box setting.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnBrowseFlat ()
	{
	// Update browse flat flag:
	m_bBrowseFlat = ((CButton *) GetDlgItem (IDC_BROWSEFLAT))->GetCheck ();

	// Resync browse position to root:
	BrowseToRoot ();

	// Get currently selected item:
	HTREEITEM hItem = m_pBranchList->GetSelectedItem ();
	
	// Select current item.  This will cause us to re-browse to position.
	// Data will come back from browser using new organization setting:
	if (hItem != NULL)
		SelectBranch (hItem);
	}

// **************************************************************************
// BrowseToRoot ()
//
// Description:
//	Reset brows position to root object.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::BrowseToRoot ()
	{
	try
		{
		// Try to browse to root in one step using "OPC_BROWSE_TO" (this
		// is not supported and will fail for OPC 1.0 servers):
		HRESULT hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_TO, L"");
		
		// If that fails, browse to root one level at a time using "OPC_BROWSE_UP".
		// (Browse up will fail when we are at root.)
		if (FAILED (hr))
			{
			do
				{
				hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_UP, L"\0");
				} while (SUCCEEDED (hr));
			}
		}
	
	catch (...)
		{
		}
	}

// **************************************************************************
// OnAutoValidate ()
//
// Description:
//	Auto validate check box event handler.  Sets flag according to check box
//	setting.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnAutoValidate ()
	{
	// Save new auto validate setting:
	m_bAutoValidate = ((CButton *) GetDlgItem (IDC_AUTOVALIDATE))->GetCheck ();
	}

// **************************************************************************
// OnBranchExpanding ()
//
// Description:
//	Handles notification that the tree control's item is about to expand or
//	collapse.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnBranchExpanding (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Cast notification header to tree view notification header:
	NM_TREEVIEW* pnmtv = (NM_TREEVIEW*) pNMHDR;

	// Expand branch:
	if (pnmtv->action & TVE_EXPAND)
		{
		ExpandBranch (pnmtv->itemNew.hItem);
		}

	// else delete child branches on collapse:
	else if (pnmtv->action & TVE_COLLAPSE)
		{
		DeleteChildBranches (pnmtv->itemNew.hItem);
		}

	*pResult = 0;
	}

// **************************************************************************
// OnBranchSelected ()
//
// Description:
//	Handles notification that the tree control's selected item has changed.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnBranchSelected (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	// Cast notificatin header to tree view notification header:
	NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)pNMHDR;

	// Select the branch:
	SelectBranch (pnmtv->itemNew.hItem);
	*pResult = 0;
	}

// **************************************************************************
// OnBranchFilterChange ()
//
// Description:
//	Handles notification that branch filter edit control has changed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnBranchFilterChange () 
	{
	// Make sure member variables have values currently displayed in
	// controls:
	UpdateData (true);

	// Delete all items and start over if the branch filter has been modified:
	if (m_pBranchList->GetCount ())
		{
		m_pBranchList->DeleteAllItems ();
		BrowseRootLevel ();
		}
	}

// **************************************************************************
// OnLeafFilterChange ()
//
// Description:
//	Handles notification that leaf filter edit control has changed.
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnLeafFilterChange () 
	{
	// Make sure member variables have values currently displayed in
	// controls: 
	UpdateData (true);

	// Get currently selected item:
	HTREEITEM hItem = m_pBranchList->GetSelectedItem ();
	
	// Select current item.  This will cause us to re-browse to position.
	// Data will come back from browser using new filter settings:
	if (hItem != NULL)
		SelectBranch (hItem);
	}

// **************************************************************************
// OnAccessFilterChange ()
//
// Description:
//	Handles notification that access filter combo box has changed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnAccessFilterChange () 
	{
	// Get the access rights:
	m_dwFilterAccessRights = 
		((CComboBox *) GetDlgItem (IDC_FILTERACCESS))->GetCurSel ();

	// Get currently selected item:
	HTREEITEM hItem = m_pBranchList->GetSelectedItem ();
	
	// Select current item.  This will cause us to re-browse to position.
	// Data will come back from browser using new filter settings:
	if (hItem != NULL)
		SelectBranch (hItem);
	}

// **************************************************************************
// OnVartypeFilterChange ()
//
// Description:
//	Handles notificatin that datatype filter combo box has changed.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnVartypeFilterChange () 
	{
	// Get pointer to data type filter combo box:
	CComboBox *pCombo = (CComboBox *)GetDlgItem (IDC_FILTERTYPE);

	// Get the selected data type string:
	CString strType;
	pCombo->GetLBText (pCombo->GetCurSel (), strType);

	// Convert that string to a variant type:
	m_vtFilterType = VartypeFromString (strType);

	// Get currently selected item:
	HTREEITEM hItem = m_pBranchList->GetSelectedItem ();
	
	// Select current item.  This will cause us to re-browse to position.
	// Data will come back from browser using new filter settings:
	if (hItem != NULL)
		SelectBranch (hItem);
	}

// **************************************************************************
// OnClickLeafList ()
//
// Description:
//	Handles notification that an leaf/item in list control has been double
//	clicked on.
//
// Parameters:
//  NMHDR		*pNMHDR			Contains information about a notification message.
//	LRESULT		*pResult		A 32-bit value returned from a window procedure 
//								  or callback function.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnClickLeafList (NMHDR *pNMHDR, LRESULT *pResult) 
	{
	int nSelItem;

	// Get selected item index:
	nSelItem = m_pLeafList->GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

	// If index looks good, get properties of selected item and
	// update all other controls accordingly:
	if (nSelItem >= 0)
		{
		HRESULT hr;
		WCHAR szItemID [DEFBUFFSIZE];
		LPWSTR lpszQualifiedID;

		// COM requires all strings to be in UNICODE format. Convert
		// item ID if needed, then copy to allocated buffer:
#ifdef _UNICODE
		lstrcpyn (szItemID, m_pLeafList->GetItemText (nSelItem, 0), sizeof (szItemID) / sizeof (WCHAR));
#else
		_mbstowcsz (szItemID, m_pLeafList->GetItemText (nSelItem, 0), sizeof (szItemID) / sizeof (WCHAR));
#endif

		try
			{
			// If we are using a flat browse space, then we need to browse to
			// root first:
			if (m_bBrowseFlat)
				BrowseToRoot ();

			// User browser to get item's fully qualified ID:
			hr = m_pIBrowse->GetItemID (szItemID, &lpszQualifiedID);

			// If we succeeded, update controls:
			if (SUCCEEDED (hr) && lpszQualifiedID)
				{
				// Update selector to end of the list:
				m_nSelIndex = m_nListIndex;

				// Re-initialize for new item:
				m_strItemID = lpszQualifiedID;
				m_strAccessPath.Empty ();
				UpdateData (false);

				// Apply new dataset:
				m_bModified = true;
				OnApplyChange ();

				// Update control state:
				UpdateStatus ();

				// Free server allocation for qualified item id:
				CoTaskMemFree (lpszQualifiedID);
				}
		
			// If we didn't get qualified ID, issue a trace statement for debugging:
			else
				{
				TRACE (_T("OTC: Unable to get the qualified item id for %s\r\n"), 
					m_pLeafList->GetItemText (nSelItem, 0));
				}
			}
		
		// Catch exceptions:
		catch (...)
			{
			m_pIBrowse = NULL;
			UpdateStatus ();
			}
		}

	*pResult = 0;
	}

// **************************************************************************
// OnChange ()
//
// Description:
//	Default event handler for control change notifications.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnChange () 
	{
	// Make sure member variables have values currently displayed in
	// controls:
	UpdateData (true);

	// We will consider ourselves modified if there is an item ID defined:
	m_bModified = !m_strItemID.IsEmpty ();

	// Update control status:
	UpdateStatus ();
	}

// **************************************************************************
// OnAddLeaves ()
//
// Description:
//	Add Leaves button event handler.  Add leaves to list.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::OnAddLeaves ()
	{
	// If one or more items/leaves are selected, then add them:
	if (m_pLeafList->GetSelectedCount ())
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;
		int nItem = 0;

		// Postpone repaint untill we are done.  This will make things
		// go faster and look smoother.
		SetRedraw (false);

		// Get first selected item:
		nItem = m_pLeafList->GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

		// Retrieve the qualified item ID and add it to the list:
		while (nItem >= 0)
			{
			HRESULT hr;
			WCHAR szItemID [DEFBUFFSIZE];
			LPWSTR lpszQualifiedID;

			// COM requires all strings to be in UNICODE format.  Convert item ID
			// if needed and copy to allocated memory:
#ifdef _UNICODE
			lstrcpyn (szItemID, m_pLeafList->GetItemText (nItem, 0), sizeof (szItemID) / sizeof (WCHAR));
#else
			_mbstowcsz (szItemID, m_pLeafList->GetItemText (nItem, 0), sizeof (szItemID) / sizeof (WCHAR));
#endif

			// Use browser to get item's fully qualified ID:
			try
				{
				// If we are using a flag browse space, need to browse to 
				// root first:
				if (m_bBrowseFlat)
					BrowseToRoot ();

				// Get item's fully qualified ID:
				hr = m_pIBrowse->GetItemID (szItemID, &lpszQualifiedID);

				// If we succeeded, then update controls accordingly:
				if (SUCCEEDED (hr) && lpszQualifiedID)
					{
					// Update selector to end of the list:
					m_nSelIndex = m_nListIndex;

					// Re-initialize for new item:
					m_strItemID = lpszQualifiedID;
					m_strAccessPath.Empty ();
					UpdateData (false);

					// Apply new dataset:
					m_bModified = true;
					OnApplyChange ();

					// Update control state:
					UpdateStatus ();

					// Free server allocation for qualified item ID:
					CoTaskMemFree (lpszQualifiedID);
					}

				// If we failed to get fully qualified ID, issue a trace
				// statement for debugging:
				else
					{
					TRACE (_T("OTC: Unable to get the qualified item id for %s\r\n"), 
						m_pLeafList->GetItemText (nItem, 0));
					}
				}
			
			// Catch exceptions:
			catch (...)
				{
				m_pIBrowse = NULL;
				UpdateStatus ();

				break;
				}

			// Get the next item.  Continue to loop until GetNextItem() returns
			// -1, indicating that we have processed all selected items:
			nItem = m_pLeafList->GetNextItem (nItem, LVNI_ALL | LVNI_SELECTED);
			}

		// Now we are done adding items, we can allow repaints:
		SetRedraw (true);

		// Force a repaint:
		Invalidate ();
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemAddDlg helpers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// SelectItem ()
//
// Description:
//	Called to select a item.
//
// Parameters:
//	int			nIndex			List control index of item to select
//	bool		bDuplicate		Set to true if duplication item currently 
//								  selected
//	bool		bDotBitAddress	Set to true if item is being duplicated and
//								  has "dot bit number" address format.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::SelectItem (int nIndex, bool bDuplicate /* = false */, bool bDotBitAddress /* = false */)
	{
	// Apply current changes:
	if (OnApplyChange ())
		{
		CKItem *pItem = NULL;

		// Select the new item:
		m_nSelIndex = nIndex;

		// If we are creating a new item, update selector:
		if (m_nSelIndex == -1)
			m_nSelIndex = m_nListIndex;

		// else if we are viewing an existing item, get pointer to associated
		// item object stored in item list:
		else if (m_nSelIndex < m_nListIndex)
			pItem = (CKItem *) m_cItemList.GetAt (m_nSelIndex);
		
		// else something is wrong:
		else
			{
			ASSERT (FALSE);
			}

		// If we are viewing an existing item update the current
		// dialog data set:
		if (pItem)
			{
			m_strAccessPath = pItem->GetAccessPath ();
			m_bActive = pItem->IsActive ();
			m_vtDataType = pItem->GetDataType ();

			if (m_bBrowseFlat)
				BrowseToRoot ();

			m_strItemID = pItem->GetItemID ();
			}

		// Else create a new item:
		else
			{
			// If we are asked to duplicate, new item will be duplicate
			// of curretnly selected item:
			if (bDuplicate)
				AutoIncrementID (bDotBitAddress);

			// else it will have default properties:
			else
				{
				m_strAccessPath = ITEM_DEFAULT_ACCESSPATH;
				m_bActive = ITEM_DEFAULT_ACTIVESTATE;
				m_strItemID = ITEM_DEFAULT_ITEMID;
				m_vtDataType = ITEM_DEFAULT_DATATYPE;
				}
			}

		// Update control values from associated member variables:
		UpdateData (false);

		// Update control status:
		UpdateStatus ();
		}
	}

// **************************************************************************
// OnApplyChange ()
//
// Description:
//	Called to apply changes.
//
// Parameters:
//  none
//
// Returns:
//  bool - true if success.
// **************************************************************************
bool CKItemAddDlg::OnApplyChange ()
	{
	// No need to apply changes unless we are modified:
	if (m_bModified)
		{
		CKItem *pItem = NULL;

		// Make sure member variables have values currently displayed in
		// associated controls:
		UpdateData (true);

		// If auto validating, validate item before applying changes.
		if (m_bAutoValidate)
			{
			// Validate currently selected item:
			HRESULT hr = Validate ();

			// If item is invalid, then inform user of problem and give him
			// option of continuing:
			if (FAILED (hr))
				{
				// Define a string format for the validation return code:
				CString strHR;
				strHR.Format (_T("0x%08X"), hr);

				// Define a string format for the validation return code:
				CString strFailure;
				strFailure.FormatMessage (IDS_VALIDATE_ITEM_FAILED_CONTINUE, m_strItemID, strHR);

				// Show message bow with error string.  Give user of option of
				// continuing.  If user clicks "NO", return now without applying
				// changes:
				if (AfxMessageBox (strFailure, MB_YESNO | MB_ICONQUESTION) == IDNO)
					return (false);
				}
			}

		// If we make it here, it is OK to apply changes.
		
		// If selected index is the same as list control index, then we
		// need to create a new item and add to the list:
		if (m_nSelIndex == m_nListIndex)
			{
			try 
				{
				// Create a new item object and add it to the list:
				pItem = new CKItem (m_pGroup);
				m_cItemList.SetAtGrow (m_nListIndex++, pItem);
				}
			
			catch (...)
				{
				ASSERT (FALSE);
				}
			}
		
		// else we are modifying an item already in list:
		else
			{
			pItem = (CKItem *) m_cItemList.GetAt (m_nSelIndex);
			ASSERT (pItem != NULL);
			}

		// Set item properties:
		pItem->SetAccessPath (m_strAccessPath);
		pItem->SetActive (m_bActive);
		pItem->SetDataType (m_vtDataType);
		pItem->SetItemID (m_strItemID);

		// Set modified flag:
		m_bModified = false;
		}

	// Return true to indicate success:
	return (true);
	}

// **************************************************************************
// AutoIncrementID ()
//
// Description:
//	Create a new item ID by incrementing current ID.
//
// Parameters:
//  bool		bDotBitAddress	Set to true if item has "dot bit number"
//								  address format.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::AutoIncrementID (bool bDotBitAddress)
	{
	static TCHAR szBuffer [DEFBUFFSIZE];
	static TCHAR *pch = NULL;

	// If the item ID is defined, we will modify it by incrementing:
	if (!m_strItemID.IsEmpty ())
		{
		// Make a local copy of the current item ID:
		lstrcpyn (szBuffer, m_strItemID, sizeof (szBuffer) / sizeof (TCHAR));

		// Ensure it is null terminated:
		szBuffer [DEFBUFFSIZE - 1] = '\0';

		// Get a pointer to the last character of the string (search for 
		// NULL terminator, then back up a character):
		pch = _tcschr (szBuffer, _T('\0'));
		pch--;

		// If it is a digit we can simply increment this address:
		if (_istdigit (*pch))
			{
			int nNumber;

			// Scan backwards from last character until we encounter
			// a non-digit:
			while (_istdigit (*pch) && pch != szBuffer)
				pch--;

			// Obtain the number we are peeling off (go to next character
			// which will be the first digit, then convert text to right
			// to an integer):
			pch++;
			nNumber = _ttol (pch);

			// Increment the number:
			nNumber++;

			// If we are dealing with a "dot bit number" address,
			// make sure we do not increment bit number beyond 15.
			// Increment bit numbers to 15, then increment offset
			// and make bit number 0.

			// If bDotBitAddress is set, then character to the left
			// of current pointer should be the "dot".
			if (bDotBitAddress && *(pch - 1) == _T('.'))
				{
				// Assume dot specifier is a bit and increment 0 - 15.

				// If nNumber is greater than 15, determine offset, and
				// increment that instead of the bit number:
				if (nNumber > 15)
					{
					// Roll offset ahead:
					pch = _tcsrchr (szBuffer, _T('.'));
					pch--;

					// Peel back looking for the first non-digit:
					while (_istdigit (*pch) && pch != szBuffer)
						pch--;

					// Go to next character and convert to integer.  This will
					// be the offset since _ttol will not use any characters to 
					// the right of the dot.  Increment the result by one.  This
					// will be our new offset.
					pch++;
					nNumber = _ttol (pch) + 1;

					// Replace old offset and bit number with incremented
					// offset and bit number "00".
					_stprintf (pch, _T("%d.00"), nNumber);
					}

				// Otherwise just replace the bit number with incremeted
				// bit number:
				else
					_stprintf (pch, _T("%02d"), nNumber);
				}

			// If not a dot bit number address, make sure we do not lose
			// any leading zeros (as in modbus addresses):
			else
				{
				// We should currently be pointing at first character of 
				// trailing digit portion of address.  Check to see if
				// this is a zero.
				if (*pch == _T('0'))
					{
					// We found a leading zero.
					TCHAR *pZeros = pch;
					int cnDigits = 0;

					// Scan forwared until we find a non-zero character.
					// Count each zero along the way:
					while (*pZeros++ != _T('\0'))
						cnDigits++;

					// Replace the trailing digits of the adress with
					// the new incremented number, keeping the same
					// number of leading zeros:
					_stprintf (pch, _T("%0*d"), cnDigits, nNumber);
					}

				// Else if no leading zeros, just replace the trailing 
				// digits with the new incremented number:
				else
					_stprintf (pch, _T("%d"), nNumber);
				}
			}
		
		// else if last character is not a digit, append a good starting offset:
		else
			*(pch + 1) = '0';

		// Update item ID:
		m_strItemID = szBuffer;

		// Update controls:
		UpdateData (false);

		// Set modifed flag:
		m_bModified = true;		
		}
	}

// **************************************************************************
// UpdateStatus ()
//
// Description:
//	Update status of dialog controls based on current item selected.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::UpdateStatus ()
	{
	// Determine the focus so that if we disable the focused control
	// we can move it.
	HWND hWnd = ::GetFocus ();

	// Enable the next button if the selected item has an item ID defined:
	m_cNext.EnableWindow (m_nSelIndex < m_nListIndex - 1);

	// Enable the previous button if the selected item has a previous button:
	m_cPrev.EnableWindow (m_nSelIndex > 0);

	// Enable the new button if there is a valid item ID entered
	// (otherwise they are already editting a new item):
	m_cNew.EnableWindow (!m_strItemID.IsEmpty ());

	// Duplicate any item that has an item ID:
	m_cDuplicate.EnableWindow (!m_strItemID.IsEmpty ());

	// Enable the delete button if the item selected is not the one we 
	// are adding to the list:
	m_cDelete.EnableWindow (m_nSelIndex < m_nListIndex);

	// Enable validate item:
	m_cValidate.EnableWindow (m_pIItemMgt && !m_strItemID.IsEmpty ());

	// Update the browse controls:
	m_pBranchList->EnableWindow (m_pIBrowse != NULL);
	m_pLeafList->EnableWindow (m_pIBrowse != NULL);

	GetDlgItem (IDC_ADD_LEAVES)->EnableWindow (m_pIBrowse != NULL);
	GetDlgItem (IDC_FILTERACCESS)->EnableWindow (m_pIBrowse != NULL);
	GetDlgItem (IDC_FILTERLEAF)->EnableWindow (m_pIBrowse != NULL);
	GetDlgItem (IDC_FILTERTYPE)->EnableWindow (m_pIBrowse != NULL);
	GetDlgItem (IDC_FILTERBRANCH)->EnableWindow (m_pIBrowse != NULL);

	// Update the item count display:
	CString strItemCount;
	strItemCount.Format (_T("Item Count: %d"), m_nListIndex);
	((CStatic *) GetDlgItem (IDC_ITEM_COUNT))->SetWindowText (strItemCount);

	// Set the focus to the first control if the previous control with the
	// focus just lost it:
	if (hWnd && !::IsWindowEnabled (hWnd))
		GetDlgItem (IDC_ITEMID)->SetFocus ();
	}

// **************************************************************************
// InitializeBrowser ()
//
// Description:
//	Initialize things related to the browser.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::InitializeBrowser ()
	{
	// Get pointers to our browse controls for easy access:
	m_pBranchList = (CTreeCtrl *) GetDlgItem (IDC_BRANCHLIST);
	ASSERT (m_pBranchList != NULL);

	m_pLeafList = (CListCtrl *) GetDlgItem (IDC_LEAFLIST);
	ASSERT (m_pLeafList != NULL);

	// Initialize the image list for the branch control.  These are the 
	// images that show up to the left of each item in list.  The bitmap
	// must use a purple background color, RGB (255, 0, 255), so that the
	// CImageList object can construct a mask.  The images are 16x16 pixels.
	// Set the image list background color to CLR_NONE so masked pixels will
	// be transparent. 
	//
	// Image number		Use
	//	0-7				not used
	//	8				Branch
	//	9				Self branch
	m_cBranchImageList.Create (IDB_GROUPIMAGES, 16, 3, RGB (255, 0, 255));
	m_cBranchImageList.SetBkColor (CLR_NONE);
	m_pBranchList->SetImageList (&m_cBranchImageList, TVSIL_NORMAL);

	// Initialize the image list for the leaf control.  These are the images
	// that show up to the left of each item in list.  The bitmap must use a 
	// purple background color, RGB (255, 0, 255), so that the CImageList
	// object can construct a mask.  The images are 14x14 pixels.  Set the
	// image list background color to CLR_NONE so masked pixels will be
	// transparent. 
	//
	// Image number		Use
	//	0-2				not used
	//	3				Leaf
	m_cLeafImageList.Create (IDB_ITEMIMAGES, 14, 2, RGB (255, 0, 255));
	m_cLeafImageList.SetBkColor (CLR_NONE);
	m_pLeafList->SetImageList (&m_cLeafImageList, LVSIL_SMALL);

	// Insert a column for our leaf item list control:
	CRect rc;
	m_pLeafList->GetClientRect (&rc);
	m_pLeafList->InsertColumn (0, _T(""), LVCFMT_LEFT, rc.Width ());	

	// Initialize filters:
	m_strFilterBranch = "*";
	((CEdit *) GetDlgItem (IDC_FILTERBRANCH))->SetWindowText (m_strFilterBranch);

	m_strFilterLeaf = "*";
	((CEdit *) GetDlgItem (IDC_FILTERLEAF))->SetWindowText (m_strFilterLeaf);

	m_vtFilterType = VT_EMPTY;
	((CComboBox *) GetDlgItem (IDC_FILTERTYPE))->SetCurSel (0);

	m_dwFilterAccessRights = CB_ACCESS_ANY;
	((CComboBox *) GetDlgItem (IDC_FILTERACCESS))->SetCurSel (CB_ACCESS_ANY);

	// Intialize browse controls if browsing is supported, otherwise
	// just disable the controls:
	if (m_pIBrowse != NULL)
		{
		// Create a wait cursor object.  This will cause the wait cursor, 
		// usually an hourglass, to be displayed.  When this object goes
		// out of scope, its destructor will restore the previous cursor
		// type.
		CWaitCursor wc;
		HRESULT hr;

		// Query the server name space:
		hr = m_pIBrowse->QueryOrganization (&m_cOpcNameSpace);

		if (SUCCEEDED (hr))
			{
			// Browse root level:
			switch (m_cOpcNameSpace)
				{
				case OPC_NS_HIERARCHIAL:
				case OPC_NS_FLAT:
					BrowseRootLevel ();
					break;

				// Did the foundation add a new type?
				default:
					ASSERT (FALSE);
					break;
				}
			}

		// We will assume a hierarchial search if the server fails this request:
		// (This allows us to work with any server that uses the ICONICS toolkit.)
		else
			{
			TRACE (_T("OTC: Attempting agressive browsing since the server failed on QueryOrganization ()\r\n"));

			m_cOpcNameSpace = OPC_NS_HIERARCHIAL;
			BrowseRootLevel ();
			}
		}
	}

// **************************************************************************
// BrowseRootLevel ()
//
// Description:
//	Reset the browser to the root object.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::BrowseRootLevel ()
	{
	HRESULT hr = E_FAIL;
	HTREEITEM hItem = NULL;

	try
		{
		// If hierarchial namespace:
		if (m_cOpcNameSpace == OPC_NS_HIERARCHIAL)
			{
			// Re-initialize server's browse position to the root:
			do
				{
				// Use the OPC_BROWSE_UP rather than the OPC_BROWSE_TO which
				// is only supported in OOPC version 2.0.  We will have to 
				// browse up to root one level at a time.  Function will
				// fail when we are at root.
				hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_UP, L"\0");
				} while (SUCCEEDED (hr));

			// Insert our root level item:
			hItem = m_pBranchList->InsertItem ((m_pGroup->GetParentServer ())->GetProgID (), 
				ILI_BRANCH, ILI_SELBRANCH, TVI_ROOT);

			// Set the item data and add a dummy child branch:
			m_pBranchList->SetItemData (hItem, DWORD (-1));
			AddDummyBranch (hItem);
			}

		// Else flat namespace:
		else
			{
			ASSERT (m_cOpcNameSpace == OPC_NS_FLAT);

			// Insert our root level item (there is no need to add any dummy
			// branches since a flat space will only have one level of leaves)
			hItem = m_pBranchList->InsertItem ((m_pGroup->GetParentServer ())->GetProgID (), 
				ILI_BRANCH, ILI_SELBRANCH, TVI_ROOT);
			}

		// Select root item:
		if (hItem)
			m_pBranchList->SelectItem (hItem);
		}
	
	catch (...)
		{
		m_pIBrowse = NULL;
		UpdateStatus ();
		}
	}

// **************************************************************************
// AddBranches ()
//
// Description:
//	Add branch to tree control.
//
// Parameters:
//  LPENUMSTRING	pIEnumString	Stores the interface pointer
//	HTREEITEM		hParent			Handle of parent tree item
//	DWORD			dwData			Item data value to give new branches
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::AddBranches (LPENUMSTRING pIEnumString, HTREEITEM hParent, DWORD dwData)
	{
	ASSERT (hParent != NULL);

	ULONG celt = 1;			
	LPOLESTR rgelt;			
	ULONG celtFetched = 0;	
	TCHAR szBuffer [DEFBUFFSIZE];

	// Remove the dummy branch if one exists:
	RemoveDummyBranch (hParent);

	// Start at the beginning of the list:
	pIEnumString->Reset ();	
	pIEnumString->Next (celt, &rgelt, &celtFetched);

	// Add each branch to the browse control:
	while (celtFetched > 0) 
		{
		HTREEITEM hNewItem = NULL;

		// COM requis that all strings be sent in UNICODE format.
		// Convert if necessary and copy to szBuffer:
#ifdef _UNICODE
		lstrcpyn (szBuffer, rgelt, sizeof (szBuffer) / sizeof (TCHAR));
#else
		_wcstombsz (szBuffer, rgelt, sizeof (szBuffer) / sizeof (TCHAR));
#endif

		// Insert the branch:
		hNewItem = m_pBranchList->InsertItem (szBuffer, ILI_BRANCH, ILI_SELBRANCH, hParent);
		m_pBranchList->SetItemData (hNewItem, dwData);	

		// Always fake each branch into having a sub item in the tree:
		AddDummyBranch (hNewItem);

		// Free the branch name:
		CoTaskMemFree (rgelt);	

		// Re-initialize and get the next item:
		celt = 1;
		celtFetched = 0;
		pIEnumString->Next (celt, &rgelt, &celtFetched);
		}
	}

// **************************************************************************
// ExpandBranch ()
//
// Description:
//	Called to expand branch.
//
// Parameters:
//  HTREEITEM	hItem		Handle of tree control branch to expand.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::ExpandBranch (HTREEITEM hItem)
	{
	ASSERT (hItem != NULL);

	int nPos;
	HRESULT hr;
	LPENUMSTRING pIEnumString;

#ifndef _UNICODE
	WCHAR szFilter [DEFBUFFSIZE];
#endif

	// Get the new browse position from the item that was previously
	// selected:
	nPos = (int) m_pBranchList->GetItemData (hItem);
	nPos++;
	ASSERT (nPos >= 0);

	try
		{
		// Re-intialize the server's position to the root level. 
		do
			{
			// Use the OPC_BROWSE_UP rather than the OPC_BROWSE_TO which
			// is only supported in OOPC version 2.0.  We will have to 
			// browse up to root one level at a time.  Function will
			// fail when we are at root.
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_UP, L"\0");
			} while (SUCCEEDED (hr));

		// Now browse down to the new position:
		CStringArray strBranches;
		HTREEITEM hParentItem;

		strBranches.SetSize (nPos + 1);
		hParentItem = hItem;

		for (int i = 0; i <= nPos; i++)
			{
			ASSERT (hItem);
			strBranches [i] = m_pBranchList->GetItemText (hParentItem);
			hParentItem = m_pBranchList->GetParentItem (hParentItem);
			}

		hr = S_OK;

		// > 0 we do not want to include the "Root" item since the
		// client only uses this branch:
		while (SUCCEEDED (hr) && nPos-- > 0)	
			{
#ifdef _UNICODE
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_DOWN, strBranches [nPos]);
#else
			WCHAR szBranch [DEFBUFFSIZE];
			_mbstowcsz (szBranch, strBranches [nPos], sizeof (szBranch) / sizeof (WCHAR));
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_DOWN, szBranch);
#endif
			}

		// Browse for root level:
#ifdef _UNICODE
		hr = m_pIBrowse->BrowseOPCItemIDs (OPC_BRANCH,				// provide items with children
										  m_strFilterBranch,		// id filtering
										  VT_EMPTY,					// no datatype filtering on a branch
										  0,						// no access filtering on a branch
										  &pIEnumString);			// store the interface pointer here
#else
		_mbstowcsz (szFilter, m_strFilterBranch, sizeof (szFilter) / sizeof (WCHAR));
		hr = m_pIBrowse->BrowseOPCItemIDs (OPC_BRANCH,				// provide items with children
										  szFilter,					// id filtering
										  VT_EMPTY,					// no datatype filtering on a branch
										  0,						// no access filtering on a branch
										  &pIEnumString);			// store the interface pointer here
#endif

		// On success add the branches to the root:
		if (SUCCEEDED (hr) && pIEnumString)
			{
			AddBranches (pIEnumString, hItem, m_pBranchList->GetItemData (hItem) + 1);
			pIEnumString->Release ();
			}
		else
			{
			RemoveDummyBranch (hItem);
			throw (-1);
			}
		}
	
	catch (...)
		{
		m_pIBrowse = NULL;
		UpdateStatus ();
		}
	}

// **************************************************************************
// SelectBranch ()
//	
// Description:
//	Select a branch in tree control.
//
// Parameters:
//  HTREEITEM	hItem		Handle of tree control's item to select
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::SelectBranch (HTREEITEM hItem)
	{
	ASSERT (hItem != NULL);

	int nPos;
	HRESULT hr;
	LPENUMSTRING pIEnumString;
	CStringArray strBranches;
	HTREEITEM hParentItem;

#ifndef _UNICODE
	WCHAR szFilter [DEFBUFFSIZE];
#endif

	// Get the new browse position from the item that was previously
	// selected:
	nPos = (int) m_pBranchList->GetItemData (hItem);
	nPos++;
	ASSERT (nPos >= 0);

	try
		{
		// Re-intialize the server's position to the root level:
		do
			{
			// Use the OPC_BROWSE_UP rather than the OPC_BROWSE_TO which
			// is only supported in OOPC version 2.0.  We will have to 
			// browse up to root one level at a time.  Function will
			// fail when we are at root.
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_UP, L"\0");
			} while (SUCCEEDED (hr));

		// Now browse down to the new position:
		strBranches.SetSize (nPos + 1);
		hParentItem = hItem;

		for (int i = 0; i <= nPos; i++)
			{
			ASSERT (hItem);
			strBranches [i] = m_pBranchList->GetItemText (hParentItem);
			hParentItem = m_pBranchList->GetParentItem (hParentItem);
			}

		hr = S_OK;

		// > 0 we do not want to include the "Root" item since the client
		// only uses this branch:
		while (SUCCEEDED (hr) && nPos-- > 0)	
			{
#ifdef _UNICODE
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_DOWN, strBranches [nPos]);
#else
			WCHAR szBranch [DEFBUFFSIZE];

			_mbstowcsz (szBranch, strBranches [nPos], sizeof (szBranch) / sizeof (WCHAR));
			hr = m_pIBrowse->ChangeBrowsePosition (OPC_BROWSE_DOWN, szBranch);
#endif
			}

		// Browse for root level:
#ifdef _UNICODE
		hr = m_pIBrowse->BrowseOPCItemIDs (
			m_bBrowseFlat ? OPC_FLAT : OPC_LEAF,	// provide items with children
			m_strFilterLeaf,						// item id filtering
			m_vtFilterType,							// datatype filter
			m_dwFilterAccessRights,					// access rights filtering
			&pIEnumString);							// store the interface pointer here
#else
		_mbstowcsz (szFilter, m_strFilterLeaf, sizeof (szFilter) / sizeof (WCHAR));

		hr = m_pIBrowse->BrowseOPCItemIDs (
			m_bBrowseFlat ? OPC_FLAT : OPC_LEAF,	// provide items with children
			szFilter,								// item id filtering
			m_vtFilterType,							// datatype filter
			m_dwFilterAccessRights,					// access rights filtering
			&pIEnumString);							// store the interface pointer here
#endif

		// On success add the branches to the root:
		if (SUCCEEDED (hr) && pIEnumString)
			{
			AddLeaves (pIEnumString);
			pIEnumString->Release ();
			}
		else
			throw (-1);
		}
	
	catch (...)
		{
		m_pIBrowse = NULL;
		UpdateStatus ();
		}
	}

// **************************************************************************
// AddDummyBranch ()
//
// Description:
//	Add a dummy branch to tree control.
//
// Parameters:
//  HTREEITEM	hParent		Handle to parent item.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::AddDummyBranch (HTREEITEM hParent)
	{
	ASSERT (hParent != NULL);
	
	HTREEITEM hDummyItem;

	// Insert a dummy item:
	hDummyItem = m_pBranchList->InsertItem (NULL_ITEM_NAME, hParent);
	ASSERT (hDummyItem != NULL);
			
	m_pBranchList->SetItemData (hDummyItem, NULL_ITEM_DATA);
	}

// **************************************************************************
// RemoveDummyBranch ()
//
// Description:
//	Remove a dummy branch from tree control.
//
// Parameters:
//  HTREEITEM	hParent		Handle of parent item.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::RemoveDummyBranch (HTREEITEM hParent)
	{
	ASSERT (hParent != NULL);

	HTREEITEM hDummyItem;

	// Get child item:
	hDummyItem = m_pBranchList->GetChildItem (hParent);
	while (hDummyItem)
		{
		CString strItem = m_pBranchList->GetItemText (hDummyItem);
		
		if (strItem.CompareNoCase (NULL_ITEM_NAME) == 0)
			{
			if (m_pBranchList->GetItemData (hDummyItem) == NULL_ITEM_DATA)
				{
				m_pBranchList->DeleteItem (hDummyItem);
				break;
				}
			}

		hDummyItem = m_pBranchList->GetNextSiblingItem (hDummyItem);
		}
	}

// **************************************************************************
// DeleteChildBranches ()
//
// Description:
//	Deletes all child branches of specified item in tree control
//
// Parameters:
//  HTREEITEM	hParent		Handle to parent item.
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::DeleteChildBranches (HTREEITEM hParent)
	{
	ASSERT (hParent != NULL);

	HTREEITEM hItem;
	hItem = m_pBranchList->GetChildItem (hParent);

	while (hItem)
		{
		m_pBranchList->DeleteItem (hItem);
		hItem = m_pBranchList->GetChildItem (hParent);
		}
	
	AddDummyBranch (hParent);
	}

// **************************************************************************
// AddLeaves ()
//
// Description:
//	Add leaves to tree control.
//
// Parameters:
//  LPENUMSTRING	pIEnumString	Stores the interface pointer
//
// Returns:
//  void
// **************************************************************************
void CKItemAddDlg::AddLeaves (LPENUMSTRING pIEnumString)
	{
	ULONG celt = 1;			
	LPOLESTR rgelt;			
	ULONG celtFetched = 0;	
	int nIndex = 0;

#ifndef _UNICODE
	TCHAR szBuffer [DEFBUFFSIZE];
#endif

	// Delete any leaves that are presently being displayed:
	m_pLeafList->DeleteAllItems ();

	// Start at the beginning of the list:
	pIEnumString->Reset ();	
	pIEnumString->Next (celt, &rgelt, &celtFetched);

	// Add each leaf to the leaf control:
	while (celtFetched > 0) 
		{
		// Insert the leaf:
#ifdef _UNICODE
		m_pLeafList->InsertItem (nIndex++, rgelt, ILI_LEAF);
#else
		_wcstombsz (szBuffer, rgelt, sizeof (szBuffer) / sizeof (TCHAR));
		m_pLeafList->InsertItem (nIndex++, szBuffer, ILI_LEAF);
#endif

		// Free the branch name:
		CoTaskMemFree (rgelt);	

		// Re-initialize and get the next item:
		celt = 1;
		celtFetched = 0;
		pIEnumString->Next (celt, &rgelt, &celtFetched);
		}

	// Select first leaf by default:
	if (m_pLeafList->GetItemCount ())
		m_pLeafList->SetItemState (0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}


/////////////////////////////////////////////////////////////////////////////
// CKDuplicateItemCountDlg dialog
/////////////////////////////////////////////////////////////////////////////

#define MIN_DUPLICATE_ITEMS		1
#define MAX_DUPLICATE_ITEMS		32767

// **************************************************************************
// CKDuplicateItemCountDlg ()
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
CKDuplicateItemCountDlg::CKDuplicateItemCountDlg () : CDialog (IDD_DUPLICATE_COUNT)
	{
	m_cnDuplicateItems = MIN_DUPLICATE_ITEMS;
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
void CKDuplicateItemCountDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Exchange data between controls and associated member variables:
	DDX_Text (pDX, IDC_COUNT, m_cnDuplicateItems);
	DDV_MinMaxInt (pDX, m_cnDuplicateItems, MIN_DUPLICATE_ITEMS, MAX_DUPLICATE_ITEMS);
	}
