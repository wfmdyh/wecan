// **************************************************************************
// item.cpp
//
// Description:
//	Implements the CKItem class.  On object of this class is associated with
//	each OPC item we wish to exchange data with.
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
#include "item.h"
#include "group.h"

#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "oleaut32.lib") 

#define VERSION_1			1
#define CURRENT_VERSION		VERSION_1


// **************************************************************************
// String container object for items
// **************************************************************************
class CStringContainer
	{
	public:
		// Constructor:
		CStringContainer ()
			{
			LPCTSTR lpszDefault = _T("");

			// Initialize member variables.  Strings set to NULL string.
			m_lpszQualityBad = lpszDefault;
			m_lpszQualityBadConfigError = lpszDefault;
			m_lpszQualityBadNotConnected = lpszDefault;
			m_lpszQualityBadDeviceFailure = lpszDefault;
			m_lpszQualityBadSensorFailure = lpszDefault;
			m_lpszQualityBadLastKnownValue = lpszDefault;
			m_lpszQualityBadCommFailure = lpszDefault;
			m_lpszQualityBadOutOfService = lpszDefault;
			m_lpszQualityUncertain = lpszDefault;
			m_lpszQualityUncertainLastUsableValue = lpszDefault;
			m_lpszQualityUncertainSensorNotAccurate = lpszDefault;
			m_lpszQualityUncertainEUExceeded = lpszDefault;
			m_lpszQualityUncertainSubnormal = lpszDefault;
			m_lpszQualityGood = lpszDefault;
			m_lpszQualityGoodLocalOverride = lpszDefault;
			m_lpszQualityNoValue = lpszDefault;

			m_lpszUnknown = lpszDefault;
			m_lpszNonApplicable = lpszDefault;

			m_bInitialized = false;
			}

		// Initialize container object.
		void Initialize ()
			{
			// Load string resources and assign public pointers if we haven't
			// already done so:
			if (!m_bInitialized)
				{
				// Load quality string resources:
				m_strQualityBad.LoadString (IDS_QUALITY_BAD);
				m_strQualityBadConfigError.LoadString (IDS_QUALITY_BAD_CONFIGERROR);
				m_strQualityBadNotConnected.LoadString (IDS_QUALITY_BAD_NOTCONNECTED);
				m_strQualityBadDeviceFailure.LoadString (IDS_QUALITY_BAD_DEVICEFAILURE);
				m_strQualityBadSensorFailure.LoadString (IDS_QUALITY_BAD_SENSORFAILURE);
				m_strQualityBadLastKnownValue.LoadString (IDS_QUALITY_BAD_LASTKNOWNVALUE);
				m_strQualityBadCommFailure.LoadString (IDS_QUALITY_BAD_COMMFAILURE);
				m_strQualityBadOutOfService.LoadString (IDS_QUALITY_BAD_OUTOFSERVICE);
				m_strQualityUncertain.LoadString (IDS_QUALITY_UNCERTAIN);
				m_strQualityUncertainLastUsableValue.LoadString (IDS_QUALITY_UNCERTAIN_LASTUSEABLEVALUE);
				m_strQualityUncertainSensorNotAccurate.LoadString (IDS_QUALITY_UNCERTAIN_SENSORNOTACCURATE);
				m_strQualityUncertainEUExceeded.LoadString (IDS_QUALITY_UNCERTAIN_EUEXCEEDED);
				m_strQualityUncertainSubnormal.LoadString (IDS_QUALITY_UNCERTAIN_SUBNORMAL);
				m_strQualityGood.LoadString (IDS_QUALITY_GOOD);
				m_strQualityGoodLocalOverride.LoadString (IDS_QUALITY_GOOD_LOCALOVERRIDE);
				m_strQualityNoValue.LoadString (IDS_QUALITY_NOVALUE);

				// Assign to public pointers:
				m_lpszQualityBad = m_strQualityBad;
				m_lpszQualityBadConfigError = m_strQualityBadConfigError;
				m_lpszQualityBadNotConnected = m_strQualityBadNotConnected;
				m_lpszQualityBadDeviceFailure = m_strQualityBadDeviceFailure;
				m_lpszQualityBadSensorFailure = m_strQualityBadSensorFailure;
				m_lpszQualityBadLastKnownValue = m_strQualityBadLastKnownValue;
				m_lpszQualityBadCommFailure = m_strQualityBadCommFailure;
				m_lpszQualityBadOutOfService = m_strQualityBadOutOfService;
				m_lpszQualityUncertain = m_strQualityUncertain;
				m_lpszQualityUncertainLastUsableValue = m_strQualityUncertainLastUsableValue;
				m_lpszQualityUncertainSensorNotAccurate = m_strQualityUncertainSensorNotAccurate;
				m_lpszQualityUncertainEUExceeded = m_strQualityUncertainEUExceeded;
				m_lpszQualityUncertainSubnormal = m_strQualityUncertainSubnormal;
				m_lpszQualityGood = m_strQualityGood;
				m_lpszQualityGoodLocalOverride = m_strQualityGoodLocalOverride;
				m_lpszQualityNoValue = m_strQualityNoValue;

				// Load unknown string resource:
				m_strUnknown.LoadString (IDS_UNKNOWN);
				m_lpszUnknown = m_strUnknown;

				// Load non-applicable string resource:
				m_strNonApplicable.LoadString (IDS_NONAPPLICABLE);
				m_lpszNonApplicable = m_strNonApplicable;
				
				// Set initialized flag so we don't waste time loading string 
				// resources if called again.
				m_bInitialized = true;
				}
			}

	public:
		// Quality strings:
		LPCTSTR m_lpszQualityBad;
		LPCTSTR m_lpszQualityBadConfigError;
		LPCTSTR m_lpszQualityBadNotConnected;
		LPCTSTR m_lpszQualityBadDeviceFailure;
		LPCTSTR m_lpszQualityBadSensorFailure;
		LPCTSTR m_lpszQualityBadLastKnownValue;
		LPCTSTR m_lpszQualityBadCommFailure;
		LPCTSTR m_lpszQualityBadOutOfService;
		LPCTSTR m_lpszQualityUncertain;
		LPCTSTR m_lpszQualityUncertainLastUsableValue;
		LPCTSTR m_lpszQualityUncertainSensorNotAccurate;
		LPCTSTR m_lpszQualityUncertainEUExceeded;
		LPCTSTR m_lpszQualityUncertainSubnormal;
		LPCTSTR m_lpszQualityGood;
		LPCTSTR m_lpszQualityGoodLocalOverride;
		LPCTSTR m_lpszQualityNoValue;

		LPCTSTR m_lpszUnknown;
		LPCTSTR m_lpszNonApplicable;

	private:
		// Quality strings:
		CString m_strQualityBad;
		CString m_strQualityBadConfigError;
		CString m_strQualityBadNotConnected;
		CString m_strQualityBadDeviceFailure;
		CString m_strQualityBadSensorFailure;
		CString m_strQualityBadLastKnownValue;
		CString m_strQualityBadCommFailure;
		CString m_strQualityBadOutOfService;
		CString m_strQualityUncertain;
		CString m_strQualityUncertainLastUsableValue;
		CString m_strQualityUncertainSensorNotAccurate;
		CString m_strQualityUncertainEUExceeded;
		CString m_strQualityUncertainSubnormal;
		CString m_strQualityGood;
		CString m_strQualityGoodLocalOverride;
		CString m_strQualityNoValue;
		
		// Unknown:
		CString m_strUnknown;
		CString m_strNonApplicable;

		bool m_bInitialized;
	
	} cStringContainer;


