// **************************************************************************
// itempropertiesdlg.cpp
//
// Description:
//	Implements a dialog class for assigning OPC item properties.
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
#include "itempropertiesdlg.h"
#include "item.h"
#include "server.h"
#include "mainwnd.h"

#define NUMCOLUMNS			4


/////////////////////////////////////////////////////////////////////////////
// CKItemPropertiesDlg dialog
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
BEGIN_MESSAGE_MAP (CKItemPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(CKItemPropertiesDlg)
	ON_BN_CLICKED (IDC_NEXT, OnNext)
	ON_BN_CLICKED (IDC_PREVIOUS, OnPrevious)
	ON_BN_CLICKED (IDC_ACTIVE, OnActive)
	ON_CBN_SELCHANGE (IDC_DATATYPE, OnChangeDataType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


// **************************************************************************
// CKItemPropertiesDlg ()
//
// Description:
//	Constructor.
//
// Parameters:
//  CObArray	&cItemList	Array of items objects that can be viewed and 
//							  modified with this dialog.
//	DWORD		cdwItems	Number of items in cItemList.
//	IOPCItemMgt	*pIItemMgt	Pointer to IOPCItemMgt interface.
//	CKServer	*pServer	Pointer to server items belong to.
//	CWnd		*pParent	Pointer to parent window.
//
// Returns:
//  none
// **************************************************************************
CKItemPropertiesDlg::CKItemPropertiesDlg (CObArray &cItemList, 
										  DWORD cdwItems, 
										  IOPCItemMgt *pIItemMgt,
										  CKServer *pServer,
										  CWnd *pParent /*=NULL*/)
	: CDialog (CKItemPropertiesDlg::IDD, pParent)
	{
	ASSERT (cdwItems > 0);
	ASSERT (pServer);

	// Intialize members:
	m_pItemList = &cItemList;
	m_cnItems = (int) cdwItems;
	m_pIItemMgt = pIItemMgt;
	m_pIItemProps = pServer->GetIItemProps ();
	m_pServer = pServer;

	m_nSelIndex = 0;
	m_bModified = false;

	// Initialize bitmap buttons:
	m_cNext.Initialize (IDB_NEXT, IDB_NEXTGRAY);
	m_cPrev.Initialize (IDB_PREVIOUS, IDB_PREVIOUSGRAY);
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
void CKItemPropertiesDlg::DoDataExchange (CDataExchange *pDX)
	{
	// Perform default processing:
	CDialog::DoDataExchange (pDX);

	// Exchange data between controls and associated member variables:
	//{{AFX_DATA_MAP(CKItemPropertiesDlg)
	DDX_Text (pDX, IDC_ACCESSPATH, m_strAccessPath);
	DDX_Check (pDX, IDC_ACTIVE, m_bActive);
	DDX_Text (pDX, IDC_ITEMID, m_strItemID);
	//}}AFX_DATA_MAP

	// Get pointer to data type combo box:
	CComboBox *pCombo = (CComboBox *)GetDlgItem (IDC_DATATYPE);
	CString strType;

	// If not save and validate, use member vaiable value to make
	// data type combo box selection:
	if (!pDX->m_bSaveAndValidate)
		{
		CString strType;

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
		
		// Select data type:
		pCombo->SelectString (-1, strType);
		}

	// else transfer data type combo box index to vartype member variable:
	else
		{
		pCombo->GetLBText (pCombo->GetCurSel (), strType);
		m_vtDataType = VartypeFromString (strType);
		}
	}	


/////////////////////////////////////////////////////////////////////////////
// CKItemPropertiesDlg message handlers
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
//	BOOL - Result of base class processing.
// **************************************************************************
BOOL CKItemPropertiesDlg::OnInitDialog () 
	{
	CString strText;

	// Limit the access path to 255 chars:
	((CEdit *)GetDlgItem (IDC_ACCESSPATH))->LimitText (255);
	
	// Allow 10 levels of 32 character names plus a tag name of 31 characters:
	((CEdit *)GetDlgItem (IDC_ITEMID))->LimitText (10*32 + 31);

	// Subclass image buttons:
	m_cNext.SubclassDlgItem (IDC_NEXT, this);
	m_cPrev.SubclassDlgItem (IDC_PREVIOUS, this);

	// Create tool tips for image buttons:
	m_cToolTip.Create (this);
	m_cToolTip.AddWindowTool (&m_cNext);
	m_cToolTip.AddWindowTool (&m_cPrev);

	// Intialize IItemProperties 2.0 list control headings:
	CListCtrl *pList = (CListCtrl *) GetDlgItem (IDC_LIST);
	ASSERT (pList != NULL);

	// Get rectangle that bounds list control:
	CRect rc;
	pList->GetWindowRect (&rc);

	// Calculate the width of the list control:
	int nTotalWidth;
	nTotalWidth = rc.right - rc.left;
	
	// Create the list control columns.  Headers will be loaded from
	// string resources and widths will be a function of total width
	// of list control:
	for (int i = 0; i < NUMCOLUMNS; i++)
		{
		int nWidth;

		switch (i)
			{
			case 0:	// ID
				strText.LoadString (IDS_ID);
				nWidth = nTotalWidth / 8;
				break;

			case 1: // Description
				strText.LoadString (IDS_DESCRIPTION);
				nWidth = nTotalWidth / 3;
				break;

			case 2:	// Value
				strText.LoadString (IDS_VALUE);
				nWidth = nTotalWidth / 6;
				break;

			case 3: // Item ID
				strText.LoadString (IDS_ITEMID);
				nWidth = nTotalWidth / 3;
				break;

			default:
				ASSERT (FALSE);
				break;
			}

		// Insert the column:
		pList->InsertColumn (i, strText, LVCFMT_LEFT, nWidth);
		}

	// Enable/disable the window based on whether or not the 2.0 
	// functionality is available. (If m_pIItemProps interface
	// pointer is NULL, we can assume functionality is not available.)
	pList->EnableWindow (m_pIItemProps != NULL);

	// Select the first item:
	SelectItem (0);

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return (CDialog::OnInitDialog ());  
	}

// **************************************************************************
// OnNext ()
//
// Description:
//	Next button event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnNext () 
	{
	// Select the next item:
	if (m_nSelIndex < m_cnItems)
		SelectItem (m_nSelIndex + 1);

#ifdef _DEBUG
	// Button should be disabled in this case:
	else
		ASSERT (FALSE);
#endif
	}

// **************************************************************************
// OnPrevious ()
//
// Description:
//	Previous button event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnPrevious () 
	{
	// Select the previous item:
	if (m_nSelIndex > 0)
		SelectItem (m_nSelIndex - 1);

#ifdef _DEBUG
	// Button should be disabled in this case:
	else
		ASSERT (FALSE);
#endif
	}

// **************************************************************************
// OnApply ()
//
// Description:
//	Called to apply changes to currently selected item.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnApply ()
	{
	// Only apply changes if Item ID control is enabled.  This will be the
	// case if the item is invalid to begin with:
	if (GetDlgItem (IDC_ITEMID)->IsWindowEnabled ())
		{
		try
			{
			// Transfer dialog contents to member vairables:
			UpdateData (true);

			// Define an array OPC Item Def structures to make property change
			// request:
			OPCITEMDEF *pItemArray = NULL;

			// Get selected item:
			CKItem *pItem = GetSelectedItem ();
			ASSERT (pItem);

			// Update item object:
			pItem->SetAccessPath (m_strAccessPath);
			pItem->SetItemID (m_strItemID);
			pItem->SetActive (m_bActive);
			pItem->SetDataType (m_vtDataType);

			// Attempt to add the item to the OPC server:
			if (m_pIItemMgt)
				{
				// Allocate memory for item definition:
				pItemArray = (OPCITEMDEF *) CoTaskMemAlloc (sizeof (OPCITEMDEF));

				// If allocation succeeds, fill in definition structure:
				if (pItemArray)
					{
					OPCITEMRESULT *pResults = NULL;
					HRESULT *pErrors = NULL;
					HRESULT hr;
					DWORD dwLen;

					// COM requires all strings be in UNICODE format.  We may have
					// to convert format before copying into request structure.

					// First process the access path string:
					dwLen = lstrlen (pItem->GetAccessPath ());

					// Access path is optional.  If it is not defined, i.e. it's length
					// is zero, just set pointer to string to NULL:
					if (dwLen)
						{
						// Allocate memory for the string:
						pItemArray->szAccessPath = (WCHAR *) CoTaskMemAlloc ((dwLen + 1) * sizeof (WCHAR));

#ifdef _UNICODE
						// This is a UNICODE build, so copy the string into allocated
						// memory as is:
						lstrcpyn (pItemArray->szAccessPath, pItem->GetAccessPath (), dwLen + 1);
#else
						// This is an ANSI build, so convert format an copy result into
						// allocated memory:
						MultiByteToWideChar (CP_ACP, 0, pItem->GetAccessPath (), -1, pItemArray->szAccessPath, dwLen + 1);
#endif
						}
					else
						pItemArray->szAccessPath = NULL;

					// Next process the item ID string the same way.  This string is 
					// required so we don't expect its length to be zero:
					dwLen = lstrlen (pItem->GetItemID ());

					pItemArray->szItemID = (WCHAR *) CoTaskMemAlloc ((dwLen + 1) * sizeof (WCHAR));

#ifdef _UNICODE
					lstrcpyn (pItemArray->szItemID, pItem->GetItemID (), dwLen + 1);
#else
					MultiByteToWideChar (CP_ACP, 0, pItem->GetItemID (), -1, pItemArray->szItemID, dwLen + 1);
#endif

					// Load other item properties into item defintion structure:
					pItemArray->bActive = pItem->IsActive ();	// active state
					pItemArray->hClient = (OPCHANDLE) pItem;	// client handle to item
					pItemArray->dwBlobSize = 0;					// no blob support
					pItemArray->pBlob = NULL;
					pItemArray->vtRequestedDataType = pItem->GetDataType (); // requested data type

					// Issue request to add item:
					hr = m_pIItemMgt->AddItems (1, pItemArray, &pResults, &pErrors);

					// If request succeeded, we will still need to look at errors,
					// and free memory, and update our item object:
					if (SUCCEEDED (hr))
						{
						// If there were no error, update the item object:
						if (pErrors && SUCCEEDED (pErrors [0]))
							{
							// Set valid state:
							pItem->SetValid (TRUE);

							// Update item (we now know the server handle to OPC
							// item associated with our CKItem object.  Server may
							// have added the item, but not accepted our requested 
							// data type or access rights.  Update item object with
							// properties server returned:
							pItem->SetServerHandle (pResults->hServer);
							pItem->SetDataType (pResults->vtCanonicalDataType);
							pItem->SetAccessRights (pResults->dwAccessRights);
							}

						// COM places the responsibility of freeing the result
						// and errors memory on us.  These will have been allocated
						// by the server only if request succeeded:
						if (pResults)
							CoTaskMemFree (pResults);

						if (pErrors)
							CoTaskMemFree (pErrors);
						}

					// Free the string memory we allocated:
					if (pItemArray->szAccessPath)
						CoTaskMemFree (pItemArray->szAccessPath);

					if (pItemArray->szItemID)
						CoTaskMemFree (pItemArray->szItemID);

					// Free the item definition memory:
					CoTaskMemFree (pItemArray);
					}
				}

			// Notify the view of the update:
			CKMainWnd *pWnd = (CKMainWnd *) AfxGetMainWnd ();
			
			if (pWnd)
				pWnd->PostMessage (UM_ITEM_READD, 0, (LPARAM) pItem);
			}
		
		catch (...)
			{
			m_pIItemMgt = NULL;
			}
		}
	}

// **************************************************************************
// OnOK ()
//
// Description:
//	OK button event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnOK () 
	{
	// Apply current changes:
	OnApply ();

	// Perform default processing:
	CDialog::OnOK ();
	}

// **************************************************************************
// OnActive ()
//
// Description:
//	Active check box event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnActive () 
	{
	// We won't be able to reset OPC item's active state unless we have
	// a pointer to the item management interface:
	if (m_pIItemMgt)
		{
		// Get pointer to selected item object:
		CKItem *pItem = GetSelectedItem ();
		ASSERT (pItem != NULL);

		try
			{
			// Get OPC server's handle to associated OPC item:
			OPCHANDLE hServer = pItem->GetServerHandle ();
			HRESULT *pErrors= NULL;

			// Get the inverse of the current active state.  This will be
			// the state we try to set the OPC item:
			BOOL bActive = pItem->IsActive () ? FALSE : TRUE;

			// Try to set the state of the OPC item.
			if (SUCCEEDED (m_pIItemMgt->SetActiveState (1, &hServer, bActive, &pErrors)))
				{
				// Succeeded in changing state of OPC item.  Update the state of
				// our CKItem object to reflect this:
				pItem->SetActive (bActive);

				// It is up to us to free the memory the server allocated
				// for the errors array:
				if (pErrors)
					CoTaskMemFree (pErrors);

				// Set our modified flag:
				m_bModified = true;
				}
			else
				{
				// On failure revert back to current settings.
				// Reset active state member variable associated control:
				m_bActive = pItem->IsActive ();
				UpdateData (FALSE);
				}
			}
		
		catch (...)
			{
			m_pIItemMgt = NULL;

			// On failure revert back to current settings.
			// Reset active state member variable associated control:
			m_bActive = pItem->IsActive ();
			UpdateData (FALSE);
			}
		}
	}

// **************************************************************************
// OnChangeDataType ()
//
// Description:
//	Data type combo box change event handler.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::OnChangeDataType () 
	{
	// We won't be able to reset OPC item's data type unless we have a
	// pointer to the item management interface:
	if (m_pIItemMgt)
		{
		// Get pointer to selected item:
		CKItem *pItem = GetSelectedItem ();
		ASSERT (pItem != NULL);

		try
			{
			// Update member variables associated with controls:
			UpdateData (TRUE);

			// If data type has changed, try to update the associated
			// OPC item:
			if (m_vtDataType != pItem->GetDataType ())
				{
				// Get OPC server's handle to associated OPC item:
				OPCHANDLE hServer = pItem->GetServerHandle ();
				HRESULT *pErrors= NULL;

				// Create a variate to pass request data type to server with:
				VARTYPE vtType = m_vtDataType;

				// Try to reset the data type:
				if (m_pIItemMgt->SetDatatypes (1, &hServer, &vtType, &pErrors) == S_OK)
					{
					// Succeeded to change the data type.  Update our CKItem
					// object's data type to reflect this:
					pItem->SetDataType (vtType);

					// It is up to us to free the memory the server allocated
					// for the errors array:
					if (pErrors)
						CoTaskMemFree (pErrors);

					// Set our modified flag:
					m_bModified = true;
					}
				else
					{
					// On failure revert back to current settings.

					// Construct an error message with invalid data type:
					CString strType;
					StringFromVartype (vtType, strType);

					CString strErr;
					strErr.Format (IDS_FAILED_TO_SETDATATYPE, strType);

					// Show error message in a message bos:
					AfxMessageBox (strErr);

					// Reset data type member variable associated control:
					m_vtDataType = pItem->GetDataType ();
					UpdateData (FALSE);
					}
				}
			}
		
		catch (...)
			{
			m_pIItemMgt = NULL;

			// On failure revert back to current settings.
			// Reset data type member variable associated control:
			m_vtDataType = pItem->GetDataType ();
			UpdateData (FALSE);
			}
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKItemPropertiesDlg helpers
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// GetSelectedItem ()
//
// Description:
//	Called to return a pointer to selected item object.
//
// Parameters:
//  none
//
// Returns:
//  CKItem* - Pointer to selected item object.
// **************************************************************************
CKItem* CKItemPropertiesDlg::GetSelectedItem ()
	{
	// Get pointer to CKItem object for object array.  It's index will
	// be the same as index of selected item in list control.
	return ((CKItem *) m_pItemList->GetAt (m_nSelIndex));
	}

// **************************************************************************
// SelectItem ()
//
// Description:
//	Called to select an item.
//
// Parameters:
//  int			nIndex		Index of item to select.
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::SelectItem (int nIndex)
	{
	// Apply changes to currently selected item:
	OnApply ();

	// Select the new item:
	CKItem *pItem = NULL;

	m_nSelIndex = nIndex;
	ASSERT (m_nSelIndex <= m_cnItems);

	// Item list object array gives us a pointer to the CKItem object 
	// assocated with the selection:
	pItem = (CKItem *) m_pItemList->GetAt (m_nSelIndex);
	ASSERT (pItem != NULL);

	// Update the member variables associated with controls:
	m_strAccessPath = pItem->GetAccessPath ();
	m_bActive = pItem->IsActive ();
	m_vtDataType = pItem->GetDataType ();
	m_strItemID = pItem->GetItemID ();

	// Update control status:
	UpdateStatus ();
	}

// **************************************************************************
// UpdateStatus ()
//
// Description:
//	Update the status of controls attached to this dialog based on current
//	properties of selected item.
//	
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::UpdateStatus ()
	{
	// Create a wait cursor object.  This will cause the wait cursor, 
	// usually an hourglass, to be displayed.  When this object goes
	// out of scope, its destructor will restore the previous cursor
	// type.
	CWaitCursor wc;

	// Save handle of item with focus.  We will have to change the focus
	// if this item is disabled:
	HWND hWnd = ::GetFocus ();

	// Enable next button if we are not at the end of the list:
	m_cNext.EnableWindow (m_nSelIndex < m_cnItems - 1);

	// Enable the previous button if we are not at the beginning of the list:
	m_cPrev.EnableWindow (m_nSelIndex > 0);

	// Allow the user to modify item ID and access path only if item is invalid:
	CKItem *pItem = GetSelectedItem ();
	ASSERT (pItem != NULL);

	GetDlgItem (IDC_ITEMID)->EnableWindow (!pItem->IsValid ());
	GetDlgItem (IDC_ACCESSPATH)->EnableWindow (!pItem->IsValid ());

	// Load appropriate datatypes (based on current data type and
	// item valid state):

	// First get pointer to data type combo box:
	CComboBox *pCombo = (CComboBox *)GetDlgItem (IDC_DATATYPE);

	// then reset it:
	pCombo->ResetContent ();

	// then load new entries:
	if (pItem->IsValid ())
		{
		// If current data type is an array type:
		if (pItem->GetDataType () & VT_ARRAY)
			{
			pCombo->AddString (_T("Byte Array"));
			pCombo->AddString (_T("Char Array"));
			pCombo->AddString (_T("Word Array"));
			pCombo->AddString (_T("Short Array"));
			pCombo->AddString (_T("DWord Array"));
			pCombo->AddString (_T("Long Array"));
			pCombo->AddString (_T("Float Array"));
			pCombo->AddString (_T("Double Array"));
			}

		// else if it is not an array type"
		else
			{
			pCombo->AddString (_T("Boolean"));
			pCombo->AddString (_T("Byte"));
			pCombo->AddString (_T("Char"));
			pCombo->AddString (_T("Word"));
			pCombo->AddString (_T("Short"));
			pCombo->AddString (_T("DWord"));
			pCombo->AddString (_T("Long"));
			pCombo->AddString (_T("Float"));
			pCombo->AddString (_T("Double"));
			pCombo->AddString (_T("String"));
			}
		}

	// If item is not valid (item has not be re-added), allow all data types:
	else
		{
		pCombo->AddString (_T("Native"));
		pCombo->AddString (_T("Boolean"));
		pCombo->AddString (_T("Byte"));
		pCombo->AddString (_T("Byte Array"));
		pCombo->AddString (_T("Char"));
		pCombo->AddString (_T("Char Array"));
		pCombo->AddString (_T("Word"));
		pCombo->AddString (_T("Word Array"));
		pCombo->AddString (_T("Short"));
		pCombo->AddString (_T("Short Array"));
		pCombo->AddString (_T("DWord"));
		pCombo->AddString (_T("DWord Array"));
		pCombo->AddString (_T("Long"));
		pCombo->AddString (_T("Long Array"));
		pCombo->AddString (_T("Float"));
		pCombo->AddString (_T("Float Array"));
		pCombo->AddString (_T("Double"));
		pCombo->AddString (_T("Double Array"));
		pCombo->AddString (_T("String"));
		}

	// Fill available 2.0 item properties (We won't be able to do this
	// unless we have a pointer to the item properties interface.)
	if (m_pIItemProps != NULL)
		{
		TCHAR szBuffer [DEFBUFFSIZE];
		WCHAR *pszItemID		= NULL;
		DWORD dwCount			= 0;
		DWORD dwIndex			= 0;		
		DWORD *pdwIDs			= NULL;
		LPWSTR *pszDescriptions	= NULL;
		LPWSTR *pszLookupIDs	= NULL;
		VARTYPE *pvtDataTypes	= NULL;
		VARIANT *pvtValues		= NULL;
		HRESULT *pValErrs		= NULL;
		HRESULT *pLookupErrs	= NULL;
		HRESULT hr				= E_FAIL;

		// Get pointer to list control:
		CListCtrl *pList = (CListCtrl *) GetDlgItem (IDC_LIST);
		ASSERT (pList != NULL);

		// Delete any list items that are present:
		pList->DeleteAllItems ();

		// Obtain the qualified ID:
		ASSERT (!m_strItemID.IsEmpty ());

		CString strQualifiedID;

		if (!m_strAccessPath.IsEmpty () && m_pServer->IsKepServerEx ())
			{
			strQualifiedID = m_strAccessPath;
			strQualifiedID += _T(".");
			strQualifiedID += m_strItemID;
			}
		else
			strQualifiedID = m_strItemID;

		// Allocate storage for item ID string:
		pszItemID = (WCHAR *) CoTaskMemAlloc ((strQualifiedID.GetLength () + 1) * sizeof (WCHAR));

		// Convert item ID to UNICODE if needed and copy to allocated memory:
#ifdef _UNICODE
		lstrcpyn (pszItemID, strQualifiedID, strQualifiedID.GetLength () + 1);
#else
		_mbstowcsz (pszItemID, strQualifiedID, strQualifiedID.GetLength () + 1);
#endif
		
		// Query available properties:
		hr = m_pIItemProps->QueryAvailableProperties (
			  pszItemID, &dwCount, &pdwIDs, &pszDescriptions, &pvtDataTypes);

		if (SUCCEEDED (hr))
			{
			// Get the current property values for the property ID's we 
			// just queried:
			hr = m_pIItemProps->GetItemProperties (pszItemID, dwCount, 
				pdwIDs, &pvtValues, &pValErrs);

			if (SUCCEEDED (hr))
				{
				// Get lookup item IDs for the property ID's we just queried:
				hr = m_pIItemProps->LookupItemIDs (pszItemID, dwCount, 
					pdwIDs, &pszLookupIDs, &pLookupErrs);

				if (SUCCEEDED (hr))
					{
					// Insert item IDs, descriptions, values and lookup IDs 
					// into the list control:
					for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
						{
						// ID:
						_itot (pdwIDs [dwIndex], szBuffer, 10);
						pList->InsertItem (dwIndex, szBuffer);

						// Description (may have to convert from UNICODE):
#ifdef _UNICODE
						pList->SetItemText (dwIndex, 1, pszDescriptions [dwIndex]);
#else
						_wcstombsz (szBuffer, pszDescriptions [dwIndex], sizeof (szBuffer) / sizeof (TCHAR));
						pList->SetItemText (dwIndex, 1, szBuffer);
#endif						

						// {roperty value:
						if (pValErrs && SUCCEEDED (pValErrs [dwIndex]))
							GetValue (pvtValues [dwIndex], szBuffer, sizeof (szBuffer) / sizeof (TCHAR)); 
						else
							lstrcpyn (szBuffer, _T("???"), sizeof (szBuffer) / sizeof (TCHAR));
						
						pList->SetItemText (dwIndex, 2, szBuffer);

						// Lookup item ID (may have to convert from UNICODE):
						if (pLookupErrs && SUCCEEDED (pLookupErrs [dwIndex]))
							{
#ifdef _UNICODE
							lstrcpyn (szBuffer, pszLookupIDs [dwIndex], sizeof (szBuffer) / sizeof (TCHAR));
#else
							_wcstombsz (szBuffer, pszLookupIDs [dwIndex], sizeof (szBuffer) / sizeof (TCHAR));
#endif
							}
						else
							{
							// No lookup item ID, so place "N/A" in list control:
							lstrcpyn (szBuffer, _T("N/A"), sizeof (szBuffer) / sizeof (TCHAR));
							pszLookupIDs [dwIndex] = NULL;
							}
						
						pList->SetItemText (dwIndex, 3, szBuffer);
						}
					}
				}
			}

		// COM requires us to free memory for data passed back to us:
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
			{
			if (pszDescriptions && pszDescriptions [dwIndex]) 
				CoTaskMemFree (pszDescriptions [dwIndex]);

			if (pszLookupIDs && pszLookupIDs [dwIndex])
				CoTaskMemFree (pszLookupIDs [dwIndex]);

			// Clear variants:
			if (pvtValues)
				VariantClear (&pvtValues [dwIndex]);
			}

		if (pdwIDs)
			CoTaskMemFree (pdwIDs);

		if (pszDescriptions)
			CoTaskMemFree (pszDescriptions);

		if (pszLookupIDs)
			CoTaskMemFree (pszLookupIDs);

		if (pvtDataTypes)
			CoTaskMemFree (pvtDataTypes);

		if (pvtValues)
			CoTaskMemFree (pvtValues);
			
		if (pValErrs)
			CoTaskMemFree (pValErrs);

		if (pLookupErrs)
			CoTaskMemFree (pLookupErrs);
		}

	// Set the focus to the first control (data type) if the control that
	// previously had it became disabled:
	if (hWnd && !::IsWindowEnabled (hWnd))
		GetDlgItem (IDC_DATATYPE)->SetFocus ();

	// Transfer data to controls:
	UpdateData (false);
	}

// **************************************************************************
// GetValue ()
//
// Description:
//	Return a string representing the value of a variant.
//
// Parameters:
//  VARIANT		&vtVal		Input variant.
//	TCHAR		*pBuffer	Pointer to buffer for output string.
//	int			nBufLen		Size of output buffer.
//
// Returns:
//  void
// **************************************************************************
void CKItemPropertiesDlg::GetValue (VARIANT &vtVal,	// [in]	
									TCHAR *pBuffer, // [out]
									int nBufLen)	// [in]
	{
	ASSERT (pBuffer != NULL);
	ASSERT (nBufLen > 0);

	// Declare a CString to help construct result string:
	CString strVal;

	// Format string according to data type:
	switch (vtVal.vt)
		{
		case VT_BOOL:
			strVal.Format (_T("%d"), vtVal.boolVal);
			break;

		case VT_UI1:
			strVal.Format (_T("%u"), vtVal.bVal);
			break;

		case VT_I1:
			strVal.Format (_T("%d"), vtVal.cVal);
			break;

		case VT_UI2:
			strVal.Format (_T("%u"), vtVal.uiVal);
			break;

		case VT_I2:
			strVal.Format (_T("%d"), vtVal.iVal);
			break;

		case VT_UI4:
			strVal.Format (_T("%u"), vtVal.ulVal);
			break;

		case VT_I4:
			strVal.Format (_T("%d"), vtVal.lVal);
			break;

		case VT_R4:
			strVal.Format (_T("%G"), vtVal.fltVal);
			break;

		case VT_R8:
			strVal.Format (_T("%G"), vtVal.dblVal);
			break;

		case VT_BSTR:
			strVal = vtVal.bstrVal;
			break;

		case VT_DATE:
			{
			bool bSuccess = false;

			// Cariant time to system time (UTC):
			SYSTEMTIME systime;
			if (VariantTimeToSystemTime (vtVal.dblVal, &systime))
				{
				// Get time zone information:
				TIME_ZONE_INFORMATION tTZI;
				if (GetTimeZoneInformation (&tTZI) != TIME_ZONE_ID_INVALID)
					{
					// Localize system time:
					SYSTEMTIME systimelocal;
					if (SystemTimeToTzSpecificLocalTime (&tTZI, &systime, &systimelocal))
						{
						strVal.Format (_T("%02d:%02d:%02d"), 
							systimelocal.wHour, systimelocal.wMinute, systimelocal.wSecond);			

						bSuccess = true;
						}
					}
				}
			
			if (!bSuccess)
				strVal = _T("???");
			}
			break;

		case VT_UI1	| VT_ARRAY:
		case VT_I1	| VT_ARRAY:
		case VT_UI2	| VT_ARRAY:
		case VT_I2	| VT_ARRAY:
		case VT_UI4	| VT_ARRAY:
		case VT_I4	| VT_ARRAY:
		case VT_R4	| VT_ARRAY:
		case VT_R8	| VT_ARRAY:
			{
			CSafeArray *pSafeArr = (CSafeArray *) vtVal.parray;
			DWORD dwCols = pSafeArr->GetNumCols ();
			DWORD dwSize = pSafeArr->GetByteLength ();
			ULONG cbElements = pSafeArr->cbElements;
			LPBYTE lpByte = (LPBYTE)pSafeArr->pvData;
			DWORD dwCol = 0;

			// Start row delimiter:
			strVal = _T("[ ");

			// Cycle through the elements:
			for (DWORD i = 0; i < dwSize; i += cbElements, lpByte += cbElements)
				{
				TCHAR szNum[32];

				// Format element according to data size:
				if (cbElements == 1)
					{
					if (vtVal.vt ==	(VT_UI1 | VT_ARRAY))
						_stprintf (szNum, _T("%u"), *lpByte);
					else
						_stprintf (szNum, _T("%d"), *(char *)lpByte);
					}
				
				else if (cbElements == 2)
					{
					if (vtVal.vt ==	(VT_UI2 | VT_ARRAY))
						_stprintf (szNum, _T("%u"), *(WORD *)lpByte);
					else 
						_stprintf (szNum, _T("%d"), *(short *)lpByte);
					}
				
				else if (cbElements == 4)
					{
					if (vtVal.vt ==	(VT_R4	| VT_ARRAY))
						_stprintf (szNum, _T("%G"), *(float *)lpByte);
					else if (vtVal.vt ==	(VT_UI4	| VT_ARRAY))
						_stprintf (szNum, _T("%u"), *(DWORD *)lpByte);
					else if (vtVal.vt ==	(VT_I4	| VT_ARRAY))
						_stprintf (szNum, _T("%d"), *(DWORD *)lpByte);
					}

				else if (cbElements == 8)
					_stprintf (szNum, _T("%G"), *(double *)lpByte);

				else
					{
					ASSERT (FALSE);
					}

				// Delimit each element within the row with a comma:
				if (dwCol != 0)
					strVal += _T(", ");

				// Append the formatted element data:
				strVal += szNum;

				// Terminate each row (except the last):
				if (++dwCol == dwCols)
					{
					if (i < dwSize - cbElements) 
						strVal += _T(" ] [ ");

					dwCol = 0;
					}
				}

			// End delimiter:
			strVal += _T(" ]");
			}
			break;

		default:
			// Unsupported datatype:
			strVal = _T ("<Bad VARTYPE>");
			break;
		}

	// Copy value to output buffer:
	lstrcpyn (pBuffer, strVal, nBufLen);
	}

