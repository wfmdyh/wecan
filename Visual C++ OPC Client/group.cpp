// **************************************************************************
// group.cpp
//
// Description:
//	Implements the CKGroup class.  An object of this class is associated with
//	each OPC Group we wish to exchange data with.
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
#include "group.h"
#include "server.h"
#include "item.h"
#include "advisesink.h"
#include "datasink20.h"

// Versions for serializing:
#define VERSION_1			1
#define CURRENT_VERSION		VERSION_1


/////////////////////////////////////////////////////////////////////////////
// Data for CSV import/export
/////////////////////////////////////////////////////////////////////////////

#define MAXHEADERLEN				64		// Maximum header length
#define MAXCSVIMPORTFIELDLENGTH		512		// Maximum field length
#define PARSEBUFFERSIZE				2048	// Size of CSV record parse buffer

// Enumerate Field type IDs:
enum CSVFIELD
	{
	CSV_ItemID = 0,	// Item ID
	CSV_AccessPath,	// Item Access Path
	CSV_DataType,	// Item Data Type
	CSV_Active,		// Item Active state
	
	// Place holder used only as a sentinel to determine the number of IDs
	CSV_FieldCount		
	};

// Define a field identifer structre.  There will be one of these for each
// column of the CSV file, as determined from the header record.  These will
// be connected in a linked list, allowing user to place columns in any order
// and omit columns.
typedef struct _tagCSVHEADERFIELD
	{
	TCHAR szText [MAXHEADERLEN];		// Column name Text
	CSVFIELD eID;						// Column field type ID
	struct _tagCSVHEADERFIELD *pNext;	// Link to next structure in list
	} CSVHEADERFIELD;

// Enumerate return values from ReadCSV ():
typedef enum _tagREADCSVRET
	{
	tFieldOK = 0,	// Parse was successfull
	tFieldOverflow,	// Field was too big to fit into buffer
	tEndOfRecord	// End of record encountered
	} READCSVRET;

// Declare an array that will contain a CSVHEADERFIELD structure for each 
// possible field type.  This master field list will be used to validate
// header fields.  It will be initialized in LoadMasterFieldList().
static CSVHEADERFIELD astMasterCSV [CSV_FieldCount];


/////////////////////////////////////////////////////////////////////////////
// Helper functions for CSV import
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// LoadMasterFieldList ()
//
// Description:
//	Initialize the master field list array.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
static void LoadMasterFieldList ()
	{
	// Declare a static flag that will prevent us from actually initializing
	// the array more than once, even if this function is called multiple times.
	static BOOL bMasterListLoaded = false;

	// If already loaded then no need to do it again...
	if (bMasterListLoaded)
		return;

	// Declare a scratch CString for loading string resources:
	CString strFieldName;

	// Loop over all possible field types (implied by sizeof astMasterCSV):
	for (int i = 0; i < sizeof (astMasterCSV) / sizeof (astMasterCSV [0]); i++)
		{
		// Load the field name string resources:
		switch (i)
			{
			case CSV_ItemID:
				strFieldName.LoadString (IDS_ITEMID);
				break;

			case CSV_AccessPath:
				strFieldName.LoadString (IDS_ACCESSPATH);
				break;

			case CSV_DataType:
				strFieldName.LoadString (IDS_DATATYPE);
				break;

			case CSV_Active:
				strFieldName.LoadString (IDS_ACTIVE);
				break;

			default:
				// Unexpected field type.  Programmer error.
				ASSERT (FALSE);
				continue;
			}
		
		// Transfer the text we loaded into text field (safely):
		lstrcpyn (astMasterCSV [i].szText, strFieldName, sizeof (astMasterCSV [i].szText) / sizeof (TCHAR));

		// Assign the field type ID:
		astMasterCSV [i].eID = (CSVFIELD)i;

		// Initialize the next pointer to NULL.  It will be set as the header
		// is processed in BuildFieldList():
		astMasterCSV [i].pNext = NULL;
		}

	// Set "loaded" flag so we don't waste time reloading:
	bMasterListLoaded = true;
	}

// **************************************************************************
// ReadCSV ()
//
// Description:
//	Parse a field from a CSV file record.  Called once per field in a given
//	record.  pnStart will be reset to end of current field for subsequent call.
//	Use master field list to know how many times per record to call this.
//
// Parameters:
//  LPCTSTR		szInBuff	Input buffer containing whole record string.
//	int			*pnStart	Character position to begin parse.  Will be reset
//							  to start of next field.
//	LPTSTR		szOutBuff	Output buffer for parsed field string.
//	int			nBuffSize	Size of output buffer.
//
// Returns:
//  READCSVRET - Type enumerated above.
// **************************************************************************
static READCSVRET _stdcall ReadCSV (LPCTSTR szInBuff, int *pnStart, LPTSTR szOutBuff, int nBuffSize)
	{
	TCHAR ch = _T('\r');
	BOOL bQuoted = FALSE;
	BOOL bOverflow = FALSE;
	int cnChars = 0;

	// If we are at the end of the record, marked by a NULL termimator,
	// then we parse another field.  Return the "end of record" code.
	if (!szInBuff [*pnStart])
		return (tEndOfRecord);

	// Continue to read characters until we hit the next comma or
	// an EOL (End Of Line - "\n") character:
	while (TRUE)
		{
		// Get the next character, and increment start position for next time:
		ch = szInBuff [(*pnStart)++];

		// If the character is NULL, we are at the end of the record and
		// therefore is nothing left to read.  Break out of the loop.
		if (!ch)
			break;

		// Trim leading whitespace, if any.  If current character is a space,
		// then continue.  This will force us to process next character without
		// saving current one (a space) in output buffer.
		if (!cnChars && _istspace (ch))
			continue;

		// If we make it here, then the current character is "a keeper".
		// Increment the character counter:
		++cnChars;

		// Quotes will bracket strings that may contain what we consider delimiters.
		if (ch == _T('"'))
			{
			// Toggle bQuoted flag.  If set TRUE, this is an opening quote, and
			// there had better be closing quote.  If there is not closing quote,
			// we will continue parsing, skipping over field dilimiters, until 
			// the output buffer is filled.
			bQuoted = !bQuoted;

			// Continue without saving delimiter in output buffer:
			continue;
			}

		// EOL and commas delimit the fields.  We don't count delimiters inside 
		// quotes. This is where we eventually want to break out of the loop.  
		if (!bQuoted)
			{
			if (ch == _T(',') || ch == _T('\n'))
				break;
			}

		// Add the current character to the output buffer as long a there
		// is room:
		if (nBuffSize > 1)
			{
			// There is room, so add the current character to the output 
			// buffer, and move output buffer pointer to next position.
			*szOutBuff++ = ch;

			// Decrement nBuffSize.  When it hits zero we know we have
			// filled the output buffer to capacity.
			--nBuffSize;
			}
		
		else
			{
			// There is no more room in the output buffer.  Set the bOverflow
			// flag, but don't break out of the loop.  We want to keep going
			// until we hit a field deleimiter.  This will allow us to parse
			// the next field, even though this one is too big to deal with.
			bOverflow = TRUE;
			}
		}

	// Make sure the output string is properly NULL terminated:
	*szOutBuff = 0;

	// If overflowed the output buffer, then return tFieldOverflow,
	// otherwise return tFieldOK:
	return (bOverflow ? tFieldOverflow : tFieldOK);
	}

// **************************************************************************
// BuildFieldList ()
//
// Description:
//	Users are free to omit certain fields and to order included fields any 
//	way they want.  To do this, this function must be called to interpret
//	the field identification record (header).  Here, we will build a linked 
//	list of CSVHEADERFIELD structures based on header by setting the "pNext"
//	members of the master field list elements.  This list defines what fields
//	we expect to see and their order.
//
//	This function will throw an exception if the header can not be parsed.
//  The caller must be prepared to handle these exceptions.
//
// Parameters:
//  LPCTSTR		lpszHeader	Pointer to header string.
//
// Returns:
//  CSVHEADERFIELD* - Head of linked list of CSVHEADERFIELD structures.
// **************************************************************************
static CSVHEADERFIELD *BuildFieldList (LPCTSTR lpszHeader)
	{
	CString str;
	TCHAR szBuff [MAXHEADERLEN];
	int nIndex = 0;
	int i;
	int nMaxFields;
	CSVHEADERFIELD *pHeader = NULL;
	CSVHEADERFIELD *pLast = NULL;
	CSVHEADERFIELD *pField;

	// Make sure the master field list is initialized.  It will do the actual
	// loading on the first call only, so don't worry about the apparent redundancy,
	// or the "pNext" members getting reset.
	LoadMasterFieldList ();

	// The maximum number of fields we should expected is implied by the
	// sizeof astMasterCSV:
	nMaxFields = sizeof (astMasterCSV) / sizeof (astMasterCSV [0]);

	// Clear any existing pointer references from the master list:
	for (i = 0; i < nMaxFields; i++)
		astMasterCSV [i].pNext = NULL;

	// Initialize our "done" flag:
	bool bDone = false;

	// Process fields in header record until we decide we are done:
	while (!bDone)
		{
		// Parse out a field:
		READCSVRET eRC = ReadCSV (lpszHeader, &nIndex, szBuff, _countof (szBuff));

		// Check return code of our field parse function.  If there was a problem,
		// throw an exception.  If not, add a link to out field list.
		switch (eRC)
			{
			case tFieldOverflow:
				// The supplied buffer was not big enough to hold the field contents.
				// The header must be considered invalid - issue an error message and
				// throw an exception.

				// "Error importing CSV item data.\n\nField buffer overflow reading identification record."
				AfxMessageBox (IDS_IDFIELDBUFFEROVERFLOW);
				throw (-1);

			case tFieldOK:
				// We got something.  See if we can match a field type name.

				// Clear the field pointer:
				pField = NULL;

				// Search the master field list for a match of the field name:
				for (i = 0; i < nMaxFields; i++)
					{
					// Compare name in master filed list element with parsed string:
					if (!lstrcmpi (szBuff, astMasterCSV [i].szText))
						{
						// A match was found.  Point this field to its match in
						// the master field list:
						pField = &astMasterCSV [i];
						break;
						}
					}

				// If pField is still NULL, the we must not have found a matching
				// name in master field list.  Thus, the header must be considered
				// invalid - issue an error message and throw an exception.
				if (!pField)
					{
					// "Error importing CSV item data.\n\nUnrecognized field name: '%s'"
					str.Format (IDS_UNRECOGNIZEDFIELDNAME, szBuff);
					AfxMessageBox (str);
					throw (-1);
					}

				// If pNext has been set, then we know this field has already been 
				// added.  If the pField is the same as pLast then we also know the 
				// field has aleady been added.  If fields are duplicated, we must
				// consider the header invalid - issue an error message and throw
				// an exception.
				if (pField->pNext || pField == pLast)
					{
					// "Error importing item database.\n\nDuplicate field name: '%s'"
					str.Format (IDS_DUPLICATEFIELDNAME, szBuff);
					AfxMessageBox (str);
					throw (-1);
					}

				// If we make it here, we know the field is "a keeper".  Add it
				// to the end of the linked list.
				if (!pHeader)
					pHeader = pField;
				else
					pLast->pNext = pField;

				pLast = pField;
				break;

			case tEndOfRecord:
				// End of record was encountered, so we're done.
				bDone = true;
				break;

			default:
				// Unexpected return code.  Programmer error.
				ASSERT (FALSE);
				break;
			}
		}

	// If we processed the whole header record, and didn't add anyting to the
	// linked list (i.e. pHeader is still NULL), then we must consider the
	// header invalid - issue an error message and throw an exception.
	if (!pHeader)
		{
		// "Error importing item database.\n\nMissing item field identification record."
		AfxMessageBox (IDS_MISSINGIDRECORD);
		throw (-1);
		}

	// If we make it here, then everything looks good.  Return a pointer
	// to the head of the field linked list.
	return (pHeader);
	}