/////////////////////////////////////////////////////////////////////////////
// CKItem construction/destruction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKItem ()
//
// Description:
//	Constructor
//
// Parameters:
//  CKGroup		*pParent	Pointer to parent group.
//
// Returns:
//  none
// **************************************************************************
CKItem::CKItem (CKGroup *pParent)
	{
	// Initialize string container so we can return strings:
	cStringContainer.Initialize ();

	// Initialize other member variables:
	ASSERT (pParent != NULL);
	m_pGroup = pParent;

	m_bActive			= FALSE;
	m_vtDataType		= VT_EMPTY;
	m_dwAccessRights	= OPC_READABLE;

	m_bValid			= FALSE;
	m_bTimeStamped		= FALSE;

	ZeroMemory (&m_bfFlags, sizeof (m_bfFlags));
	
	GetSystemTimeAsFileTime (&m_ftTimeStamp);
	VariantInit (&m_vtValue);

	m_wQuality	= OPC_QUALITY_BAD_OUT_OF_SERVICE;
	m_cdwUpdates = 0;

	m_hServer	= NULL;
	m_pPrev		= NULL;
	m_pNext		= NULL;

	m_wParam	= 0;
	}

// **************************************************************************
// ~CKItem ()
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
CKItem::~CKItem ()
	{
	// Make sure our variant members get cleaned up properly:
	VariantClear (&m_vtValue);
	}


