// **************************************************************************
// safearray.cpp
//
// Description:
//	This class wraps this struct for local useage.  Do not use CSafeArray
//	if the data must be passed to another process via COM.
//
//	typedef struct  tagSAFEARRAY
//		{
//		USHORT cDims;
//		USHORT fFeatures;
//		ULONG cbElements;
//		ULONG cLocks;
//		PVOID pvData;
//		SAFEARRAYBOUND rgsabound[ 1 ];
//		}	SAFEARRAY;
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
#include "safearray.h"


// **************************************************************************
// operator new
//
// Description:
//	Custom new operator that allocates enough memory to manage a single or
//	two dimension array.  No actual array memory is allocated here.
//
// Parameters:
//  none
//
// Returns:
//  void * - Pointer to new CSafeArray object.
// **************************************************************************
void *CSafeArray::operator new (size_t s)
	{
	// SAFEARRAY has enough memory on its own to manage a single dimension
	// array.  If we add one more SAFEARRAYBOUND, then it can manage a second
	// dimension.  For our purposes this is all we will ever need.
	void *p = malloc (sizeof (SAFEARRAY) + sizeof (SAFEARRAYBOUND));

	// Handle allocation error:
	if (!p)
		{
#ifdef _WIN32_WCE
		return (NULL);
#else
		throw (-1);
#endif//_WIN32_WCE
		}
	
	// Initialize allocated memory zero:
	memset (p, 0, sizeof (SAFEARRAY) + sizeof (SAFEARRAYBOUND));

	// Return pointer to allocated memory:
	return (p);
	}

// **************************************************************************
// operator delete
//
// Description:
//	Custom delete operator.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CSafeArray::operator delete (void *p)
	{
	// Free allocated memory:
	if (p)
		free (p);
	}

// **************************************************************************
// CSafeArray ()
//
// Description:
//	Copy constructor.
//
// Parameters:
//  const CSafeArray	&cSrc	CSafeArray object to copy properties from.
//
// Returns:
//  none
// **************************************************************************
CSafeArray::CSafeArray (const CSafeArray &cSrc)
	{
	// Copy properties:
	cDims = cSrc.cDims;
	fFeatures = cSrc.fFeatures;
	cbElements = cSrc.cbElements;
	cLocks = cSrc.cLocks;
	
	rgsabound [0] = cSrc.rgsabound [0];
	rgsabound [1] = cSrc.rgsabound [1];
	
	// Allocate memory:
	DWORD cdwBytes = cSrc.GetByteLength ();
	Alloc (cdwBytes);

	// Copy data into our memory block:
	memcpy (pvData, cSrc.pvData, cdwBytes);
	}

// **************************************************************************
// CSafeArray ()
//
// Description:
//	Constructor for 2-D array.
//
// Parameters:
//  WORD		wRows				Number of rows to allocate.
//	WORD		wCols				Number of columns to allocate.
//	WORD		wBytesPerElement	Number of bytes per array element.
//	void		*pData				Pointer to data to initialize array with.
//									  If NULL, array is not initialized.
//
// Returns:
//  none
// **************************************************************************
CSafeArray::CSafeArray (WORD wRows, WORD wCols, WORD wBytesPerElement, void *pData /*=NULL*/)
	{
	// Initialize member variables:

	// If number of row is 1, then 1-D array, else 2-D:
	cDims = (wRows > 1) ? 2 : 1;
	fFeatures = FADF_FIXEDSIZE;
	cbElements = wBytesPerElement;
	cLocks = 0;
	
	// If a 2-D array:
	if (cDims == 2)
		{
		// We shouldn't have single element arrays
		ASSERT ((DWORD)wRows * (DWORD)wCols > 1);

		// Set bounds for rows:
		rgsabound [0].lLbound = 0;
		rgsabound [0].cElements = wRows;
		
		// Set bounds for columns:
		rgsabound [1].lLbound = 0;
		rgsabound [1].cElements = wCols;
		}

	// Else a 1-D array:
	else
		{
		// Set bounds for columns (no rows):
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = wCols;
		}

	// Allocate memory for data:
	DWORD dwSize = (DWORD)wRows * (DWORD)wCols * (DWORD)wBytesPerElement;
	Alloc (dwSize);

	// Load data into our memory block if data is provided:
	if (pData)
		memcpy (pvData, pData, dwSize);
	}