// **************************************************************************
// CSVToItem ()
//
// Description:
//	Set item property according to a field parsed from CSV record.
//
// Parameters:
//  CSVFIELD	eID			Field type ID.
//	LPCTSTR		szFieldText	Property value parsed from record.
//	CKItem		*pItem		Item to modify.
//
// Returns:
//  void
// **************************************************************************
static void _stdcall CSVToItem (CSVFIELD eID, LPCTSTR szFieldText, CKItem *pItem)
	{
	// If the field is empty (i.e. it starts with a NULL terminator), then
	// there is nothing to do.
	if (!szFieldText [0])
		return;

	// Selecte the item "Set" function based on given field type ID:
	switch (eID)
		{
		case CSV_ItemID:
			// Item ID:
			pItem->SetItemID (szFieldText);
			break;

		case CSV_AccessPath:
			// Item Access Path:
			pItem->SetAccessPath (szFieldText);
			break;

		case CSV_DataType:
			// Item Data Type.  (Need to convert string to variant type.)
			pItem->SetDataType (VartypeFromString (szFieldText));
			break;

		case CSV_Active:
			// Item Active state.  (Need to convert string to bool type.)
			if (lstrcmpi (szFieldText, _T("TRUE")) == 0)
				pItem->SetActive (true);
			else
				pItem->SetActive (false);
			break;
		
		default:
			// Unexpected field type ID.  Programmer error.
			ASSERT (FALSE);
			break;
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroup construction/destruction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKGroup ()
//
// Description:
//	Constructor.
//
// Parameters:
//  CKServer	*pParent	Pointer to parent server object
//
// Returns:
//  none
// **************************************************************************
CKGroup::CKGroup (CKServer *pParent)
	{
	ASSERT (pParent != NULL);
	m_pServer = pParent;

	// Initialize member variables.

	// OPC Group properties:
	m_strName			= GROUP_DEFAULT_NAME;
	m_dwUpdateRate		= GROUP_DEFAULT_UPDATERATE;	
	m_dwLanguageID		= GROUP_DEFAULT_LANGUAGEID;	
	m_lBias				= GROUP_DEFAULT_TIMEBIAS;
	m_fDeadband			= GROUP_DEFAULT_DEADBAND;
	m_bActive			= GROUP_DEFAULT_ACTIVESTATE;
	m_dwUpdateMethod	= GROUP_DEFAULT_UPDATEMETHOD;

	m_hServer		= NULL;
	m_bValid		= FALSE;

	// Flags:
	ZeroMemory (&m_bfFlags, sizeof (m_bfFlags));

	// Item management:
	m_pPrev = NULL;
	m_pNext = NULL;

	m_pItemHead = NULL;
	m_cdwItems = 0;

	// GUI management:
	m_hTreeItem = NULL;

	// Interface pointers we will query and maintain:
	m_pIGroupState			= NULL;
	m_pIPublicGroupState	= NULL;
	m_pIItemMgt				= NULL;
	m_pISync				= NULL;
	m_pIAsync				= NULL;
	m_pIDataObject			= NULL;
	m_pIAsync2				= NULL;		
	m_pIConnPtContainer		= NULL;

	m_pIDataSink20			= NULL;
	m_dwCookieDataSink20	= 0;

	m_pIAdviseSink			= NULL;
	m_dwCookieRead			= 0;
	m_dwCookieWrite			= 0;
	}

// **************************************************************************
// // ~CKGroup ()
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
CKGroup::~CKGroup ()
	{
	// Assert that all interfaces and items have been properly released:
	ASSERT (m_cdwItems == 0);

	ASSERT (m_pIGroupState == NULL);
	ASSERT (m_pIPublicGroupState == NULL);
	ASSERT (m_pIItemMgt == NULL);
	ASSERT (m_pISync == NULL);
	ASSERT (m_pIAsync == NULL);
	ASSERT (m_pIDataObject == NULL);
	ASSERT (m_pIAsync2 == NULL);
	ASSERT (m_pIConnPtContainer == NULL);
	
	ASSERT (m_pIDataSink20 == NULL);
	ASSERT (m_dwCookieDataSink20 == 0);

	ASSERT (m_pIAdviseSink == NULL);
	ASSERT (m_dwCookieRead == 0);
	ASSERT (m_dwCookieWrite == 0);
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroup item management
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// AddItemToList ()
//
// Description:
//	Add an item to this group.
//	
// Parameters:
//  CKItem		*pItem		Pointer to item object to add.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::AddItemToList (CKItem *pItem)
	{
	// New items are added to the head of the linked list.

	// That means new the items's "next" item is old head of list,
	pItem->SetNext (m_pItemHead);

	// and the new item is the old head of list's "previous" item,
	if (m_pItemHead)	
		m_pItemHead->SetPrev (pItem);
		
	// and that new item is now the new "head" of list.
	m_pItemHead = pItem;

	// Don't forget to bump up our local item count,
	++m_cdwItems;

	// and the global item count.
	UpdateItemCount (+1);
	}

// **************************************************************************
// RemoveItemFromList ()
//
// Description:
//	Remove an item from this group.
//
// Parameters:
//  CKItem		*pItem		Pointer to item object to remove.  If NULL, all
//							  items will be removed.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::RemoveItemFromList (CKItem *pItem)
	{
	// Remove all items:
	if (pItem == NULL)
		{
		// Start with head of linked list and work our way up:
		pItem = m_pItemHead;

		// Loop over items until we hit the end of the list (where
		// pItem will be NULL).
		while (pItem)
			{
			// Must save the next item before we delete item.  The next
			// item will become the new head of list.
			m_pItemHead = pItem->GetNext ();

			// Delete item:
			delete pItem;

			// Process next item (the new head of list):
			pItem = m_pItemHead;
			}

		// Update global item count,
		UpdateItemCount (-(int)m_cdwItems);

		// and reset our local count,
		m_cdwItems = 0;
		}

	// Remove a single item:
	else
		{
		// To remove a link, we must first get pointers to the adjacent links:
		CKItem *pPrev = pItem->GetPrev ();
		CKItem *pNext = pItem->GetNext ();
		
		// If there is a "previous" link, then its "next" is removed link's "next",
		if (pPrev)	
			pPrev->SetNext (pNext);

		// and if there is a "next" link, then its "previous" is removed link's "previous",
		if (pNext)
			pNext->SetPrev (pPrev);

		// and if removed link was the "head", then the new head is removed link's "next".
		if (pItem == m_pItemHead)
			m_pItemHead = pNext;

		// Decrement our local item count,
		--m_cdwItems;

		// and the global item count.
		UpdateItemCount (-1);
		}
	}

// **************************************************************************
// AddItems ()
//
// Description:
//	Called to add multiple OPC Items at one time.  A CKItem will be added to
//  this object if	needed and a request to add a corresponding OPC Item to
//  the OPC Group will be made.
//
// Parameters:
//  CObArray	&cItemList		Array of item objects to add.
//	DWORD		dwCount			Number of items in cItemList.
//	bool		bLoadingProject Set to true if call is made during project 
//								  load so item can be added to this object's
//								  imem list.  Otherwise we will assume item
//								  already exists in list, and only needs to
//								  added to OPC Group.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::AddItems (CObArray &cItemList, DWORD dwCount, bool bLoadingProject /* = false */)
	{
	CKItem *pItem = NULL;
	DWORD dwIndex = 0;

	ASSERT (dwCount > 0);

	// If the item is new add it to our list:
	if (!bLoadingProject)
		{
		// Add the list of items to our project whether we they are valid or not:
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
			{
			// Get pointer to item in input array:
			pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Add it to our list:
			AddItemToList (pItem);
			}
		}

	// If we are connected to an OPC Server, and have a pointer to its IOPCItemMgt
	// interface, then issue an "add items" request:
	if ((m_pServer->IsConnected ()) && (m_pIItemMgt != NULL))
		{
		HRESULT hr;
		OPCITEMDEF *pItemArray = NULL;
		OPCITEMRESULT *pResults = NULL;
		HRESULT *pErrors = NULL;
		DWORD dwLen;
		DWORD cdwSuccess = 0;

		// Allocate memory for item definition array:
		pItemArray = (OPCITEMDEF *) CoTaskMemAlloc (dwCount * sizeof (OPCITEMDEF));

		// Don't bother if memory allocation failed:
		if (pItemArray != NULL)
			{
			// Fill the item definition array:
			for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
				{
				// Get pointer to item in input array:
				pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// COM requires that all string be in wide character format.  The
				// access path and Item ID properties are strings, so we may have
				// to convert their format.

				// First get the length of access path string:
				dwLen = lstrlen (pItem->GetAccessPath ());

				if (dwLen)
					{
					// Allocate memory for string:
					pItemArray [dwIndex].szAccessPath = (WCHAR *) CoTaskMemAlloc ((dwLen + 1) * sizeof (WCHAR));

#ifdef _UNICODE
					// If Unicode build, string will already be in wide character
					// format, so copy into allocated memory as is:
					lstrcpyn (pItemArray [dwIndex].szAccessPath,  pItem->GetAccessPath (), dwLen + 1);
#else
					// If ANSI build, then string format needs to be converted.  Place 
					// result of conversion into allocated memory:
					MultiByteToWideChar (CP_ACP, 0, pItem->GetAccessPath (), -1, pItemArray [dwIndex].szAccessPath, dwLen + 1);
#endif
					}
				else
					{
					// Access path string length is zero, so set output to NULL: 
					pItemArray [dwIndex].szAccessPath = NULL;
					}

				// Multibyte to wide character conversion for Item ID:
				dwLen = lstrlen (pItem->GetItemID ()); // This can't be zero, so no test as above
				pItemArray [dwIndex].szItemID = (WCHAR *) CoTaskMemAlloc ((dwLen + 1) * sizeof (WCHAR));

#ifdef _UNICODE
				lstrcpyn (pItemArray [dwIndex].szItemID, pItem->GetItemID (), dwLen + 1);
#else
				MultiByteToWideChar (CP_ACP, 0, pItem->GetItemID (), -1, pItemArray [dwIndex].szItemID, dwLen + 1);
#endif

				// Set remaining structure members:
				// (If requested data type is NULL, the OPC Server should return
				// the default type.  The returned canonical data type may not be
				// the same as the requested data type.)
				pItemArray [dwIndex].bActive = pItem->IsActive ();	// active state
				pItemArray [dwIndex].hClient = (OPCHANDLE) pItem;	// our handle to item
				pItemArray [dwIndex].dwBlobSize = 0;				// no blob support
				pItemArray [dwIndex].pBlob = NULL;
				pItemArray [dwIndex].vtRequestedDataType = pItem->GetDataType (); // Requested data type
				}

			// Issue request add items request through IOPCItemMgt interface:
			hr = m_pIItemMgt->AddItems (
					dwCount,		// Item count
					pItemArray,		// Array of item definition structures
					&pResults,		// Result array
					&pErrors);		// Error array

			// Check results for each item:
			for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
				{
				// Get pointer to item in input array:
				pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// Everything looks good:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [dwIndex])))
					{
					// Set CKItem valid:
					pItem->SetValid (TRUE);

					// Update CKItem properties:
					// (Note, the returned canonical data type may not be the same
					// as the requested data type.)
					pItem->SetServerHandle (pResults [dwIndex].hServer);
					pItem->SetDataType (pResults [dwIndex].vtCanonicalDataType);
					pItem->SetAccessRights (pResults [dwIndex].dwAccessRights);

					// If the server passed back a blob, it is our responsibilty
					// to free it.  (Wrap with exception handler in case they were
					// bad and did not initialize the blob pointer.)
					try
						{
						if (pResults [dwIndex].pBlob != NULL)
							CoTaskMemFree (pResults [dwIndex].pBlob);
						}
					
					catch (...)
						{
						// Probably hit a bad pointer.  Try to process next item.
						TRACE (_T("OTC: AddItems exception thrown from freeing invalid blob.\r\n"));
						}

					// Bump success count:
					++cdwSuccess;
					}
				
				// There were errors:
				else
					{
					// Log error message:
					if (pErrors && FAILED (pErrors [dwIndex]))
						{
						// We have a good error array, so use it's HRESULT value.
						// (We could use GetErrorString() to get use friendly text if 
						// we wish.)
						LogMsg (IDS_GROUP_ADD_ITEMS_FAILED, pItem->GetItemID (), GetName (),
							pErrors [dwIndex]);
						}
					else
						{
						// Error array does not look good.  Use HRESULT value returned by add
						// item call.  (Again we could use the GetErrorString() if we wish.)
						LogMsg (IDS_GROUP_ADD_ITEMS_FAILED, pItem->GetItemID (), GetName (), hr);
						}

					// Set item invalid:
					pItem->SetValid (FALSE);

					// Use NULL server handle for invalid items:
					pItem->SetServerHandle (NULL);
					}

				// COM requires us to free memory allocated for [out] and [in/out]
				// arguments (i.e. access path and item id strings):
				if (pItemArray [dwIndex].szAccessPath)
					CoTaskMemFree (pItemArray [dwIndex].szAccessPath);

				if (pItemArray [dwIndex].szItemID)
					CoTaskMemFree (pItemArray [dwIndex].szItemID);
				}
			
			// COM requires us to free memory allocated for [out] and [in/out]
			// arguments (i.e. pResults, pErrors, and pItemArray).
			if (pResults)
				CoTaskMemFree (pResults);

			if (pErrors)
				CoTaskMemFree (pErrors);

			CoTaskMemFree (pItemArray);

			// Log success status:
			if (cdwSuccess > 0)
				LogMsg (IDS_GROUP_ADD_ITEMS, cdwSuccess, GetName ());
			}
		
		// Failed to allocate memory for item definition array:
		else
			{
			TRACE (_T("OTC: Unable to add items to OPC server %s. (failed to allocate OPCITEMDEF structure)\r\n"),
				m_pServer->GetProgID ());
			}
		}

	// Failed to add items because OPC is not connected or we
	// do not have pointer to IOPCItemMgt interface:
	else
		{
		// Log error message:
		if (!m_pServer->IsConnected ())
			LogMsg (IDS_GROUP_ADD_ITEMS_FAILED_NOCONNECTION, GetName ());
		else
			LogMsg (IDS_GROUP_ADD_ITEMS_FAILED_NOINTERFACE, GetName ());
		}
	}

// **************************************************************************
// RemoveItems ()
//
// Description:
//	Remove OPC Items from associated OPC Group and delete associated CKItems
//	if asked.
//
// Parameters:
//  CObArray	&cItemList		Array of items to remove.
//	DWORD		dwCount			Number of items in dwCount
//	bool		bDelete			Set to true to delete CKItem objects.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::RemoveItems (CObArray &cItemList, DWORD dwCount, bool bDelete /* = true */)
	{
	DWORD dwIndex = 0;
	CKItem *pItem = NULL;

	// We can only remove OPC Items if we are connected:
	if (m_pServer->IsConnected ())
		{
		OPCHANDLE *phServer		= NULL;
		HRESULT *pErrors		= NULL;
		HRESULT hr				= E_FAIL;
		DWORD cdwItemsRemoved	= 0;

		ASSERT (m_pIItemMgt != NULL);
		ASSERT (dwCount > 0);

		// Allocate memory for IOPCServer::RemoveItems() [in] arguments:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (dwCount * sizeof (OPCHANDLE));

		// We can't do anything if allocation failed:
		if (phServer == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Get the server handles associated with each item in input list:
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
			{
			// Get pointer to item in input list:
			pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Set server handle in request argument:
			phServer [dwIndex] = pItem->GetServerHandle ();
			}
		
		// Issue the remove items request through the IOPCItemMgt interface:
		hr = m_pIItemMgt->RemoveItems (
				dwCount,	// Item count
				phServer,	// Array of server handles for items
				&pErrors);	// Error array

		// Check error result and remove the items from the project:
		cdwItemsRemoved = dwCount;
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
			{
			// Get pointer to item in input list:
			pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Check for successful remove:
			if (FAILED (hr))
				{
				// Request failed.  
				TRACE (_T("OTC: Failed to remove item %s on %s\\\\%s. (%08X)\r\n"), 
					pItem->GetItemID (), m_pServer->GetProgID (), GetName (), hr);
				}
			else if (FAILED (pErrors [dwIndex]))
				{
				// Request succeeded, but this item could not be removed.

				// Decrement number of (valid) items removed:
				if (pItem->IsValid ())
					--cdwItemsRemoved;

				TRACE (_T("OTC: Failed to remove item %s on %s\\\\%s. (%08X)\r\n"), 
					pItem->GetItemID (), m_pServer->GetProgID (), GetName (), pErrors [dwIndex]);
				}

			// Delete the CKItems if asked:
			if (bDelete)
				{
				// Remove the item from our item list first:
				RemoveItemFromList (pItem);

				// Now it is safe to delete it:
				delete pItem;
				}
			else
				{
				// Since we didn't delete the item, we need to set it invalid
				// because there is no longer an assoicated OPC Item.
				pItem->SetValid (false);
				}
			}

		// log success or failure
		if (cdwItemsRemoved == dwCount)
			LogMsg (IDS_GROUP_REMOVE_ITEMS, cdwItemsRemoved, GetName ());
		else
			{
			ASSERT (dwCount >= cdwItemsRemoved);
			LogMsg (IDS_GROUP_REMOVE_ITEMS_FAILED, dwCount - cdwItemsRemoved, 
				dwCount, GetName ());
			}

		CoTaskMemFree (phServer);
		CoTaskMemFree (pErrors);
		}

	// If we are not connected, just delete the CKItems if asked:
	else if (bDelete)
		{
		// Loop over items in input array:
		for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
			{
			// Get a pointer to an item in the array:
			pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Remove the item from out list first:
			RemoveItemFromList (pItem);

			// Now it is safe to delete it:
			delete pItem;
			}
		}
	}

// **************************************************************************
// RemoveAllItems ()
//
// Description:
//	Remove all OPC Items from OPC Group, and delete all CKItems if asked.
//
// Parameters:
//  bool		bDelete			Set to true to delete CKItem objects.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::RemoveAllItems (bool bDelete /* = true */)
	{
	CKItem *pItem = NULL;
	HRESULT hr = E_FAIL;

	// If there are no items to remove, then return:
	if (m_cdwItems == 0)
		return;

	ASSERT (m_pItemHead);

	// Search for all valid items, i.e. items that were successfully
	// added to the OPC Server, and remove them.  If we don't have a
	// valid IOPCItemMgt interface pointer, then don't bother since 
	// call to RemoveItems() would fail.  There is no sence in building 
	// a list of items to remove if we won't be able to use it.
	if (m_pIItemMgt != NULL)
		{
		// Declare an object array to hold list of valid items:
		CObArray cItemList;
		DWORD cdwValidItems = 0;

		// Wrap our search with an exception handler in case we get a
		// bad pointer:
		try
			{
			// Allocate enough memory for object array to hold all items:
			cItemList.SetSize (m_cdwItems);

			// Start with head of linked list and work our way up the chain:
			pItem = m_pItemHead;

			// Keep looping until we hit the end of the linked list 
			// (when pItem is NULL):
			while (pItem)
				{
				// Check to see if item is valid:
				if (pItem->IsValid ())
					{
					// Item is valid.  Add it to out list of items to 
					// remove, and increment the item counter:
					cItemList.SetAt (cdwValidItems++, pItem);

					// Set the item invalid since it will no longer have
					// an associated OPC Item if remove request succeeds:
					pItem->SetValid (false);
					}
					
				// Get next item for next time around:
				pItem = pItem->GetNext ();
				}

			// Now remove the valid items (if any) from the OPC Server:
			if (cdwValidItems > 0)
				RemoveItems (cItemList, cdwValidItems, bDelete);
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			ASSERT (FALSE);
			TRACE (_T("OTC: RemoveAllItems memory exception thrown.\r\n"));
			}
		}

	// Any remaining items (those that were not valid):
	if (bDelete)
		RemoveItemFromList (NULL);
	}

// **************************************************************************
// SetItemActiveState ()
//
// Description:
//	Set active state of OPC Items.
//
// Parameters:
//  CObArray	&cItemList		Array of items to set state.
//	DWORD		cdwItems		Number of items in cItemList.
//	bool		bActive			Set to true to set items active.
//
// Returns:
//  bool - true if success
// **************************************************************************
bool CKGroup::SetItemActiveState (CObArray &cItemList, DWORD cdwItems, bool bActive)
	{
	ASSERT (cdwItems > 0);
	DWORD cdwSuccess = 0;

	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCItemMgt interface.
	if (m_pServer->IsConnected () && m_pIItemMgt)
		{
		DWORD dwIndex		= 0;
		CKItem *pItem		= NULL;
		OPCHANDLE *phServer	= NULL;
		HRESULT *pErrors	= NULL;
		HRESULT hr			= E_FAIL;

		// Allocate storage for server item handles:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));

		// Return if allocation fails:
		if (phServer == NULL)
			{
			ASSERT (FALSE);
			return (false);
			}

		// Fill request [in] arguments.
		for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Set server handle:
			phServer [dwIndex] = pItem->GetServerHandle ();
			}

		// Wrap our request processing with an exception handler in case
		// we get a bad pointer:
		try
			{
			// Issue the set state request using the the IOPCItemMgt interface:
			hr = m_pIItemMgt->SetActiveState (
					cdwItems,	// Item count
					phServer,	// Server handles for items
					bActive,	// Array of active states
					&pErrors);	// Error array

			// Check results for each item in input list:
			for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// If no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [dwIndex])))
					{
					// Success if we are changing the state of the item:
					if (pItem->IsActive () != (BOOL)bActive)
						{
						++cdwSuccess;

						// Update active state:
						pItem->SetActive (bActive);
						}
					}

				// Else errors:
				else
					{
					// Request succeeded, but item state was not changed:
					if (pErrors && FAILED (pErrors [dwIndex]))
						{
						// Issue error message with error code.  (We could use
						// GetErrorString() to get user friendly error text
						// if we wish.)
						LogMsg (IDS_SET_ACTIVE_STATE_FAILED, bActive ? 1 : 0,
							pItem->GetItemID (), GetName (), pErrors [dwIndex]);
						}

					// Request failed:
					else
						{
						// Issue error message with error code.  (We could use
						// GetErrorString() to get user friendly error text
						// if we wish.)
						LogMsg (IDS_SET_ACTIVE_STATE_FAILED, bActive ? 1 : 0,
							pItem->GetItemID (), GetName (), hr);
						}
					}
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.

			if (m_pServer->IsConnected ())	
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer and pErrors):
		CoTaskMemFree (phServer);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)		
			{
			LogMsg (IDS_SET_ACTIVE_STATE_SUCCESS, bActive ? 1 : 0, 
				cdwSuccess, GetName ());
			}
		}

	// Return true if success.  (cdwSuccess will be non-zero if success)
	return (cdwSuccess != 0);
	}

// **************************************************************************
// ReadSync ()
//
// Description:
//	Perform a synchronous read for specified items.
//	
// Parameters:
//	CObArray	&cItemList		Array of items to read.
//	DWORD		cdwItems		Number of items in cItemList.
//	bool		bDeviceRead		Set to true to perform device read, false
//								  for cache read.
//	bool		bPostMsg		Set to true to post successful read message.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::ReadSync (CObArray &cItemList, DWORD cdwItems, bool bDeviceRead, bool bPostMsg /* = true */)
	{
	ASSERT (cdwItems > 0);

	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCSyncIO interface.
	if (m_pServer->IsConnected () && m_pISync)
		{
		DWORD dwIndex			= 0;
		CKItem *pItem			= NULL;
		OPCDATASOURCE dwSource	= bDeviceRead ? OPC_DS_DEVICE : OPC_DS_CACHE;
		OPCHANDLE *phServer		= NULL;
		OPCITEMSTATE *pValues	= NULL;
		HRESULT *pErrors		= NULL;
		HRESULT hr				= E_FAIL;
		DWORD cdwSuccess		= 0;

		// Allocate storage for server item handles:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));

		// Return if allocation failed:
		if (phServer == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Set server handle:
			phServer [dwIndex] = pItem->GetServerHandle ();
			}

		// Wrap request processing in exception handler in case we get
		// a bad pointer:
		try
			{
			// Issue read request using IOPCSyncIO interface:
			hr = m_pISync->Read (
					dwSource,	// Source (device or cache)
					cdwItems,	// Item count
					phServer,	// Array of server handles for items
					&pValues,	// Array of values
					&pErrors);	// Array of errors

			// Check results:
			for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [dwIndex])))
					{
					// Increment the successful read counter:
					++cdwSuccess;
					}

				// Failure:
				else
					{
					// Request succeeded, but one or more items could not be read:
					if (pErrors && FAILED (pErrors [dwIndex]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (bDeviceRead ? IDS_SYNC_READDEVICE_FAILED : IDS_SYNC_READCACHE_FAILED,
							pItem->GetItemID (), GetName (), pErrors [dwIndex]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (bDeviceRead ? IDS_SYNC_READDEVICE_FAILED : IDS_SYNC_READCACHE_FAILED,
							pItem->GetItemID (), GetName (), hr);
						}
					}

				// Update item data:
				pItem->UpdateData (pValues [dwIndex].vDataValue, 
					pValues [dwIndex].wQuality, pValues [dwIndex].ftTimeStamp);

				// Should clear variant before we free it later:
				VariantClear (&pValues [dwIndex].vDataValue);
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: ReadSync exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, pValues, and pErrors):
		CoTaskMemFree (phServer);

		if (pValues)
			CoTaskMemFree (pValues);	

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status if we are asked to post messages:
		if (bPostMsg && cdwSuccess > 0)
			{
			// Log diferent message for read device and read cache:
			LogMsg (bDeviceRead ? IDS_SYNC_READDEVICE_SUCCESS : IDS_SYNC_READCACHE_SUCCESS, 
				cdwSuccess, GetName ());
			}
		}
	}

// **************************************************************************
// WriteSync ()
//
// Description:
//	Perform synchronous write for specified items.
//
// Parameters:
//	CObArray		&cItemList	Array of items to read.
//	CStringArray	&cValues	Array of values to write.
//	DWORD			cdwItems	Number of items in cItemList.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::WriteSync (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems)
	{
	ASSERT (cdwItems > 0);

	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCSyncIO interface.
	if (m_pServer->IsConnected () && m_pISync)
		{
		DWORD dwIndex		= 0;
		CKItem *pItem		= NULL;
		OPCHANDLE *phServer = NULL;
		VARIANT *pValues	= NULL;
		HRESULT *pErrors	= NULL;
		HRESULT hr			= E_FAIL;
		DWORD cdwSuccess	= 0;
		DWORD cdwSentItems  = 0;

		// Allocate storage for server item handles and write values
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));
		pValues = (VARIANT *) CoTaskMemAlloc (cdwItems * sizeof (VARIANT));

		// Return if allocation failed:
		if (phServer == NULL || pValues == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Load data for array type:
			if (pItem->GetDataType () & VT_ARRAY)
				{
				// Convert array values in string format to variant array.  If succeed,
				// set server handle:
				if (MapStringValToArrayVariant (cValues [cdwSentItems], pItem->GetValue (), &pValues [cdwSentItems]))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed to convert array values in string format to variant array:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}

			// Load data for non-array type:
			else
				{
				// Convert value in string format to variant.  If succeed, set server
				// handle:
				if (MapStringValToVariant (cValues [cdwSentItems], pValues [cdwSentItems], pItem->GetDataType ()))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed to convert value in string format to variant:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}
			}

		// Wrap request processing in execption handler in case we get
		// a bad pointer:
		try
			{
			// Issue read request using IOPCSyncIO interface:
			hr = m_pISync->Write (
					cdwSentItems,	// Item count
					phServer,		// Array of server handles for items
					pValues,		// Array of values
					&pErrors);		// Array of errors

			// Check results:
			for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];

				// Continue to next item if we invalidated current item above in
				// MapStringToVariant() failure:
				if (pItem == NULL)
					continue;

				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [cdwSentItems])))
					{
					// Increment the successful write counter:
					++cdwSuccess;
					}

				// Failure:
				else
					{
					// Request succeeded, but on or more items could not be written to:
					if (pErrors && FAILED (pErrors [cdwSentItems]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_SYNC_WRITE_FAILED, pItem->GetItemID (), GetName (),
							pErrors [cdwSentItems]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_SYNC_WRITE_FAILED, pItem->GetItemID (), GetName (), hr);
						}
					}
				
				// Should clear variant before we free it later:
				VariantClear (&pValues [cdwSentItems]);

				// Increment sent items counter:
				cdwSentItems++;
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: WriteSync exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, pValues, and pErrors):
		CoTaskMemFree (phServer);
		CoTaskMemFree (pValues);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)
			LogMsg (IDS_SYNC_WRITE_SUCCESS, cdwSuccess, GetName ());
		}
	}