/////////////////////////////////////////////////////////////////////////////
// CKItem manipulators
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// UpdateData ()
//
// Description:
//	Called to update the item's data and quality.
//
// Parameters:
//  VARIANT		&vtVal		Item's data.
//	WORD		wQuality	Item's quality.
//
// Returns:
//  void
// **************************************************************************
void CKItem::UpdateData (VARIANT &vtVal, WORD wQuality)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// Update item value:
	VariantCopy (&m_vtValue, &vtVal);

	// Update item quality:
	m_wQuality = wQuality;

	// This function is used for non-timestamped data:
	m_bTimeStamped = FALSE;

	// Increment update count:
	++m_cdwUpdates;
	}

// **************************************************************************
// UpdateData ()
//
// Description:
//	Called to update the item's data, quality and timestamp.
//
// Parameters:
//  VARIANT		&vtVal			Item's data.
//	WORD		wQuality		Item's quality.
//	FILETIME	&ftTimeStamp	Item's timestap.
//
// Returns:
//  void
// **************************************************************************
void CKItem::UpdateData (VARIANT &vtVal, WORD wQuality, FILETIME &ftTimeStamp)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// Update item value:
	VariantCopy (&m_vtValue, &vtVal);

	// Update item quality:
	m_wQuality = wQuality;

	// Update item timestamp:
	m_bTimeStamped = TRUE;
	m_ftTimeStamp = ftTimeStamp;

	// Increment update count:
	++m_cdwUpdates;
	}