// **************************************************************************
// CSafeArray
//
// Description:
//	Constructor for 1-D array.
//
// Parameters:
//	DWORD		cdwElements			Number of elements to allocate.
//	WORD		wBytesPerElement	Number of bytes per array element.
//	void		*pData				Pointer to data to initialize array with.
//									  If NULL, array is not initialized.
//
// Returns:
//  none
// **************************************************************************
CSafeArray::CSafeArray (DWORD cdwElements, WORD wBytesPerElement, void *pData /*=NULL*/)
	{
	// We shouldn't have single element arrays:
	ASSERT (cdwElements > 1);

	// Initialize member variables:
	cDims = 1;
	fFeatures = FADF_FIXEDSIZE;
	cbElements = wBytesPerElement;
	cLocks = 0;
	
	// Set bounds:
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = cdwElements;
	
	// Allocate memory for data:
	Alloc (cdwElements * wBytesPerElement);

	// Load data into our memory block if data is provided:
	if (pData)
		memcpy (pvData, pData, cdwElements * wBytesPerElement);
	}

// **************************************************************************
// ~CSafeArray ()
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
CSafeArray::~CSafeArray ()
	{
	// Free memory we allocated for data:
	if (pvData)
		free (pvData);
	}

// **************************************************************************
// operator =
//
// Description:
//	Custom assignment operator.
//
// Parameters:
//  const SAFEARRAY		&cSrc		Object to assign properties from.
//
// Returns:
//  void
// **************************************************************************
void CSafeArray::operator = (const SAFEARRAY &cSrc)
	{
	ASSERT (cSrc.cDims <= 2);

	// Get size of array to copy:
	DWORD dwNewSize = ((CSafeArray &)cSrc).GetByteLength ();

	// Save our current size:
	DWORD dwOldSize = GetByteLength ();

	// If size is different, dimensioning may be different as well:
	if (dwNewSize != dwOldSize)
		{
		// If new size is larger than our current size, we will
		// need to allocate a larger block of memory for data:f
		if (dwNewSize > dwOldSize)
			{
			// Free old memory:
			if (pvData)
				{
				free (pvData);
				pvData = NULL;
				}

			// Allocate new memory:
			Alloc (dwNewSize);
			}

		// Assign basic properties:
		cDims = cSrc.cDims;
		fFeatures = FADF_FIXEDSIZE;
		cbElements = cSrc.cbElements;
		cLocks = 0;

		// Set bound "0" for 1-D and 2-D arrays:
		rgsabound [0] = cSrc.rgsabound [0];

		// Set bound "1" for 2-D arrays only:
		if (cSrc.cDims == 2)
			rgsabound [1] = cSrc.rgsabound [1];
		}

	// Copy the array data:
	memcpy (pvData, cSrc.pvData, dwNewSize);
	}

// **************************************************************************
// Alloc ()
//
// Description:
//	Called to allocate memory for array.
//
// Parameters:
//  DWORD		cdwBytes	Number of bytes to allocate.
//
// Returns:
//  void
// **************************************************************************
void CSafeArray::Alloc (DWORD cdwBytes)
	{
	ASSERT (!pvData);
	ASSERT (cdwBytes);

	// Allocate specified number of bytes.  If succeed, indicated by an
	// non=NULL pointer return, then initialize the memory to zero:
	if (pvData = malloc (cdwBytes))
		{
		memset (pvData, 0, cdwBytes);
		return;
		}

	// If we get here, there was an allocation error.
#ifdef _WIN32_WCE
	AfxThrowUserException ();
#else
	throw (-1);
#endif //_WIN32_WCE
	}

// **************************************************************************
// GetByteLength ()
//
// Description:
//	Called to get size of array.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - size of array in bytes.
// **************************************************************************
DWORD CSafeArray::GetByteLength () const
	{
	// Return the array length in bytes (number of elements times the
	// number of bytes per element):
	return (GetNumElements () * (DWORD)cbElements);
	}

// **************************************************************************
// GetNumElements ()
//
// Description:
//	Called to get the number of elements in array.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - number of elements.
// **************************************************************************
DWORD CSafeArray::GetNumElements () const
	{
	// We assume 1-D or 2-D.  Check for debugging:
	ASSERT (cDims > 0 && cDims < 3);

	// Calculate the number of elements (product of dimension bounds)
	DWORD dwSize = 1;
	for (unsigned short i = 0; i < cDims; i++)
		dwSize *= rgsabound [i].cElements;

	// Return result:
	return (dwSize);
	}

// **************************************************************************
// GetNumRows ()
//
// Description:
//	Called to get number of rows in array.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - number of rows.
// **************************************************************************
DWORD CSafeArray::GetNumRows () const
	{
	// We assume 1-D or 2-D.  Check for debugging:
	ASSERT (cDims > 0 && cDims < 3);
	
	// If 1-D, data is stored as columns (1 row implied):
	if (cDims == 1)
		return (1);

	// else if 2-D , dimension 0 is rows:
	return (rgsabound [0].cElements);
	}