// **************************************************************************
// ReadAsync10 ()
//
// Description:
//	Perform an asynchronous 1.0 read for specified items.
//
// Parameters:
//	CObArray	&cItemList		Array of items to read.
//	DWORD		cdwItems		Number of items in cItemList.
//	bool		bDeviceRead		Set to true to perform device read, false
//								  for cache read.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::ReadAsync10 (CObArray &cItemList, DWORD cdwItems, bool bDeviceRead)
	{
	ASSERT (cdwItems > 0);

	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO interface.
	if (m_pServer->IsConnected () && m_pIAsync)
		{
		ASSERT (m_dwCookieRead != 0);

		DWORD dwIndex			= 0;
		CKItem *pItem			= NULL;
		OPCDATASOURCE dwSource	= bDeviceRead ? OPC_DS_DEVICE : OPC_DS_CACHE;
		OPCHANDLE *phServer		= NULL;
		DWORD dwTransID			= 0;
		HRESULT *pErrors		= NULL;
		HRESULT hr				= E_FAIL;
		DWORD cdwSuccess		= 0;

		// Allocate storage for server item handles:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));

		// Return if allocation failed:
		if (phServer == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Set server handle:
			phServer [dwIndex] = pItem->GetServerHandle ();
			}

		// Wrap request processing in exception hander in case we get
		// a bad pointer:
		try
			{
			// Issue read request using IOPCAsyncIO interface:
			hr = m_pIAsync->Read (
					m_dwCookieRead,	// Cookie returned from DAdvise
					dwSource,		// Source (device of cache)
					cdwItems,		// Number of items requested
					phServer,		// Item server handles	
					&dwTransID,		// Server generated transaction id
					&pErrors);		// Item errors

			// Check results:
			for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);
			
				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [dwIndex])))
					{
					// Increment the successful read counter:
					++cdwSuccess;
					}
				
				// Failure:
				else
					{
					// Request succeeded, but one or more items could not be read:
					if (pErrors && FAILED (pErrors [dwIndex]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (bDeviceRead ? IDS_ASYNC10_READDEVICE_FAILED : IDS_ASYNC10_READCACHE_FAILED,
							pItem->GetItemID (), GetName (), pErrors [dwIndex]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (bDeviceRead ? IDS_ASYNC10_READDEVICE_FAILED : IDS_ASYNC10_READCACHE_FAILED,
							pItem->GetItemID (), GetName (), hr);
						}
					}
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: ReadAsyncSync10 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, and pErrors):
		CoTaskMemFree (phServer);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)
			{
			// Log diferent message for read device and read cache:
			LogMsg (bDeviceRead ? IDS_ASYNC10_READDEVICE_SUCCESS : IDS_ASYNC10_READCACHE_SUCCESS, 
				dwTransID, cdwSuccess, GetName ());
			}
		}
	}