// **************************************************************************
// GetValue ()
//
// Description:
//	Returns a string representation of the current value.
//
// Parameters:
//  CString		&strValue	Output string.
//
// Returns:
//  void
// **************************************************************************
void CKItem::GetValue (CString &strValue)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// If item is active and valid, we can rely on server's data:
	if (IsValid ())
		{
		TCHAR szNum [32];

		// If data item is an array:
		if (m_vtValue.vt & VT_ARRAY)
			{
			// Format output string:
			switch (m_vtValue.vt)
				{
				// Supported types:
				case VT_UI1	| VT_ARRAY:
				case VT_I1	| VT_ARRAY:
				case VT_UI2	| VT_ARRAY:
				case VT_I2	| VT_ARRAY:
				case VT_UI4	| VT_ARRAY:
				case VT_I4	| VT_ARRAY:
				case VT_R4	| VT_ARRAY:
				case VT_R8	| VT_ARRAY:
					{
					// Get pointer to array data:
					CSafeArray *pSafeArr = (CSafeArray *) m_vtValue.parray;

					// Process array data if pointer appears to be good:
					if (pSafeArr)
						{
						// Get the array dimensions:
						DWORD dwCols = pSafeArr->GetNumCols ();
						DWORD dwSize = pSafeArr->GetByteLength ();
						ULONG cbElements = pSafeArr->cbElements;
						LPBYTE lpByte = (LPBYTE)pSafeArr->pvData;
						DWORD dwCol = 0;

						// Start delimiter out putput string:
						strValue = _T("[ ");

						// Cycle through the elements:
						for (DWORD i = 0; i < dwSize; i += cbElements, lpByte += cbElements)
							{
							// Format a string for each element according to data type:

							// Single-byte types:
							if (cbElements == 1)
								{
								if (m_vtValue.vt ==	(VT_UI1 | VT_ARRAY))
									_stprintf (szNum, _T("%u"), *lpByte);
								else
									_stprintf (szNum, _T("%d"), *(char *)lpByte);
								}
							
							// Two-byte types:
							else if (cbElements == 2)
								{
								if (m_vtValue.vt ==	(VT_UI2 | VT_ARRAY))
									_stprintf (szNum, _T("%u"), *(WORD *)lpByte);
								else 
									_stprintf (szNum, _T("%d"), *(short *)lpByte);
								}
							
							// Four-byte types:
							else if (cbElements == 4)
								{
								if (m_vtValue.vt ==	(VT_R4	| VT_ARRAY))
									_stprintf (szNum, _T("%G"), *(float *)lpByte);
								else if (m_vtValue.vt ==	(VT_UI4	| VT_ARRAY))
									_stprintf (szNum, _T("%u"), *(DWORD *)lpByte);
								else if (m_vtValue.vt ==	(VT_I4	| VT_ARRAY))
									_stprintf (szNum, _T("%d"), *(DWORD *)lpByte);
								}

							// Eight-byte types:
							else if (cbElements == 8)
								_stprintf (szNum, _T("%G"), *(double *)lpByte);

							// Else something is foobar!
							else
								{
								ASSERT (FALSE);
								}

							// Delimit each element within the row:
							if (dwCol != 0)
								strValue += _T(", ");

							// Append the formatted element data:
							strValue += szNum;

							// Terminate each row (except the last):
							if (++dwCol == dwCols)
								{
								if (i < dwSize - cbElements) 
									strValue += _T(" ] [ ");

								dwCol = 0;
								}
							}

						// End delimiter:
						strValue += _T(" ]");
						}

					// Else bad array data:
					else
						{
						strValue = _T("???");
						LogMsg (IDS_BADITEMARRAYDATA, GetItemID ());
						}
					}
					break;

				default:
					strValue = cStringContainer.m_lpszUnknown;
					break;
				}
			}
		
		// Else if data item is not an array:
		else
			{
			// Format output string according to data type:
			switch (m_vtValue.vt)
				{
				case VT_BOOL:
					wsprintf (szNum, _T("%d"), m_vtValue.boolVal ? 1 : 0);
					break;

				case VT_UI1:
					wsprintf (szNum, _T("%u"), m_vtValue.bVal);
					break;

				case VT_I1:
					wsprintf (szNum, _T("%d"), m_vtValue.cVal);
					break;

				case VT_UI2:
					wsprintf (szNum, _T("%u"), m_vtValue.uiVal);
					break;

				case VT_I2:
					wsprintf (szNum, _T("%d"), m_vtValue.iVal);
					break;

				case VT_UI4:
					wsprintf (szNum, _T("%u"), m_vtValue.ulVal);
					break;

				case VT_I4:
					wsprintf (szNum, _T("%d"), m_vtValue.lVal);
					break;

				case VT_R4:
					_stprintf (szNum, _T("%G"), m_vtValue.fltVal);
					break;

				case VT_R8:
					_stprintf (szNum, _T("%G"), m_vtValue.dblVal);
					break;

				case VT_BSTR:
					strValue = m_vtValue.bstrVal;
					return;

				default:
					strValue = cStringContainer.m_lpszUnknown;
					return;
				}

			strValue = szNum;
			}
		}

	// Else if item is inactive and/or invalid set value accordingly:
	else
		{
		strValue = cStringContainer.m_lpszUnknown;
		}
	}