// **************************************************************************
// GetNumCols ()
//
// Description:
//	Called to get number of columns in array.
//
// Parameters:
//  none
//
// Returns:
//  DWORD - number of columns.
// **************************************************************************
DWORD CSafeArray::GetNumCols () const
	{
	// We assume 1-D or 2-D.  Check for debugging:
	ASSERT (cDims > 0 && cDims < 3);
	
	// If 1-D, dimension 0 is columns:
	if (cDims == 1)
		return (rgsabound [0].cElements);

	// else if 2-D, dimension 1 is columns:
	return (rgsabound [1].cElements);
	}

// **************************************************************************
// Format ()
//
// Description:
//	Formats the array to a tab delimited string, in row major order where 
//	each row (except the last) is terminated with a CR-LF.
//
// Parameters:
//  CString		&str	Output string with tab delimited data.
//	VARTYPE		vt		Variant used to specify data type.
//
// Returns:
//  void
// **************************************************************************
void CSafeArray::Format (CString &str, VARTYPE vt) const
	{
	// Get array size in bytes:
	DWORD dwSize = GetByteLength ();

	// Get number of columns (dimension 0 if 1-D, dimension 1 if 2-D):
	DWORD dwCols = cDims == 1 ? rgsabound [0].cElements : rgsabound [1].cElements;

	// Clear output string:
	str.Empty ();

	// Get pointer to first byte if data:
	LPBYTE lpByte = (LPBYTE)pvData;
	DWORD dwCol = 0;

	// Clear array bit from the data type:
	vt &= ~VT_ARRAY;


	// Loop over bytes (increment loop index by bytes per element and 
	// increment data pointer by same):
	for (DWORD i = 0; i < dwSize; i += cbElements, lpByte += cbElements)
		{
		// Create a scratch buffer to contain formatted element value:
		TCHAR szNum [32];

		// Single byte elements (byte, char):
		if (cbElements == 1)
			{
			if (vt == VT_UI1)
				_stprintf (szNum, _T("%u"), *lpByte);
			else
				_stprintf (szNum, _T("%d"), *(char *)lpByte);
			}
		
		// Two-byte elements (short, word):
		else if (cbElements == 2)
			{
			if (vt == VT_I2)
				_stprintf (szNum, _T("%d"), *(short *)lpByte);
			else
				_stprintf (szNum, _T("%u"), *(WORD *)lpByte);
			}
		
		// Four-byte elements (long, float, dword):
		else if (cbElements == 4)
			{
			if (vt == VT_I4)
				_stprintf (szNum, _T("%d"), *(long *)lpByte);
			else if (vt == VT_R4)
				_stprintf (szNum, _T("%G"), *(float *)lpByte);
			else
				_stprintf (szNum, _T("%u"), *(DWORD *)lpByte);
			}

		// Eight-byte elements (double):
		else if (cbElements == 8)
			_stprintf (szNum, _T("%G"), *(double *)lpByte);

		// Bad format - return:
		else
			{
#ifdef _DEBUG
			str = _T("Unknown array format");
#endif
			return;
			}

		// Delimit each element within the row
		if (dwCol != 0)
			str += _T('\t');

		// Append the formatted element data
		str += szNum;

		// Terminate each row 
		if (++dwCol == dwCols)
			{
			str += _T("\r\n");
			dwCol = 0;
			}
		}
	}

// **************************************************************************
// operator ==
//
// Description:
//	Custom comparison operator.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
bool CSafeArray::operator == (const SAFEARRAY &cSrc) const
	{
	// If neither object is initialized, objects are the same so return true.
	if (!cSrc.pvData && !pvData)
		return (true);

	// If one object is initialized, but not the other, the objects
	// different so return false.
	if ((!cSrc.pvData && pvData) || (cSrc.pvData && !pvData))
		return (false);

	// If we make it here, we know both objects are initialized.  We need
	// to look more closely to see if they are the same.
	
	// Get size of both arrays:
	DWORD dwSrcLen = ((CSafeArray &)cSrc).GetByteLength ();
	DWORD dwLen = GetByteLength ();

	// If lengths are different, the objects are different so return false.
	if (dwSrcLen != dwLen)
		return (false);

	// Compare the memory as a vector (ignore dimensions, size of 
	// element etc.).  Return result.
	return (memcmp (cSrc.pvData, pvData, GetByteLength ()) == 0);
	}