// **************************************************************************
// RefreshAsync10 ()
//
// Description:
//	Perform an asynchronous 1.0 refresh.
//
// Parameters:
//	bool		bDeviceRead		Set to true to perform device refresh, false
//								  for cache refresh.	
//
// Returns:
//  void
// **************************************************************************
void CKGroup::RefreshAsync10 (bool bDeviceRead)
	{
	ASSERT (m_cdwItems > 0);

	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO interface.
	if (m_pServer->IsConnected () && m_pIAsync)
		{
		ASSERT (m_dwCookieRead != 0);
	
		OPCDATASOURCE dwSource	= bDeviceRead ? OPC_DS_DEVICE : OPC_DS_CACHE;
		DWORD dwTransID = 0;
		HRESULT hr = E_FAIL;

		// Wrap request processing in exception hander in case we get
		// a bad pointer - Not really needed in this case, but this keeps
		// same code structure as other operations:
		try
			{
			// Issue refresh request using IOPCAsyncIO interface:
			hr = m_pIAsync->Refresh (
					m_dwCookieRead,	// Cookie returned from DAdvise
					dwSource,		// Source (device or cache)
					&dwTransID);	// Server generated transaction id

			// Log results:
			if (SUCCEEDED (hr))
				{
				// Log request succeeded message.  Log different message for device and
				// cache refres.
				LogMsg (bDeviceRead ? IDS_ASYNC10_REFRESHDEVICE_SUCCESS : IDS_ASYNC10_REFRESHCACHE_SUCCESS, 
					dwTransID, GetName ());
				}
			else
				{
				// Log request faild messgae.  Log different message for device and 
				// cache refreshes:
				LogMsg (bDeviceRead ? IDS_ASYNC10_REFRESHDEVICE_FAILURE : IDS_ASYNC10_REFRESHCACHE_FAILURE, 
					GetName (), hr);
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: RefreshAsync10 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}
		}
	}

// **************************************************************************
// WriteAsync10 ()
//
// Description:
//	Perform an asynchronous 1.0 write for specified items.
//
// Parameters:
//	CObArray		&cItemList	Array of items to read.
//	CStringArray	&cValues	Array of values to write.
//	DWORD			cdwItems	Number of items in cItemList.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::WriteAsync10 (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems)
	{
	ASSERT (cdwItems > 0);
	
	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO interface.
	if (m_pServer->IsConnected () && m_pIAsync)
		{
		ASSERT (m_dwCookieWrite != 0);

		DWORD dwIndex		= 0;
		CKItem *pItem		= NULL;
		OPCHANDLE *phServer = NULL;
		VARIANT *pValues	= NULL;
		HRESULT *pErrors	= NULL;
		HRESULT hr			= E_FAIL;
		DWORD dwTransID		= 0;
		DWORD cdwSuccess	= 0;
		DWORD cdwSentItems  = 0;

		// Allocate storage for server item handles and write values:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));
		pValues = (VARIANT *) CoTaskMemAlloc (cdwItems * sizeof (VARIANT));

		// Return if allocation failed:
		if (phServer == NULL || pValues == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Load data for array type:
			if (pItem->GetDataType () & VT_ARRAY)
				{
				// Convert array values in string format to variant array.  If succeed,
				// set server handle:
				if (MapStringValToArrayVariant (cValues [cdwSentItems], pItem->GetValue (), &pValues [cdwSentItems]))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed to convert array values in string format to variant array:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}

			// Load data for non-array type:
			else
				{
				// Convert value in string format to variant.  If succeed, set server
				// handle:
				if (MapStringValToVariant (cValues [cdwSentItems], pValues [cdwSentItems], pItem->GetDataType ()))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed ot convert value in string format to variant:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}
			}
		
		// Wrap request processing in execption handler in case we get
		// a bad pointer:
		try
			{
			// Issue write request using IOPCAsyncIO interface:
			hr = m_pIAsync->Write (
					m_dwCookieWrite,	// DAdvise cookie returned by server for writes
					cdwSentItems,		// Number of items to write
					phServer,			// Server item handles
					pValues,			// Item data to write
					&dwTransID,			// [out] server assigned transaction ID for this request
					&pErrors);			// [out] error results

			// Check results:
			for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwSentItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				
				// Continue to next item if we invalidated current item above in
				// MapStringToVariant() failure:
				if (pItem == NULL)
					continue;

				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [cdwSentItems])))
					{
					// Increment the successful write counter:
					++cdwSuccess;
					}

				// Failure:
				else
					{
					// Request succeeded, but on or more items could not be written to:
					if (pErrors && FAILED (pErrors [cdwSentItems]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC10_WRITE_FAILED, pItem->GetItemID (), GetName (),
							pErrors [cdwSentItems]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC10_WRITE_FAILED, pItem->GetItemID (), GetName (), hr);
						}
					}

				// Increment sent items counter:
				cdwSentItems++;
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: WriteAsync10 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, pValues, and pErrors):
		CoTaskMemFree (phServer);
		CoTaskMemFree (pValues);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)
			LogMsg (IDS_ASYNC10_WRITE_SUCCESS, dwTransID, cdwSuccess, GetName ());
		}
	}

// **************************************************************************
// ReadAsync20 ()
//
// Description:
//	Perform an asynchronous 2.0 read for specified items.
//
// Parameters:
//	CObArray	&cItemList		Array of items to read.
//	DWORD		cdwItems		Number of items in cItemList.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::ReadAsync20 (CObArray &cItemList, DWORD cdwItems)
	{
	ASSERT (cdwItems > 0);
	
	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO2 interface.
	if (m_pServer->IsConnected () && m_pIAsync2)
		{
		ASSERT (m_dwCookieDataSink20 != 0);

		DWORD dwIndex			= 0;
		CKItem *pItem			= NULL;
		OPCHANDLE *phServer		= NULL;
		DWORD dwClientTransID	= GetTickCount ();	// client provided transaction for notifications (not useful right now)
		DWORD dwCancelTransID	= 0;
		HRESULT *pErrors		= NULL;
		HRESULT hr				= E_FAIL;
		DWORD cdwSuccess		= 0;

		// Allocate storage for server item handles:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));

		// Return if allocation failed:
		if (phServer == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Set server handle:
			phServer [dwIndex] = pItem->GetServerHandle ();
			}

		// Wrap request processing in exception hander in case we get
		// a bad pointer:
		try
			{
			// Issue read request using IOPCAsyncIO2 interface:
			hr = m_pIAsync2->Read 
					(cdwItems,			// Number of items requested
					phServer,			// Item server handles	
					dwClientTransID,	// Client generated transaction id received in callback
					&dwCancelTransID,	// Server generated transaction id used for canceling this xact
					&pErrors);

			// Check results:
			for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
				ASSERT (pItem != NULL);

				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [dwIndex])))
					{
					// Increment the successful read counter:
					++cdwSuccess;
					}

				// Failure:
				else
					{
					// Request succeeded, but one or more items could not be read:
					if (pErrors && FAILED (pErrors [dwIndex]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC20_READ_FAILED,
							pItem->GetItemID (), GetName (), pErrors [dwIndex]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC20_READ_FAILED,
							pItem->GetItemID (), GetName (), hr);
						}
					}
				}

			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: ReadAsync20 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, and pErrors):
		CoTaskMemFree (phServer);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)
			LogMsg (IDS_ASYNC20_READ_SUCCESS, dwClientTransID, cdwSuccess, GetName ());
		}
	}