// **************************************************************************
// GetQuality ()
//
// Description:
//	Returns a string representation of the current quality.
//
// Parameters:
//  none
//
// Returns:
//  LPCTSTR	- Pointer to quality string.
// **************************************************************************
LPCTSTR CKItem::GetQuality ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// If item is active an valid, we can rely on server's data:
	if (IsValid ())
		{
		// Quality bits arranged QQSSSLL (each letter a bit).

		// Parse quality bits (omit limit bits) and output appropriate string:
		switch (m_wQuality & 0xFC)
			{
			case OPC_QUALITY_BAD_NON_SPECIFIC:
				return (cStringContainer.m_lpszQualityBad);

			case OPC_QUALITY_BAD_CONFIG_ERR0R:
				return (cStringContainer.m_lpszQualityBadConfigError);

			case OPC_QUALITY_BAD_NOT_CONNECTED:
				return (cStringContainer.m_lpszQualityBadNotConnected);

			case OPC_QUALITY_BAD_DEVICE_FAILURE:
				return (cStringContainer.m_lpszQualityBadDeviceFailure);

			case OPC_QUALITY_BAD_SENSOR_FAILURE:
				return (cStringContainer.m_lpszQualityBadSensorFailure);

			case OPC_QUALITY_BAD_LAST_KNOWN_VALUE:
				return (cStringContainer.m_lpszQualityBadLastKnownValue);

			case OPC_QUALITY_BAD_COMM_FAILURE:
				return (cStringContainer.m_lpszQualityBadCommFailure);

			case OPC_QUALITY_BAD_OUT_OF_SERVICE:
				return (cStringContainer.m_lpszQualityBadOutOfService);

			case OPC_QUALITY_UNCERTAIN_NON_SPECIFIC:
				return (cStringContainer.m_lpszQualityUncertain);

			case OPC_QUALITY_UNCERTAIN_LAST_USABLE_VALUE:
				return (cStringContainer.m_lpszQualityUncertainLastUsableValue);

			case OPC_QUALITY_UNCERTAIN_SENSOR_NOT_ACCURATE:
				return (cStringContainer.m_lpszQualityUncertainSensorNotAccurate);

			case OPC_QUALITY_UNCERTAIN_EU_UNITS_EXCEEDED:
				return (cStringContainer.m_lpszQualityUncertainEUExceeded);

			case OPC_QUALITY_UNCERTAIN_SUB_NORMAL:
				return (cStringContainer.m_lpszQualityUncertainSubnormal);

			case OPC_QUALITY_GOOD_NON_SPECIFIC:
				return (cStringContainer.m_lpszQualityGood);

			case OPC_QUALITY_GOOD_LOCAL_OVERRIDE:
				return (cStringContainer.m_lpszQualityGoodLocalOverride);

			default:
				return (cStringContainer.m_lpszQualityNoValue);
			}
		}

	// If we make it here, then item is invalid.  Return appropriate string:
	return (cStringContainer.m_lpszQualityNoValue);
	}

// **************************************************************************
// GetTimeStamp ()
//
// Description:
//	Returns a string representation of the current timestamp.
//
// Parameters:
//  CString		&strTimeStamp	Output string.
//
// Returns:
//  void
// **************************************************************************
void CKItem::GetTimeStamp (CString &strTimeStamp)
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// Output timestamp if we are infact timestamped:
	if (m_bTimeStamped)
		{
		FILETIME ftLocal;

		// Convert raw timestamp from file time to local file time.
		// Proceed with next step in conversion only if successful.
		if (FileTimeToLocalFileTime (&m_ftTimeStamp, &ftLocal))
			{
			SYSTEMTIME systime;
			TCHAR szTime [64];

			// Convert file time to system time:
			if (FileTimeToSystemTime (&ftLocal, &systime))
				{
				// Format system time string:
				wsprintf (szTime, _T("%02d:%02d:%02d:%03d"), 
					systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);			
				
				// Assign formatted system time string to output string:
				strTimeStamp = szTime;
				}
			
			// Conversion failed:
			else
				{
				ASSERT (FALSE);
				}
			}

		// Conversion failed:
		else
			{
			ASSERT (FALSE);
			}
		}

	// Else we are not timestamped so set timestamp to non-applicable:
	else
		strTimeStamp = cStringContainer.m_lpszNonApplicable;
	}

// **************************************************************************
// GetUpdateCount ()
//
// Description:
//	Returns the number of updates the item has recieved.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - Current update count.
// **************************************************************************
DWORD CKItem::GetUpdateCount ()
	{
	// Create a CSafeLock to make this object thread safe.  Our critical
	// section gets locked here, and will automatically be unlocked when the
	// CSafeLock goes out of scope.
	CSafeLock cs (&m_csDataLock);

	// Return current update count:
	return (m_cdwUpdates);
	}