// **************************************************************************
// RefreshAsync20 ()
//
// Description:
//	Perform an asynchronous 2.0 refresh.
//
// Parameters:
//	bool		bDeviceRead		Set to true to perform device refresh, false
//								  for cache refresh.	
//
// Returns:
//  void 
// **************************************************************************
void CKGroup::RefreshAsync20 (bool bDeviceRead)
	{
	ASSERT (m_cdwItems > 0);
	
	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO2 interface.
	if (m_pServer->IsConnected () && m_pIAsync2)
		{
		ASSERT (m_dwCookieDataSink20 != 0);

		OPCDATASOURCE dwSource	= bDeviceRead ? OPC_DS_DEVICE : OPC_DS_CACHE;
		DWORD dwCancelTransID		= 0;
		DWORD dwClientTransID		= GetTickCount ();	// client provided transaction for notifications (not useful right now)
		HRESULT hr = E_FAIL;

		// Wrap request processing in exception hander in case we get
		// a bad pointer - Not really needed in this case, but this keeps
		// same code structure as other operations:
		try
			{
			// Issue refresh request using IOPCAsyncIO2 interface:
			hr = m_pIAsync2->Refresh2 (
				dwSource,			// Source (device or cache)
				dwClientTransID,	// Client generated transaction id received in callback
				&dwCancelTransID);	// Server generated transaction id used for canceling this xact

			// Log results:
			if (SUCCEEDED (hr))
				{
				// Log request succeeded message.  Log different message for device and
				// cache refres.
				LogMsg (bDeviceRead ? IDS_ASYNC20_REFRESHDEVICE_SUCCESS : IDS_ASYNC20_REFRESHCACHE_SUCCESS, 
					dwClientTransID, GetName ());
				}
			else
				{
				// Log request faild messgae.  Log different message for device and 
				// cache refreshes:
				LogMsg (bDeviceRead ? IDS_ASYNC20_REFRESHDEVICE_FAILURE : IDS_ASYNC20_REFRESHCACHE_FAILURE, 
					GetName (), hr);
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: RefreshAsync20 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}
		}
	}

// **************************************************************************
// WriteAsync20 ()
//
// Description:
//	Perform an asynchronous 2.0 write for specified items.
//
// Parameters:
//	CObArray		&cItemList	Array of items to read.
//	CStringArray	&cValues	Array of values to write.
//	DWORD			cdwItems	Number of items in cItemList.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::WriteAsync20 (CObArray &cItemList, CStringArray &cValues, DWORD cdwItems)
	{
	ASSERT (cdwItems > 0);
	
	// There is no sence in building a request unless we are connected to
	// the OPC Server and have a pointer to the IOPCAsyncIO2 interface.
	if (m_pServer->IsConnected () && m_pIAsync2)
		{
		ASSERT (m_dwCookieDataSink20 != 0);

		DWORD dwIndex				= 0;
		CKItem *pItem				= NULL;
		OPCHANDLE *phServer			= NULL;
		VARIANT *pValues			= NULL;
		HRESULT *pErrors			= NULL;
		HRESULT hr					= E_FAIL;
		DWORD dwCancelTransID		= 0;
		DWORD cdwSuccess			= 0;
		DWORD dwClientTransID		= GetTickCount ();	// client provided transaction for notifications (not useful right now)
		DWORD cdwSentItems			= 0;
		
		// Allocate storage for server item handles and write values:
		phServer = (OPCHANDLE *) CoTaskMemAlloc (cdwItems * sizeof (OPCHANDLE));
		pValues = (VARIANT *) CoTaskMemAlloc (cdwItems * sizeof (VARIANT));

		// Return if allocation failed:
		if (phServer == NULL || pValues == NULL)
			{
			ASSERT (FALSE);
			return;
			}

		// Fill request [in] arguments:
		for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwItems; dwIndex++)
			{
			// Get pointer to item in input list:
			CKItem *pItem = (CKItem *) cItemList [dwIndex];
			ASSERT (pItem != NULL);

			// Load data for array type:
			if (pItem->GetDataType () & VT_ARRAY)
				{
				// Convert array values in string format to variant array.  If succeed,
				// set server handle:
				if (MapStringValToArrayVariant (cValues [cdwSentItems], pItem->GetValue (), &pValues [cdwSentItems]))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed to convert array values in string format to variant array:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}

			// Load data for non-array type:
			else
				{
				// Convert value in string format to variant.  If succeed, set server
				// handle:
				if (MapStringValToVariant (cValues [cdwSentItems], pValues [cdwSentItems], pItem->GetDataType ()))
					phServer [cdwSentItems++] = pItem->GetServerHandle ();
				else
					{
					// Failed ot convert value in string format to variant:
					LogMsg (IDS_WRITE_DATA_MAP_FAILED, pItem->GetItemID ());

					// Invalidate item:
					cItemList [dwIndex] = NULL;
					}
				}
			}
		
		// Wrap request processing in execption handler in case we get
		// a bad pointer:
		try
			{
			// Issue write request using IOPCAsyncIO2 interface:
			hr = m_pIAsync2->Write (
					cdwSentItems,		// Number of items to write
					phServer,			// Server item handles
					pValues,			// Item data to write
					dwClientTransID,	// Client generated transaction ID
					&dwCancelTransID,	// [out] server assigned transaction ID for this request (use if canceling transaction)
					&pErrors);			// [out] error results

			// Check results:
			for (dwIndex = 0, cdwSentItems = 0; dwIndex < cdwItems; dwIndex++)
				{
				// Get pointer to item in input list:
				CKItem *pItem = (CKItem *) cItemList [dwIndex];
						
				// Continue to next item if we invalidated current item above in
				// MapStringToVariant() failure:
				if (pItem == NULL)
					continue;

				// Request succeeded and there were no errors:
				if (SUCCEEDED (hr) && (pErrors && SUCCEEDED (pErrors [cdwSentItems])))
					{
					// Increment the successful write counter:
					++cdwSuccess;
					}

				// Failure:
				else
					{
					// Request succeeded, but on or more items could not be written to:
					if (pErrors && FAILED (pErrors [cdwSentItems]))
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC20_WRITE_FAILED, pItem->GetItemID (), GetName (),
							pErrors [cdwSentItems]);
						}

					// Request failed:
					else
						{
						// Log error message with error code.  (Could use GetErrorString()
						// to get user friendly error string if we wish.)
						LogMsg (IDS_ASYNC20_WRITE_FAILED, pItem->GetItemID (), GetName (), hr);
						}
					}

				// Increment sent items counter:
				cdwSentItems++;
				}
			}
		
		catch (...)
			{
			// Probably hit a bad pointer.  Try to process next item.
			TRACE (_T("OTC: WriteAsync20 exception handler invoked\r\n"));

			if (m_pServer->IsConnected ())
				{
				// We not disconnected so why was an exception thrown?
				ASSERT (FALSE);
				}
			}

		// COM requires us to free memory allocated for [out] and [in/out]
		// arguments (i.e. phServer, pValues, and pErrors):
		CoTaskMemFree (phServer);
		CoTaskMemFree (pValues);

		if (pErrors)
			CoTaskMemFree (pErrors);

		// Log success status:
		if (cdwSuccess > 0)
			LogMsg (IDS_ASYNC20_WRITE_SUCCESS, dwClientTransID, cdwSuccess, GetName ());
		}
	}

// **************************************************************************
// Clone ()
//
// Description:
//	Clone this group.
//
//  The OPC Server should create a new OPC Group with a unique name and
//  identical properties to the group that is being cloned.  If the 
//  cloned group contains items, the items should also be cloned, again
//  with the same properties.  The active state of the cloned group and
//  items will be set to an inactive state.  This makes use of the 
//	IOPCGroupStateMgt::CloneGroup interface member function, and is therefore
//	not the same as a copy and paste operation.
//
// Parameters:
//  none
//
// Returns:
//  CKGroup* - Pointer to clone object.
// **************************************************************************
CKGroup* CKGroup::Clone ()
	{
	ASSERT (m_pIGroupState != NULL);

	CKGroup *pClone		= NULL;
	HRESULT hr			= E_FAIL;
	IUnknown *pUnknown	= NULL;
	WCHAR pszName [1];
	bool bSuccess		= false;

	// Wrap whole process in exception wrapper in case something bad happens:
	try
		{
		// Create a new group:
		pClone = new CKGroup (m_pServer);

		// Issue a clone group request using the IOPCGroupStateMgt interface:
		pszName [0] = NULL;
		hr = m_pIGroupState->CloneGroup (
				pszName,		// Let the server create a unique name
				IID_IUnknown,	// Request the IUnknown pointer to the clone
				&pUnknown);		// IUnknown pointer

		// Test success:
		if (SUCCEEDED (hr))
			{
			// We successfully cloned this group.

			// Set success flag:
			bSuccess = true;

			// Set new CKGroup valid since it has a corresponding OPC Group:
			pClone->SetValid (TRUE);

			// Initialize clone and add its items if any.  (We should have a 
			// valid IUnknown pointer, but wrap just in case.)
			if (pUnknown)
				{
				IEnumOPCItemAttributes *pEnum = NULL;

				// Initialize our new group:
				pClone->Initialize (pUnknown);

				// We no longer need IUnknown, so release it:
				pUnknown->Release ();

				// Set the server of the client handle and update rate for this group:
				OPCHANDLE hClient = (OPCHANDLE) pClone;
				DWORD dwRevUpdateRate = 0;

				// Issue request to set group update rate and handle using
				// IOPCGroupStateMgt interface (NULL indicates don't change):
				hr = pClone->m_pIGroupState->SetState (
						NULL,				// Requested update rate
						&dwRevUpdateRate,	// Revised update rate
						NULL,				// Active state
						NULL,				// Timer bias
						NULL,				// Percent deadband
						NULL,				// Localization ID use by group
						&hClient);			// Client supplied group handle

				ASSERT (SUCCEEDED (hr));

				// Now see what items we picked up by this group.  Use the
				// CreateEnumerator function of the IOPCItemMgt interface:
				ASSERT (pClone->m_pIItemMgt != NULL);
				hr = pClone->m_pIItemMgt->CreateEnumerator (
						IID_IEnumOPCItemAttributes,	// Requested interface
						(IUnknown **)&pEnum);		// Interface pointer

				// Note: this should succeed even if no items are attached to
				// the cloned group:
				if (hr == S_OK && pEnum != NULL)
					{
					OPCITEMATTRIBUTES *pIA;
					ULONG fetched;
					CKItem *pItem = NULL;
					TCHAR szBuffer [DEFBUFFSIZE];
					CString strTemp;

					// Read the contents of returned enumeration:
					while ((hr = pEnum->Next (1, &pIA, &fetched)) == S_OK)
						{
						// Create and add the item to our cloned group list:
						try
							{
							pItem = new CKItem (pClone);
							pClone->AddItemToList (pItem);
							}
						
						catch (...)
							{
							// new probably failed - not likely.  Break out of loop because
							// this would indicate major problems.
							ASSERT (FALSE);
							break;
							}

						// Set the item properties:
						
						// The string values (Item ID and access path will come in
						// with wide character format.  They will have to be 
						// reformatted for ANSI builds.
#ifdef _UNICODE
						lstrcpyn (szBuffer, pIA->szItemID, sizeof (szBuffer) / sizeof (TCHAR));
#else
						_wcstombsz (szBuffer, pIA->szItemID, sizeof (szBuffer) / sizeof (TCHAR));
#endif
						strTemp = szBuffer;
						pItem->SetItemID (strTemp);

						if (pIA->szAccessPath)
							{
#ifdef _UNICODE
							lstrcpyn (szBuffer, pIA->szAccessPath, sizeof (szBuffer) / sizeof (TCHAR));
#else
							_wcstombsz (szBuffer, pIA->szAccessPath, sizeof (szBuffer) / sizeof (TCHAR));
#endif
							strTemp = szBuffer;
							pItem->SetAccessPath (strTemp);
							}

						// Set other item properties:
						pItem->SetActive (pIA->bActive);
						pItem->SetServerHandle (pIA->hServer);
						pItem->SetAccessRights (pIA->dwAccessRights);
						pItem->SetDataType (pIA->vtCanonicalDataType);

						// The item is valid because it has a corresponding OPC Item:
						pItem->SetValid (TRUE);

						// Now update the client handle for this item.  (When the
						// server clones an item, as part of this cloned group, it
						// assigns the same client handles for the items.)
						HRESULT *pErrors = NULL;
						OPCHANDLE hServer = pIA->hServer;
						OPCHANDLE hClient = (OPCHANDLE) pItem;

						// Issue requet to set client handle using IOPCItemMgt interface:
						hr = pClone->m_pIItemMgt->SetClientHandles (
							1,			// Number of items
							&hServer,	// Server item handle
							&hClient,	// Client item handle
							&pErrors);	// Errors

						ASSERT (SUCCEEDED (hr));

						// COM requires us to free memory allocated for [out] and [in/out]
						// arguments (i.e. pErrors, string resources, and the blob):
						if (pErrors)
							CoTaskMemFree (pErrors);

						if (pIA->szAccessPath) 
							CoTaskMemFree (pIA->szAccessPath);

						if (pIA->szItemID) 
							CoTaskMemFree (pIA->szItemID);
						
						if (pIA->pBlob) 
							CoTaskMemFree (pIA->pBlob);
						
						// Should clear variant member before freeing pIA:
						VariantClear (&pIA->vEUInfo);

						// Now free the attribute:
						CoTaskMemFree (pIA);
						}

					// Release the enumerator now we're done with it:
					pEnum->Release ();
					}
				}
			
			// Failed to get valid *pIUnknown on success - should never occur:
			else
				{
				ASSERT (FALSE);
				LogMsg (IDS_CLONE_GROUP_INVALID_INTERFACE, GetName (), m_pServer->GetProgID ());
				}
			}

		// Failure to clone group:
		else
			{
			LogMsg (IDS_CLONE_GROUP_FAILED, GetName (), m_pServer->GetProgID (), hr);
			}

		// Let exception handler cleanup on failure:
		if (!bSuccess)
			throw (-1);
		}

	catch (...)
		{
		// There was some sort of a problem.  Clean up:
		if (pClone)
			{
			// Unitialise the clone (UnAdvise etc.) 
			pClone->Uninitialize ();

			// Delete the clone:
			delete pClone;
			}

		pClone = NULL;
		LogMsg (IDS_CLONE_GROUP_FAILED, GetName (), m_pServer->GetProgID (), hr);
		}

	// Add the cloned group to our server's group list:
	if (pClone)
		{
		m_pServer->AddClonedGroup (pClone);

		// Log success:
		LogMsg (IDS_CLONE_GROUP_SUCCESS, pClone->GetName (), GetName (),
			pClone->m_cdwItems, m_pServer->GetProgID ());
		}

	// Return pointer to clone.  This will be NULL if failure:
	return (pClone);
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroup helpers
/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
static double wtof (LPCWSTR lpszValue)
	{
	static char szBuffer [256];

	wcstombs (szBuffer, lpszValue, sizeof (szBuffer) / sizeof (char));
	return (atof (szBuffer));
	}
#endif

// **************************************************************************
// MapStringValToVariant ()
//
// Description:
//	Map a string value to a variant.
//
// Parameters:
//  CString		&strValue	String to be converted to variant
//	VARIANT		&vtVal		Variant.
//	VARTYPE		vtType		Variant type to force string to.
//
// Returns:
//  bool - true if success.
// **************************************************************************
bool CKGroup::MapStringValToVariant (CString &strValue, VARIANT &vtVal, VARTYPE vtType)
	{
	// Assume success until a problem arises:
	bool bSuccess = true;

	// Initialize variant for use:
	VariantInit (&vtVal);

	// Set variant type:
	vtVal.vt = vtType;

	// Cast string to proper value:
	switch (vtVal.vt)
		{
		case VT_BOOL:
			{
			unsigned char ucVal = (unsigned char) _ttol (strValue);
			vtVal.boolVal = ucVal ? VARIANT_TRUE : VARIANT_FALSE;
			}
			break;

		case VT_UI1:
			vtVal.bVal = (unsigned char) _ttol (strValue);
			break;

		case VT_I1:
			vtVal.cVal = (char) _ttol (strValue);
			break;

		case VT_UI2:
			vtVal.uiVal = (unsigned short) _ttol (strValue);
			break;

		case VT_I2:
			vtVal.iVal = (short) _ttol (strValue);
			break;

		case VT_UI4:
			vtVal.ulVal = (unsigned long) _ttol (strValue);
			break;

		case VT_I4:
			vtVal.lVal = (long) _ttol (strValue);
			break;

		case VT_R4:
			// Text to Float conversions can threow expections.  Be
			// prepared to handle them:
			try
				{
#ifdef _UNICODE
				vtVal.fltVal = (float) wtof (strValue);
#else
				vtVal.fltVal = (float) atof (strValue);
#endif
				}
			
			catch (...)
				{
				// Set value to zero in case of conversion error:
				vtVal.fltVal = 0;
				}
			break;

		case VT_R8:
			// Text to Float conversions can threow expections.  Be
			// prepared to handle them:
			try
				{
#ifdef _UNICODE
				vtVal.dblVal = (double) wtof (strValue);
#else
				vtVal.dblVal = (double) atof (strValue);
#endif
				}
			
			catch (...)
				{
				// Set value to zero in case of conversion error:
				vtVal.dblVal = 0;
				}
			break;

		case VT_BSTR:
			{
			// Get string length:
			DWORD dwLength = strValue.GetLength () + 1;

			// Alocate memory for string:
			vtVal.bstrVal = SysAllocStringLen (NULL, dwLength);

#ifdef _UNICODE
			// UNICODE build, so do a simple string copy:
			lstrcpyn (vtVal.bstrVal, strValue, dwLength);
#else
			// ANSI build requires string conversion:
			MultiByteToWideChar (CP_ACP, 0, strValue, -1, vtVal.bstrVal, dwLength);
#endif
			}
			break;

		// We don't expect array types.  If so, return false:
		case VT_UI1	| VT_ARRAY:
		case VT_I1	| VT_ARRAY:
		case VT_UI2	| VT_ARRAY:
		case VT_I2	| VT_ARRAY:
		case VT_UI4	| VT_ARRAY:
		case VT_I4	| VT_ARRAY:
		case VT_R4	| VT_ARRAY:
		case VT_R8	| VT_ARRAY:
			ASSERT (FALSE);
			bSuccess = false;
			break;

		default:
			// Unexpected variant type.  Return false:
			bSuccess = false;
			break;
		}

	// Return success:
	return (bSuccess);
	}

// **************************************************************************
// GetArrayElement ()
//
// Description:
//	Parse an element from a comma delimited string of array values.  Called 
//	once per element.  pnStart will be reset to end of current element for 
//	subsequent call.
//
// Parameters:
//  LPCTSTR		szInBuff	Input string.
//	int			*pnStart	Character position to begin parse.
//	LPTSTR		szOutBuff	Array element output string buffer.
//	int			nBuffSize	Size of output buffer.
//
// Returns:
//  GETARRELEMRET - Values enumerated in group.h.
// **************************************************************************
CKGroup::GETARRELEMRET CKGroup::GetArrayElement (LPCTSTR szInBuff, int *pnStart, LPTSTR szOutBuff, int nBuffSize)
	{
	// Initialize some variables:
	TCHAR ch = _T('\r');
	BOOL bLeftBracket = FALSE;
	int cnChars = 0;
	
	GETARRELEMRET eRet = tElement;

	// If we are at the end of the buffer then we're done:
	if (!szInBuff [*pnStart])
		return (tDone);

	// Continue to read characters until we hit the next comma or
	// an EOL (End Of Line - "\n") character:
	while (TRUE)
		{
		// Get the next character, and increment start position for next time:
		ch = szInBuff [(*pnStart)++];

		// If the character is NULL, we are at the end of the record and
		// therefore is nothing left to read.  Back up the start position
		// and break out of the loop.
		if (!ch)
			{
			(*pnStart)--;
			break;
			}

		// Trim leading whitespace, if any.  If current character is a space,
		// then continue.  This will force us to process next character without
		// saving current one (a space) in output buffer.
		if (!cnChars && _istspace (ch))
			continue;

		// Brackets and commas delimit the fields:
		if (ch == _T('['))
			{
			if (bLeftBracket)
				{
				eRet = tInvalid;
				break;
				}

			continue;
			}

		if (ch == _T(']'))
			{
			eRet = tEndRow;
			break;
			}

		if (ch == _T(','))
			break;

		// If we make it here, then the current character is "a keeper".
		// Increment the character counter:
		++cnChars;

		// Add the current character to the output buffer as long a there
		// is room:
		if (nBuffSize > 1)
			{
			// There is room, so add the current character to the output 
			// buffer, and move output buffer pointer to next position.
			*szOutBuff++ = ch;

			// Decrement nBuffSize.  When it hits zero we know we have
			// filled the output buffer to capacity.
			--nBuffSize;
			}
		else
			{
			// There is no more room in the output buffer.  Set the bOverflow
			// flag, but don't break out of the loop.  We want to keep going
			// until we hit a field deleimiter.  This will allow us to parse
			// the next field, even though this one is too big to deal with.
			eRet = tOverflow;
			break;
			}
		}

	// Make sure the output string is properly NULL terminated:
	*szOutBuff = 0;

	// Return return code:
	return (eRet);
	}

// **************************************************************************
// MapStringValToArrayVariant ()
//
// Description:
//	Map a string of array elements to a variant array.
//
// Parameters:
//  CString		&strValue		Input string.
//	VARIANT		*pvtSrc			Source variant.  Used to set properties of
//								  destination variant.
//	VARIANT		*pvtDst			Destination variant
//
// Returns:
//  bool - true if successful.
// **************************************************************************
bool CKGroup::MapStringValToArrayVariant (CString &strValue, VARIANT *pvtSrc, VARIANT *pvtDst)
	{
	// Source variant must not be empty:
	if (pvtSrc->vt == VT_EMPTY)
		return (false);

	ASSERT (pvtSrc != NULL);
	ASSERT (pvtDst != NULL);
	ASSERT (pvtSrc->vt & VT_ARRAY);

	// Source variant must contain array data:
	if (!pvtSrc->parray)
		return (false);

	int cnRows = 0;
	int cnCols = 0;
	TCHAR szValue [DEFBUFFSIZE];

	// Create a local copy of input string:
	lstrcpyn (szValue, strValue, (sizeof (szValue) / sizeof (TCHAR)) - sizeof (TCHAR));

	// Set number of rows and columns for this array:
	
	// 1-D array:
	if (pvtSrc->parray->cDims == 1)
		{
		cnRows = 1;
		cnCols = pvtSrc->parray->rgsabound [0].cElements;
		}

	// 2-D array:
	else if (pvtSrc->parray->cDims == 2)
		{
		cnRows = pvtSrc->parray->rgsabound [0].cElements;
		cnCols = pvtSrc->parray->rgsabound [1].cElements;
		}

	// Do not support more than 2 dimensions:
	else
		{
		ASSERT (FALSE);
		return (false);
		}

	// Initialize destination array:
	VariantInit (pvtDst);

	// Set destination array type:
	pvtDst->vt = pvtSrc->vt;

	// Allocate memory for destination array:
	pvtDst->parray = SafeArrayCreate (pvtSrc->vt & ~VT_ARRAY, 
		pvtSrc->parray->cDims, pvtSrc->parray->rgsabound);

	// Copy data to destination array:
	VariantCopy (pvtDst, pvtSrc);

	// Overwrite fields with data:
	int nIndex = 0;
	GETARRELEMRET eRet;
	BYTE HUGEP *pVal = (BYTE *) pvtDst->parray->pvData;
	int cnRowElements = 0;

	// Loop over all array elements:
	for (int i = 0; i < cnRows * cnCols; i++)
		{
		// Create a scratch buffer for parsed array element string:
		TCHAR szBuffer [DEFBUFFSIZE];

		// Parse next element in string:
		eRet = GetArrayElement (szValue, &nIndex, szBuffer, DEFBUFFSIZE);

		// If parse resulted in done code, the break out of loop:
		if (eRet == tDone)
			break;

		// If parse resulted in invalid code, return false:
		if (eRet == tInvalid)
			{
			TRACE (_T("OTC: Invalid character parsing array value\r\n"));
			return (false);
			}

		// If parse reslted in overflow code, return false:
		if (eRet == tOverflow)
			{
			TRACE (_T("OTC: Buffer overflow parsing array value\r\n"));
			return (false);
			}

		// If we make it here, we expect to have element or end of row code:
		ASSERT (eRet == tElement || eRet == tEndRow);

		// If element is not NULL string, then overwrite field in destination:
		if (*szBuffer != _T('\0'))
			{
			// Cast string to proper type:
			switch (pvtDst->vt & ~VT_ARRAY)
				{
				case VT_UI1	:	*pVal = (BYTE) _ttol (szBuffer); break;
				case VT_I1	:	*pVal = (char) _ttol (szBuffer);	break;
				case VT_UI2	:	*(WORD *)pVal = (WORD) _ttol (szBuffer);	break;
				case VT_I2	:	*(short *)pVal = (short) _ttol (szBuffer); break;
				case VT_UI4 :	*(DWORD *)pVal = (DWORD) _ttol (szBuffer); break;
				case VT_I4	:	*(long *)pVal = (long) _ttol (szBuffer);	break;

				case VT_R4:
					// Text to float conversions can throw exceptions, so 
					// be prepared to handle them:
					try
						{
#ifdef _UNICODE
						*(float *)pVal = (float) wtof (szBuffer);
#else
						*(float *)pVal = (float) atof (szBuffer);
#endif
						}
					
					catch (...)
						{
						*(float *)pVal = 0;
						}

					break;

				case VT_R8:
					// Text to float conversions can throw exceptions, so 
					// be prepared to handle them:
					try
						{
#ifdef _UNICODE
						*(double *)pVal = (double) wtof (szBuffer);
#else
						*(double *)pVal = (double) atof (szBuffer);
#endif
						}
					
					catch (...)
						{
						*(double *)pVal = 0;
						}
					break;

				// Unexpected type:
				default:
					ASSERT (FALSE);
					break;
				}
			}

		// Increment destination array pointer to accept next element:

		// If last element in row:
		if (eRet == tEndRow && cnCols > cnRowElements)
			{
			pVal += (pvtDst->parray->cbElements * (cnCols - cnRowElements));
			cnRowElements = 0;
			}

		// Else next element:
		else
			{
			cnRowElements++;
			pVal += pvtDst->parray->cbElements;
			}
		}

	// If we make it here, then return true to indicate success:
	return (true);
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroup serialization
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Serialize ()
//
// Description:
//	Save or load group setting.
//
// Parameters:
//  CArchive		&ar			The archive to save or load group settins.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Serialize (CArchive &ar)
	{
	// If saving object properties:
	if (ar.IsStoring ())
		{
		// Save version first.  This will let us know how to read data 
		// later.  If changes are made, say another parameter is added,
		// then version should be bumped up.
		ar << CURRENT_VERSION;

		// Save object properties:
		ar << m_strName;							// name
		ar << m_dwUpdateRate;						// update rate
		ar << m_dwLanguageID;						// language ID
		ar << m_lBias;								// time bias
		ar << m_fDeadband;							// deadband
		ar << m_bActive;							// active state
		ar << m_dwUpdateMethod;						// update method
		
		// Save flags bit field.  Use of bit field allows us to add
		// more flags (for up to 32 total) without having to change version.
		ar.Write (&m_bfFlags, sizeof (m_bfFlags));

		// Now store items:

		// Save item count first, so we will know how many items to try
		// to read later:
		ar << m_cdwItems;
						
		// Now store item data.  Start with first item in our linked list
		// and work our way to end (indicated by NULL pointer).
		CKItem *pItem = m_pItemHead;

		while (pItem)			
			{
			// Call item's serialize function to save properties:
			pItem->Serialize (ar);
			
			// Get next item in linked list:
			pItem = pItem->GetNext ();
			}
		}

	// Else loading object properties:
	else
		{
		DWORD dwIndex;
		DWORD dwSchema;
		DWORD cdwItems;

		// Read version so we will know how to read subsequent data:
		ar >> dwSchema;

		// Read according to version:
		switch (dwSchema)
			{
			case VERSION_1:
				// Read object properties:
				ar >> m_strName;						// name
				ar >> m_dwUpdateRate;					// update rate
				ar >> m_dwLanguageID;					// language ID
				ar >> m_lBias;							// time bias
				ar >> m_fDeadband;						// deadband
				ar >> m_bActive;						// active state
				ar >> m_dwUpdateMethod;					// update method

				// Read flags bit field:
				ar.Read (&m_bfFlags, sizeof (m_bfFlags));

				// Now read item data:

				// Get number of items first:
				ar >> cdwItems;

				// Add items to our linked list, which should be
				// empty at this point:
				ASSERT (m_pItemHead == NULL);

				// Loop over expected number of items:
				for (dwIndex = 0; dwIndex < cdwItems; dwIndex++)
					{
					CKItem *pItem = NULL;

					// Prepare to handle exceptions:
					try
						{
						// Instantiate a new CKItem object:
						pItem = new CKItem (this);

						// Call item's serialize funtion to read its
						// properties:
						pItem->Serialize (ar);

						// Add the item to our list:
						AddItemToList (pItem);
						}
					
					catch (...)
						{
						// If propblem, self-delete and rethrow exception:
						delete this;
						AfxThrowArchiveException (CArchiveException::generic);
						}
					}

				break;

			default:
				// Unexpected version.  We don't know how to read data
				// so self-delete and throw exception.
				delete this;
				AfxThrowArchiveException (CArchiveException::badSchema);
				break;
			}
		}
	}

// **************************************************************************
// Start ()
//
// Description:
//	Add all items to group.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Start ()
	{
	// Add all items, if any, to this group:
	if (m_cdwItems)
		{
		CKItem *pItem;
		DWORD dwIndex;

		// Create an object array to contain pointers to all
		// items to be added:
		CObArray cList;

		// Be prepared to handle exceptions that could get
		// thrown by object array:
		try
			{
			// Allocate enough array memory to contain pointers for 
			// all items we will be adding:
			cList.SetSize (m_cdwItems);
			}
		
		catch (...)
			{
			// If problem, don't add any items.
			ASSERT (FALSE);
			TRACE (_T("OTC: CKGroup::Start memory exception thrown\r\n"));
			return;
			}

		// Load the object array.  Start with head of linked list:
		pItem = m_pItemHead;
		ASSERT (m_pItemHead != NULL);

		for (dwIndex = 0; dwIndex < m_cdwItems; dwIndex++)
			{
			// Add item to object array:
			cList.SetAt (dwIndex, pItem);

			// Get next item in linked list:
			pItem = pItem->GetNext ();
			}

		// Add items:
		AddItems (cList, m_cdwItems, true /* loading project */);
		}
	}

// **************************************************************************
// ExportCsv ()
//
// Description:
//	Export properties of all item's in group to CSV file.
//
// Parameters:
//  CStdioFile	&csv		File to export properties to.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::ExportCsv (CStdioFile &csv)
	{
	CString str;

	// Get handle to file stream:
	FILE *fh = csv.m_pStream;

#ifdef _DEBUG
	// ************************************
	// Step 1. 
	// Write a comment line to indicate what server and group
	// data is from.  For debug only.  Comment is of the form:  
	//	;
	//	; <server\group>
	//	;
	// ************************************

	csv.WriteString (_T(";\n"));

	str = m_pServer->GetProgID ();
	str += _T("\\");
	str += GetName ();

	csv.WriteString (_T("; "));
	csv.WriteString (str);
	csv.WriteString (_T("\n"));

	csv.WriteString (_T(";\n"));
#endif

	// ************************************
	// Step 2. Write out the header record
	// ************************************

	// Make sure master field list is loaded:
	LoadMasterFieldList ();
	int i;
	// Write out the default header:
	for (i = 0; i < sizeof (astMasterCSV) / sizeof (astMasterCSV[0]) - 1; i++)
		_ftprintf (fh, _T("%s,"), astMasterCSV[i].szText);

	_ftprintf (fh, _T("%s\n"), astMasterCSV[i].szText); // Last field get a new line character

	// ************************************
	// Step 3. Write out the item records
	// ************************************

	// Start with head of item linked list:
	CKItem *pItem = GetItemHead ();

	// Loop until we hit end of linked list, indicated by NULL pointer:
	while (pItem)
		{
		// Use the master header information to select the order of each record.
		// Loop over number of fields in master header:
		for (int i = 0; i < sizeof (astMasterCSV) / sizeof (astMasterCSV[0]); i++)
			{
			CString str;

			// Put a comma after each record:
			if (i)
				_fputts (_T(","), fh);

			// Process according to field type:
			switch (astMasterCSV [i].eID)
				{
				case CSV_ItemID:
					_ftprintf (fh, _T("%s"), pItem->GetItemID ());
					break;

				case CSV_AccessPath:
					_ftprintf (fh, _T("\"%s\""), pItem->GetAccessPath ());
					break;

				case CSV_DataType:
					StringFromVartype (pItem->GetDataType (), str);
					_ftprintf (fh, _T("%s"), str);
					break;

				case CSV_Active:
					_ftprintf (fh, _T("%s"), pItem->IsActive () ? _T("TRUE") : _T("FALSE"));
					break;

				default:
					ASSERT (FALSE);	// We missed a column
					break;
				}
			}

		// Terminate the record:
		_fputts (_T("\n"), fh);

		// Get next item in linked list:
		pItem = pItem->GetNext ();
		}
	}

// **************************************************************************
// ImportCsv ()
//
// Description:
//	Import items from CSV file.
//
// Parameters:
//  CStdioFile	&csv			File to import items from.
//	CObArray	&cItemList		Array to fill during import.
//	DWORD		&cdwItems		Number of items imported.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::ImportCsv (CStdioFile &csv, CObArray &cItemList, DWORD &cdwItems)
	{
	// Get handle to file stream:
	FILE *fh = csv.m_pStream;

	// Initialize variables:
	CKItem *pItem = NULL;
	cdwItems = 0;

	// Be prepared to handle exceptions:
	try
		{
		// Do an initial array memory allocation of 16 element, and
		// set grow size to 16:
		cItemList.SetSize (16, 16);

		// ****************************************
		// Step 1.  Read the ID record
		// ****************************************

		// Allocate memory for a parse buffer (use a CString so that we get
		// a destructor to clean up for us):
		CString strParseBuff;
		LPTSTR pParseBuff = strParseBuff.GetBufferSetLength (PARSEBUFFERSIZE * sizeof (TCHAR));
		LPTSTR pFirstChar;

		// Read the header record (skip over blank lines, and comments):
		do
			{
			// Initialize parse buffer:
			*pParseBuff = 0;
			
			// Read a record.  Throw an exception if error:
			if (!_fgetts (pParseBuff, PARSEBUFFERSIZE - 1, fh))
				throw (-1);
			
			// Read the rest of the line if we didn't get it all.
			// We won't be able to do anything with this extra data.
			// This will just place the file pointer at the start of
			// the next record.
			if (!_tcsrchr (pParseBuff, _T('\n')))
				{
				TCHAR sz [256];
				do
					{
					_fgetts (sz, _countof (sz) - 1, fh);
					}
				while (!_tcsrchr (sz, _T('\n')));
				}

			// Save pointer to first character:
			pFirstChar = pParseBuff;
			
			// Skip over leading white space:
			while (_istspace (*pFirstChar))
				++pFirstChar;

			// Keep looping as long as the first detected character is the
			// NULL terminator or a start of comment indicator:
			} while (!*pFirstChar || *pFirstChar == _T(';'));

		// Parse out the fields that we expect to see (BuildFieldList() will throw an
		// exception if an error occurs.)
		CSVHEADERFIELD *pFieldList = BuildFieldList (pParseBuff);
		ASSERT (pFieldList);

		// Clear the parse buffer:
		memset (pParseBuff, 0, PARSEBUFFERSIZE * sizeof (TCHAR));

		int nRecord = 0; // Record counter (for error messages)
		CString str;

		// ****************************************
		// Step 3.  Read each CSV item record
		// ****************************************

		// Read records until an error occurs or the end of file is encountered,
		// indicated by NULL return from _ffetts():
		while (_fgetts (pParseBuff, PARSEBUFFERSIZE - 1, fh))
			{
			// If the only character in the buffer is an EOL then we're done:
			if (pParseBuff[0] == _T('\n'))
				break;

			// Get our field list:
			CSVHEADERFIELD *pField = pFieldList;
			int nBuffIndex = 0;

			// Increment record counter (for error reporting):
			++nRecord;

			// Create a new item to accept input:
			pItem = new CKItem (this);

			// Run through all the requested fields:
			while (pField)
				{
				TCHAR szBuff[MAXCSVIMPORTFIELDLENGTH];
				READCSVRET eRC;

				// Parse next field from record:
				eRC = ReadCSV (pParseBuff, &nBuffIndex, szBuff, MAXCSVIMPORTFIELDLENGTH);

				// If we got something, assign it to the item:
				if (eRC == tFieldOK)
					{
					CSVToItem (pField->eID, szBuff, pItem);
					pField = pField->pNext;
					}
				
				// Else if field was too big for the supplied buffer, 
				// issue error message and throw exception:
				else if (eRC == tFieldOverflow)
					{
					// "Error importing CSV item record %d\n\nField buffer overflow."
					str.Format (IDS_FIELDBUFFEROVERFLOW, nRecord);
					AfxMessageBox (str);
					throw (-1);
					}

				// Else if we reached the end of the record, we're done. Set 
				// field pointer to NULL to terminate loop.
				else if (eRC == tEndOfRecord)
					pField = NULL;

				// Programmer error -> Forgot to add handler for return value:
				else
					{
					ASSERT (FALSE);
					throw (-1);
					}
				}

			// Get item ID:
			str = pItem->GetItemID ();

			// Attempt to insert the record (if it contains a valid id),
			// otherwise delete the item and proceed to next record:
			if (!str.IsEmpty ())
				cItemList.SetAtGrow (cdwItems++, (CObject *)pItem);
			else
				delete pItem;
				
			// Now are done with this item, we can set pointer to NULL.
			// Don't delete it.  We want to keep the object.  It is still
			// referenced in object array.
			pItem = NULL;

			// Clear the parse buffer for the next record:
			memset (pParseBuff, 0, PARSEBUFFERSIZE * sizeof (TCHAR));
			}

		// Terminate the list.  Will allow us to process list using
		// "while (element)" loop if we want:
		cItemList.SetAtGrow (cdwItems, NULL);
		}

	catch (...)
		{
		TRACE (_T("OTC: An exception was thrown during CSV item import.\n"));
		
		// Delete current item since there was a problem reading it's
		// properties:
		if (pItem)
			delete pItem;

		// Delete all other items:
		while (cdwItems--)
			{
			// Get pointer to last item in object array:
			pItem = (CKItem *)cItemList.GetAt (cdwItems);

			// Delete that item:
			delete pItem;
			}

		// Re-throw the exception:
		throw;
		}
	}

// **************************************************************************
// Copy ()
//
// Description:
//	Copy group.  Save properties to shared memory file.
//
// Parameters:
//  CFixedSharedFile	&sf		Shared memory file to save properties to.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Copy (CFixedSharedFile &sf)
	{
	// Define a variable for string lengths.  Shared memory files are'nt as
	// smart as CArchive objects, so we will need to explicitly save string
	// lengths along with the string data.
	int nLen;

	// Save name string:
	nLen = m_strName.GetLength ();
	sf.Write (&nLen, sizeof (nLen));
	sf.Write (m_strName, nLen * sizeof (TCHAR));

	// Save update rate:
	sf.Write (&m_dwUpdateRate, sizeof (m_dwUpdateRate));

	// Save language ID (LCID):
	sf.Write (&m_dwLanguageID, sizeof (m_dwLanguageID));

	// Save time bias:
	sf.Write (&m_lBias, sizeof (m_lBias));

	// Save deadband:
	sf.Write (&m_fDeadband, sizeof (m_fDeadband));

	// Save active state:
	sf.Write (&m_bActive, sizeof (m_bActive));

	// Save update method:
	sf.Write (&m_dwUpdateMethod, sizeof (m_dwUpdateMethod));

	// Save flag bit field:
	sf.Write (&m_bfFlags, sizeof (m_bfFlags));

	// Save item count so we'll know how much item data follows when we
	// back:
	sf.Write (&m_cdwItems, sizeof (m_cdwItems));

	// Save item information.  Start with head of linked list and work
	// our way to end (indicated by a NULL pointer).
	CKItem *pItem = m_pItemHead;

	while (pItem)			
		{
		// Call the item's copy function to save its properties to 
		// shared memory file:
		pItem->Copy (sf);

		// Get the next item in linked list:
		pItem = pItem->GetNext ();
		}
	}

// **************************************************************************
// Paste ()
//
// Description:
//	Paste group.  Load properties from shared memory file
//
// Parameters:
//  CFixedSharedFile	&sf		Shared memory file to load properties from.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Paste (CFixedSharedFile &sf)
	{
	// Define a variable for string lengths.  Shared memory files are'nt as
	// smart as CArchive objects, so we will need to explicitly save string
	// lengths along with the string data.
	int nLen;

	// Read name string:
	sf.Read (&nLen, sizeof (nLen));
	sf.Read (m_strName.GetBufferSetLength (nLen), nLen * sizeof (TCHAR));

	// Read update rate:
	sf.Read (&m_dwUpdateRate, sizeof (m_dwUpdateRate));

	// Read language ID (LCID):
	sf.Read (&m_dwLanguageID, sizeof (m_dwLanguageID));

	// Read time bias:
	sf.Read (&m_lBias, sizeof (m_lBias));

	// Read deadband:
	sf.Read (&m_fDeadband, sizeof (m_fDeadband));

	// Read active state:
	sf.Read (&m_bActive, sizeof (m_bActive));

	// Read update method:
	sf.Read (&m_dwUpdateMethod, sizeof (m_dwUpdateMethod));

	// Read flag bit field:
	sf.Read (&m_bfFlags, sizeof (m_bfFlags));

	// Read item count so we will know how many item's we need to 
	// read next:
	DWORD cdwItems = 0;
	sf.Read (&cdwItems, sizeof (cdwItems));

	// Read item data and add to list.  Our linked list should be empty 
	// at this point.
	ASSERT (m_pItemHead == NULL);

	// Loop over number of items:
	for (DWORD dwIndex = 0; dwIndex < cdwItems; dwIndex++)
		{
		CKItem *pItem = NULL;

		// Handle exceptions:
		try
			{
			// Instantiate a new CKItem object.  Pass it a pointer to
			// its parent group (this object).
			pItem = new CKItem (this);

			// Call item's paste function to read in its properties:
			pItem->Paste (sf);

			// Add the item to our linked list:
			AddItemToList (pItem);
			}
		
		catch (...)
			{
			// Rethrow the exception:
			ASSERT (FALSE);
			throw (CArchiveException::generic);
			}
		}
	}


/////////////////////////////////////////////////////////////////////////////
// CKGroup OPC Specifics
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Initialize ()
//
// Description:
//	Initialize group.
//
// Parameters:
//  LPUNKNOWN	pUnknown		Pointer to group IUnknown interface.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Initialize (LPUNKNOWN pUnknown)
	{
	ASSERT (pUnknown != NULL);
	TRACE (_T("OTC: Initializing group %s interfaces.\r\n"), GetName ());

	HRESULT hr;


	//////////////////////////////////////////////////////////////
	// Get IID_IOPCGroupStateMgt interface and set group state	//
	//////////////////////////////////////////////////////////////

	// Get pointer to IID_IOPCGroupStateMgt interface, save it in 
	// m_pIGroupState:
	ASSERT (m_pIGroupState == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCGroupStateMgt, (void **) &(m_pIGroupState));

	// If we got the interface, use it to get the OPC Group state, and
	// save state properties in this object:
	if (SUCCEEDED (hr) && m_pIGroupState)
		{
		// Get our current group name setting:
		CString strItemID = GetName ();

		// If name has not been set yet, we are relying on server to get it
		// and other properties.  Get name and other properties from server:
		if (strItemID.IsEmpty ())
			{
			WCHAR *pszName;
			OPCHANDLE hClient;

			// Use IID_IOPCGroupStateMgt interface to get OPC Group
			// properties:
			if (SUCCEEDED (m_pIGroupState->GetState (
				&m_dwUpdateRate,	// update rate
				&m_bActive,			// active state
				&pszName,			// put the name here
				&m_lBias,			// time bias
				&m_fDeadband,		// deadband
				&m_dwLanguageID,	// language id
				&hClient,			// server handle
				&m_hServer)))		// server handle
				{
				if (pszName)
					{
					// Set our name to the OPC Group name.  COM will give us
					// this name, as with all strings, it UNICODE format.  
#ifdef _UNICODE
					// This is a UNOCODE build, so save the name as is:
					SetName (pszName);
#else
					// This is an ANSI build, so convert the name to ANSI
					// format before saving:
					TCHAR szBuffer [DEFBUFFSIZE];
					if (WideCharToMultiByte (CP_ACP, 0, pszName, -1, szBuffer, sizeof (szBuffer), NULL, NULL))
						SetName ((LPCTSTR) szBuffer);
#endif

					// COM places the responsibility of freeing the string
					// string memory on us.  We're done with the name string
					// argument now, so free the memory:
					CoTaskMemFree (pszName);
					}
				}
			}
		}

	// Issue TRACE message if we failed to get IID_IOPCGroupStateMgt
	// interface pointer:
#ifdef _DEBUG
	if (FAILED (hr) || m_pIGroupState == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IOPCGroupStateMgt (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIGroupState);
		}
#endif // _DEBUG


	//////////////////////////////////////
	// Get other interface pointers.	//
	//////////////////////////////////////

	// Get IID_IOPCPublicGroupStateMgt interface:
	ASSERT (m_pIPublicGroupState == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCPublicGroupStateMgt, (void **) &(m_pIPublicGroupState));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIPublicGroupState == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IOPCPublicGroupStateMgt (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIPublicGroupState);
		}
#endif // _DEBUG

	// Get IID_IOPCItemMgt interface:
	ASSERT (m_pIItemMgt == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCItemMgt, (void **) &(m_pIItemMgt));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIItemMgt == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IItemMgt (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIItemMgt);
		}
#endif // _DEBUG

	// Get IID_IOPCSyncIO interface:
	ASSERT (m_pISync == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCSyncIO, (void **) &(m_pISync));

#ifdef _DEBUG
	if (FAILED (hr) || m_pISync == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_ISyncIO (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pISync);
		}
#endif // _DEBUG

	// Get IID_IOPCAsyncIO interface:
	ASSERT (m_pIAsync == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCAsyncIO, (void **) &(m_pIAsync));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIAsync == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IAsyncIO (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIAsync);
		}