/////////////////////////////////////////////////////////////////////////////
// CKItem serialization
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Serialize ()
//
// Description:
//	Save or load item settings.
//
// Parameters:
//  CArchive		&ar			The archive to save or load item settings.
//
// Returns:
//  void
// **************************************************************************
void CKItem::Serialize (CArchive &ar)
	{
	// Saving object information:
	if (ar.IsStoring ())
		{
		// Save verion first so we will know how to read information later.
		// If format of data needs to change at some point, say we need
		// to add another property, then bump up the version number.
		ar << CURRENT_VERSION;						// current version

		// Save item access path:
		ar << m_strAccessPath;

		// Save full qualified item ID:
		ar << m_strItemID;

		// Save item active state:
		ar << m_bActive;

		// Save item data type:
		ar << m_vtDataType;

		// Save item access rights:
		ar << m_dwAccessRights;

		// Save flag bit field (we can add flags, for up to a total of 32 in
		// this case, without having to change version):
		ar.Write (&m_bfFlags, sizeof (m_bfFlags));
		}

	// Else loading object information:
	else
		{
		// Read version of saved information:
		DWORD dwSchema;
		ar >> dwSchema;

		// Read data according to version:
		switch (dwSchema)
			{
			case VERSION_1:
				ar >> m_strAccessPath;						// access path
				ar >> m_strItemID;							// fully qualified item ID
				ar >> m_bActive;							// active state
				ar >> m_vtDataType;							// data type
				ar >> m_dwAccessRights;						// access rights
				ar.Read (&m_bfFlags, sizeof (m_bfFlags));	// flags
				break;

			default:
				// Unexpected version.  Self-delete and throw achive exception:
				delete this;
				AfxThrowArchiveException (CArchiveException::badSchema);
				break;
			}
		}
	}

// **************************************************************************
// Copy ()
//
// Description:
//	Copy an instance of this class.  (Save properties to a shared memory file.)
//
// Parameters:
//  CFixedSharedFile	&sf		Shared memory file to copy properties to.
//
// Returns:
//  void
// **************************************************************************
void CKItem::Copy (CFixedSharedFile &sf)
	{
	int nLen;

	// To save a string to a shared memory file, we need to first save the
	// string length.  (CArchive objects, as in Serialize(), has this logic
	// built in.)

	// Save access path string:
	nLen = m_strAccessPath.GetLength ();
	sf.Write (&nLen, sizeof (nLen));
	sf.Write (m_strAccessPath, nLen * sizeof (TCHAR));

	// Save item ID string:
	nLen = m_strItemID.GetLength ();
	sf.Write (&nLen, sizeof (nLen));
	sf.Write (m_strItemID, nLen * sizeof (TCHAR));

	// Save active state:
	sf.Write (&m_bActive, sizeof (m_bActive));

	// Save data type:
	sf.Write (&m_vtDataType, sizeof (m_vtDataType));

	// Save access rights:
	sf.Write (&m_dwAccessRights, sizeof (m_dwAccessRights));

	// Save flag bit field:
	sf.Write (&m_bfFlags, sizeof (m_bfFlags));
	}

// **************************************************************************
// Paste ()
//
// Description:
//	Paste an instance of this class.  (Load properties from a shared memory file.)
//
// Parameters:
//  CFixedSharedFile	&sf		Shared memory file to paste properties from.
//
// Returns:
//  void
// **************************************************************************
void CKItem::Paste (CFixedSharedFile &sf)
	{
	int nLen;

	// To save a string to a shared memory file, we need to first save the
	// string length.  (CArchive objects, as in Serialize(), has this logic
	// built in.)  So to read the string back, we read its length first:

	// Read access path string:
	sf.Read (&nLen, sizeof (nLen));
	sf.Read (m_strAccessPath.GetBufferSetLength (nLen), nLen * sizeof (TCHAR));

	// Read item ID string:
	sf.Read (&nLen, sizeof (nLen));
	sf.Read (m_strItemID.GetBufferSetLength (nLen), nLen * sizeof (TCHAR));

	// Read active state:
	sf.Read (&m_bActive, sizeof (m_bActive));

	// Read data type:
	sf.Read (&m_vtDataType, sizeof (m_vtDataType));

	// Read access rights:
	sf.Read (&m_dwAccessRights, sizeof (m_dwAccessRights));

	// Read flag bit field:
	sf.Read (&m_bfFlags, sizeof (m_bfFlags));
	}