#endif // _DEBUG

	// Get IID_IDataObject interface:
	ASSERT (m_pIDataObject == NULL);
	hr = pUnknown->QueryInterface (
		IID_IDataObject, (void **) &(m_pIDataObject));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIDataObject == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IDataObject (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIDataObject);
		}
#endif // _DEBUG

	// Get IID_IOPCAsyncIO2 interface:
	ASSERT (m_pIAsync2 == NULL);
	hr = pUnknown->QueryInterface (
		IID_IOPCAsyncIO2, (void **) &(m_pIAsync2));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIAsync2 == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IAsyncIO2 (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIAsync2);
		}
#endif // _DEBUG

	// Get IID_IConnectionPointContainer interface:
	ASSERT (m_pIConnPtContainer == NULL);
	hr = pUnknown->QueryInterface (
		IID_IConnectionPointContainer, (void **) &(m_pIConnPtContainer));

#ifdef _DEBUG
	if (FAILED (hr) || m_pIConnPtContainer == NULL)
		{
		TRACE (_T("OTC: Failed to query IID_IConnectionPointContainer (hr -> %08X, pI -> %08X)\r\n"),
			hr, m_pIConnPtContainer);
		}
#endif // _DEBUG


	//////////////////////////////////////////////////
	// Request on data change update notifications.	//
	//////////////////////////////////////////////////

	// Set this flag if 2.0 is requested but not supported:
	bool bDropTo10 = false;
	
	// If we want OPC 2.0 Data Change Notifications:
	if (GetUpdateMethod () == OPC_20_DATACHANGE)
		{
		// Assume failure until proven good:
		HRESULT hr = E_FAIL;

		// We will need the IID_IConnectionPointContainer interface.  If
		// not we will fall through with hr = E_FAIL:
		if (m_pIConnPtContainer)
			{
			// Get connection point (IID_IOPCDataCallback interface):
			IConnectionPoint *pCP = NULL;
			hr = m_pIConnPtContainer->FindConnectionPoint (IID_IOPCDataCallback, &pCP);

			// If we succeeded to get connection point interface, create
			// our data sink interface and advise server of it:
			if (SUCCEEDED (hr))
				{
				try
					{
					// Instantiate a new IKDataSink20:
					m_pIDataSink20 = new IKDataSink20 ();

					// Add ourselves to its reference count:
					m_pIDataSink20->AddRef ();
					
					// Advise the server of our data sink:
					hr = pCP->Advise (m_pIDataSink20, &m_dwCookieDataSink20);

					// We are done with the IID_IOPCDataCallback, so release
					// (remove us from its reference count):
					pCP->Release ();
					}
				
				catch (...)
					{
					// If a problem, make sure hr = E_FAIL so error gets 
					// processed correctly below:
					ASSERT (FALSE);
					hr = E_FAIL;
					}
				}
			}

		// If we did not succeed in connecting to the server 2.0 data,
		// try the 1.0 interface.  We do this by setting bDropTo10 flag.
		// Make sure we also reset our update method too.
		if (FAILED (hr))
			{
			bDropTo10 = true;
			LogMsg (IDS_CONNECT_TO_DATASINK20_FAILED, GetName (), m_pServer->GetProgID (), hr); 
			SetUpdateMethod (OPC_10_DATATIMECHANGE);
			}
		}

	// If we want OPC 1.0 Data Change Notifications (with or without timestamp)
	// or if we requested 2.0 but found that it is not supported by server:
	if ((GetUpdateMethod () == OPC_10_DATACHANGE) ||
		(GetUpdateMethod () == OPC_10_DATATIMECHANGE) ||
		(bDropTo10 == true))
		{
		// We will need the IID_IDataObject interface:
		if (m_pIDataObject)
			{
			// Will need to fill in a FORMATETC stucture to advise server
			// of our callback format:
			FORMATETC fmt;

			// First set "format" member (timestamp or not):
			if (GetUpdateMethod () == OPC_10_DATACHANGE)
				fmt.cfFormat = CF_DATA_CHANGE;
			else
				fmt.cfFormat = CF_DATA_CHANGE_TIME;

			// Fill in the "etc." members:
			fmt.dwAspect = DVASPECT_CONTENT;	// aspect of the object	
			fmt.ptd = NULL;						// pointer to a target device for which data is being composed
			fmt.tymed = TYMED_HGLOBAL;			// storage medium is a global memory handle (HGLOBAL
			fmt.lindex = -1;

			ASSERT (m_pIAdviseSink == NULL);

			try
				{
				// Instantiate a new CKAdviseSink object:
				m_pIAdviseSink = new CKAdviseSink ();

				// Add ourselves to its reference count:
				m_pIAdviseSink->AddRef ();

				// Advise server of our data sink:
				hr = m_pIDataObject->DAdvise (
					&fmt,					// format of interest
					0,						// advise flags	(OPC not supported)
					m_pIAdviseSink,			// our advise sink callback
					&m_dwCookieRead);		// cookie used for AsyncIO and DUnadvise

				if (FAILED (hr))
					{
					TRACE (_T("OTC: IDataObject::DAdvise failed\r\n"));
					}
				}
			
			// If a problem, return:
			catch (...)
				{
				TRACE (_T("OTC: CKGroup::Initialize memory exception thrown\r\n"));
				return;
				}

			// Must also request 1.0 write complete notifications.
			// Reuse FORMATETC structure to advise server:
			fmt.cfFormat = CF_WRITE_COMPLETE;	// format		
			fmt.dwAspect = DVASPECT_CONTENT;	// aspect of the object	
			fmt.ptd = NULL;						// pointer to a target device for which data is being composed
			fmt.tymed = TYMED_HGLOBAL;			// storage medium is a global memory handle (HGLOBAL
			fmt.lindex = -1;

			// Advise server that we want write complete notifications to
			// come to our data sink:
			hr = m_pIDataObject->DAdvise (
				&fmt,					// format of interest
				0,						// advise flags (OPC not supported)
				m_pIAdviseSink,			// our advise sink callback
				&m_dwCookieWrite);		// cookie used for AsyncIO and DUnadvise

			if (FAILED (hr))
				{
				TRACE (_T("OTC: IDataObject::DAdvise failed\r\n"));
				}
			}
		
		// Can't get 1.0 updates because server does not support required
		// IID_IDataObject interface:
		else
			{
			TRACE (_T("OTC: Unable to request OnDataChange 1.0 updates, interface not supported.\r\n"));
			}
		}
	}

// **************************************************************************
// Uninitialize ()
//
// Description:
//	Cleanup group before remove. If bDelete is false, we  remove our server
//	references but we do not free any objects
//
// Parameters:
//  bool		bDelete		Set to true to delete items.
//
// Returns:
//  void
// **************************************************************************
void CKGroup::Uninitialize (bool bDelete /* = true */)
	{
	HRESULT hr;

	// Unadvise read notifications:
	if (m_dwCookieRead != 0)
		{
		ASSERT (m_pIDataObject != NULL);
		hr = m_pIDataObject->DUnadvise (m_dwCookieRead);

		if (FAILED (hr))
			{
			TRACE (_T("OTC: Failed on IDataObject::DUnadvise on group %s, server %s (%08X)\r\n"),
				GetName (), m_pServer->GetProgID (), hr);
			}

		m_dwCookieRead = 0;
		}

	// Unadvise write notifications:
	if (m_dwCookieWrite != 0)
		{
		ASSERT (m_pIDataObject != NULL);
		hr = m_pIDataObject->DUnadvise (m_dwCookieWrite);

		if (FAILED (hr))
			{
			TRACE (_T("OTC: Failed on IDataObject::DUnadvise on group %s, server %s (%08X)\r\n"),
				GetName (), m_pServer->GetProgID (), hr);
			}

		m_dwCookieWrite = 0;
		}

	// Release our reference to our 1.0 sink:
	if (m_pIAdviseSink != NULL)
		{
		m_pIAdviseSink->Release ();
		m_pIAdviseSink = NULL;
		}

	// Unadvise data access 2.0 notifications:
	if (m_dwCookieDataSink20 != 0)
		{
		ASSERT (m_pIConnPtContainer != NULL);

		hr = E_FAIL;
		IConnectionPoint *pCP = NULL;

		hr = m_pIConnPtContainer->FindConnectionPoint (IID_IOPCDataCallback, &pCP);

		if (SUCCEEDED (hr))
			{
			hr = pCP->Unadvise (m_dwCookieDataSink20);
			pCP->Release ();
			}

		if (FAILED (hr))
			{
			TRACE (_T("OTC: CKGroup::Uninitialize failed to unadvise 2.0 updates\r\n"));
			}

		m_dwCookieDataSink20 = 0;
		}

	// Release our reference to our 2.0 sink:
	if (m_pIDataSink20 != NULL)
		{
		m_pIDataSink20->Release ();
		m_pIDataSink20 = NULL;
		}

	// Remove all outstanding items:
	RemoveAllItems (bDelete);

	// Release outstanding group references:
	if (m_pIGroupState)
		{
		m_pIGroupState->Release ();
		m_pIGroupState = NULL;
		}

	if (m_pIPublicGroupState)
		{
		m_pIPublicGroupState->Release ();
		m_pIPublicGroupState = NULL;
		}

	if (m_pIItemMgt)
		{
		m_pIItemMgt->Release ();
		m_pIItemMgt = NULL;
		}

	if (m_pISync)
		{
		m_pISync->Release ();
		m_pISync = NULL;
		}

	if (m_pIAsync)
		{
		m_pIAsync->Release ();
		m_pIAsync = NULL;
		}

	if (m_pIDataObject)
		{
		m_pIDataObject->Release ();
		m_pIDataObject = NULL;
		}

	if (m_pIAsync2)
		{
		m_pIAsync2->Release ();
		m_pIAsync2 = NULL;
		}

	if (m_pIConnPtContainer)
		{
		m_pIConnPtContainer->Release ();
		m_pIConnPtContainer = NULL;
		}

	TRACE (_T("OTC: Uninitialized group %s on server %s.\r\n"), 
		GetName (), m_pServer->GetProgID ());
	}
