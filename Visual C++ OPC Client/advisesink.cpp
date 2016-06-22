// **************************************************************************
// advisesink.cpp
//
// Description:
//	Implements the CKAdviseSink class and IAdviseSink COM interface.  Groups
//	in OPC Servers can use the IAdviseSink interface to advise us of data
//  change events.
//
//	OPC 1.0 requires that only the OnDataChange member function of the 
//	IAdviseSink interface be implemented. None of the others (OnClose, 
//	OnSave etc.) should ever get called.
//
//	The IAdviseSink interface is used by OPC version 1.0.  OPC version 2.0
//	uses the IOPCDataCallback interface instead.  If you know that your 
//  server(s) will only be using OPC version 2.0, you don't really need this.
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
#include "advisesink.h"
#include "item.h"
#include "group.h"


// **************************************************************************
// CKAdviseSink ()
//
// Description:
//	Constructor.  Reference count is initialized to zero.
//
// Parameters:
//	none
//
// Returns:
//	none
// **************************************************************************
CKAdviseSink::CKAdviseSink () : m_cRef (0)
	{
	}

// **************************************************************************
// QueryInterface ()
//
// Description:
//	This function is called to obtain a pointer to one of the COM interfaces
//	objects of this class support (in this case only IAdviseSink and its base
//	class IUnknown).  As is standard COM practice, our reference count is 
//	incremented upon a successful query.  
//
//  This is one of the 3 member functions all COM interfaces must implement.	
//
// Parameters:
//	REFIID		riid			Requested interface type: IID_IUnknown,
//								  or IID_IAdviseSink.
//	LPVOID		*ppInterface	Pointer to requested interface.
//
// Returns:
//	STDMETHODIMP -
//		S_OK - Query successful, ppInterface set to requested pointer.
//		E_INVALIDARG - One of the arguments was invalid.
//		E_NOINTERFACE - Requested interface is not supported.
// **************************************************************************
STDMETHODIMP CKAdviseSink::QueryInterface (REFIID riid, LPVOID *ppInterface)
	{
	// Validate ppInterface.  Return with "invalid argument" error code if invalid:
	if (ppInterface == NULL)
		return (E_INVALIDARG);

	// Standard COM practice requires that we invalidate output arguments
	// if an error is encountered.  Let's assume an error for now and invalidate
	// ppInterface.  We will reset it to a valid interface pointer later if we
	// determine requested ID is valid:
	*ppInterface = NULL;

	// Reset ppInterface if requested interface type is valid:
	if (riid == IID_IUnknown)
		*ppInterface = (IUnknown*) this;
	else if (riid == IID_IAdviseSink)
		*ppInterface = (IAdviseSink*) this;
	else
		{
		// We have been asked for an interface we don't support.  Return 
		// immediately with "no interface" error code.  ppInterface should
		// still be NULL as required by COM.
		return (E_NOINTERFACE);
		}

	// If we made it here, then the query was successful and ppInterface
	// has been set to requested interface pointer.  Standard COM practice
	// requires us to increment our reference count now.
	AddRef ();

	// Return with "success" code:
	return (S_OK);
	}

// **************************************************************************
// AddRef ()
//
// Description:
//	This function is called to increment our reference count.  Caller should 
//	increment our reference count each time a new pointer to this interface
//	is created (except when obtained by a call to QueryInterface which will
//  bump the reference count on its behalf). 
//
//	This is one of the 3 member functions all COM interfaces must implement.  
//
// Parameters:
//	none
//
// Returns:
//	STDMETHODIMP_(ULONG) - Reference count resulting from this call.
// **************************************************************************
STDMETHODIMP_(ULONG) CKAdviseSink::AddRef ()
	{
	// Increment the reference count then return value:
	return (++m_cRef);
	}

// **************************************************************************
// Release ()
//
// Description:
//	This function is called to decrement our reference count.  Caller should
//	decrement our reference count just before each pointer to this interface 
//	is destroyed (goes out of scope).  Standard COM practice requires us to
//  self-delete once the refernce count returns to zero.
//
//	This is one of the 3 member functions all COM interfaces must implement. 
//
// Parameters:
//	none
//
// Returns:
//	STDMETHODIMP_(ULONG) - Reference count resulting from this call.
// **************************************************************************
STDMETHODIMP_(ULONG) CKAdviseSink::Release ()
	{
	// Decrement reference count and return immediately if not zero:
	if (--m_cRef > 0)
		return (m_cRef);

	// If we make it here, then the reference count is zero.  We are 
	// therefore obliged to delete ourselves:
	delete this;

	// Return our refence count, which is zero now:
	return (0);
	}

// **************************************************************************
// OnDataChange ()
//
// Description:
//	This function is provided to handle notifications from an OPC Group that
//  one or more of its items we are linked to have data changes.  This will
//  get called after asynchronous read and write requests complete, or as an
//  unsolicited notification from the OPC Server.  (We have not implemented
//	1.0 asynchronous refreshes so we don't expect refresh complete 
//	notifications.)
//
//	This is the only IAdviseSink member function OPC 1.0 requires us to 
//	implement.  None of the others (OnClose, OnSave etc.) should ever get
//	called.
//	
// Parameters:
//	FORMATETC	*pFormatEtc		Format of data in storage medium.
//	STGMEDIUM	*pMedium		Storage medium containing the data.
//
// Returns:
//	STDMETHODIMP_(void)
// **************************************************************************
STDMETHODIMP_(void) CKAdviseSink::OnDataChange (FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
	{
	HRESULT hr;
	IStream *pStream = NULL;
	LARGE_INTEGER dlibMove;

	// Check for valid pointers.  Return if invalid:
	if ((pFormatEtc == NULL) || (pMedium == NULL))
		return;
	
	// Check that type of storage medium is handle to global memory.  Return if not:
	if ((pFormatEtc->tymed != TYMED_HGLOBAL) && (pMedium->tymed != TYMED_HGLOBAL))
		return;

	// Obtain an IStream instance from the global memory handle.
	hr = CreateStreamOnHGlobal (pMedium->hGlobal,	// global memory handle
								TRUE,				// delete memory when stream is release
								&pStream);			// put the stream here

	// Make sure we succeeded.  Release stream and return if not:
	if (FAILED (hr))
		{
		TRACE (_T("OTC: CKAdviseSink - CreateStreamOnHGlobal failed.\r\n"));
		pStream->Release ();
		return;
		}

	// Seek to the beginning of the stream.
	dlibMove.LowPart = dlibMove.HighPart = 0;

	hr = pStream->Seek (dlibMove,			// relative offset set to
					    STREAM_SEEK_SET,	// beginning of the stream
						NULL);				// don't care about the new seek pointer

	// Make sure we succeeded.  Release stream and return if not:
	if (FAILED (hr))
		{
		TRACE (_T("OTC: CKAdviseSink - IStream::Seek Beginning failed.\r\n"));
		pStream->Release ();
		return;
		}

	//////////////////////////////////////////////////	
	// DATA CHANGE WITHOUT TIMESTAMP NOTIFICATION	//
	//////////////////////////////////////////////////
	if (pFormatEtc->cfFormat == CF_DATA_CHANGE)
		{
		OPCGROUPHEADER groupheader;
		OPCITEMHEADER2 itemheader;
		VARIANT vtVal;

		// Read the group header:
		hr = pStream->Read (&groupheader,				// where to put the stream data
							sizeof (OPCGROUPHEADER),	// how much to read
							NULL);						// don't care about how many bytes are read

		// Release stream and return if read failed:
		if (FAILED (hr))
			{
			TRACE (_T("OTC: CKAdviseSink - IStream::Read GROUPHEADER failed.\r\n"));
			pStream->Release ();
			return;
			}

		// Post a "read complete" message if this is a read/refresh transaction
		// complete but not for unsolicited data change events.  Data change 
		// events will have a zero transaction ID.
		if (groupheader.dwTransactionID != 0)
			{
			// The client group handle in the group header structure gives us 
			// a pointer to the destination CKGroup:
			CKGroup *pGroup = (CKGroup *) groupheader.hClientGroup;

			// Wrap use of pGroup in exception handler in case pointer is bad:
			try 
				{
				// Log transaction complete status:
				LogMsg (IDS_ASYNC10_READXACT_COMPLETE, groupheader.dwTransactionID, groupheader.dwItemCount,
					pGroup->GetName (), groupheader.hrStatus);
				}
			catch (...)
				{
				ASSERT (FALSE);
				}
			}

		// Initialize a variant to place a data value in:
		VariantInit (&vtVal);

		// Loop over items in the stream:
		for (DWORD dwItem = 0; dwItem < groupheader.dwItemCount; dwItem++)
			{
			// Compute offset of next item:
			dlibMove.HighPart = 0;
			dlibMove.LowPart = sizeof (OPCGROUPHEADER) + (dwItem * sizeof (OPCITEMHEADER2));

			// Seek to next item:
			hr = pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);

			// If seek failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Seek Next Item failed.\r\n"));
				continue;
				}

			// Read the item header:
			hr = pStream->Read (&itemheader, sizeof (OPCITEMHEADER2), NULL);

			// If read failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Read Item Header failed.\r\n"));
				continue;
				}

			// Seek to beginning of data:
			dlibMove.LowPart = itemheader.dwValueOffset;	
			hr = pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);

			// If seek failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Seek Data failed.\r\n"));
				continue;
				}

			// Read the variant data:
			hr = pStream->Read (&vtVal, sizeof (VARIANT), NULL);

			// If read failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Read Variant failed.\r\n"));
				continue;
				}

			// A little more work for strings and arrays:
			switch (vtVal.vt)
				{
				case VT_BSTR:
					{
					DWORD dwLength = 0;

					// Read string length:
					hr = pStream->Read (&dwLength, sizeof (DWORD), NULL); 
        
					// If read failed, try to get next item:
					if (FAILED (hr))
						{
						TRACE (_T("OTC: CKAdviseSink - IStream::Read string size failed.\r\n"));
						continue;
						}

					if (dwLength == 0)
						vtVal.bstrVal = NULL;
					else 
						{
						// Allocate memory for string.  We will free this once we copy
						// the contents of the string into the item object:
						vtVal.bstrVal = SysAllocStringLen (NULL, dwLength);

						if (vtVal.bstrVal != NULL)
							{
							// Read string data:
							hr = pStream->Read (vtVal.bstrVal, (dwLength + 1) * sizeof (OLECHAR), NULL);

							// If read failed, try to get next item:
							if (FAILED (hr))
								{
								TRACE (_T("OTC: CKAdviseSink - IStream::Read string value failed.\r\n"));
								SysFreeString (vtVal.bstrVal);
								continue;
								}
							}
						}
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
					// Read array parameters:
					SAFEARRAY sa;
					DWORD cdwElements;
					pStream->Read (&sa, sizeof (SAFEARRAY), NULL);

					// 2-D array:
					if (sa.cDims != 1)
						{
						// Read array parameters for second dimension:
						ASSERT (sa.cDims == 2);
						SAFEARRAYBOUND sab [2];

						sab [0] = sa.rgsabound [0];
						pStream->Read (&sab [1], sizeof (SAFEARRAYBOUND), NULL);

						// Compute array size:
						cdwElements = sa.rgsabound [0].cElements * sab [1].cElements;

						// Allocate array (specify basic, i.e. non-array, data type):
						vtVal.parray = SafeArrayCreate (vtVal.vt & ~VT_ARRAY, sa.cDims, sab);
						}

					// 1-D array
					else
						{
						// Compute array size:
						cdwElements = sa.rgsabound [0].cElements;

						// Allocate array (specify basic, i.e. non-array, data type):
						vtVal.parray = SafeArrayCreate (vtVal.vt & ~VT_ARRAY, sa.cDims, sa.rgsabound);
						}

					// Read array data:
					pStream->Read (vtVal.parray->pvData, sa.cbElements * cdwElements, NULL);
					}
					break;

				case VT_BSTR | VT_BYREF:
					ASSERT (FALSE);
					break;
				}

			// The client handle in the item header structure gives us a
			// pointer to the destination CKItem:
			CKItem *pItem = (CKItem *) itemheader.hClient;

			// Wrap use of pItem in exception handler in case pointer is bad:
			try
				{
				// Fill the item's data and quality:
				pItem->UpdateData (vtVal, itemheader.wQuality);
				}
			catch (...)
				{
				TRACE (_T("OTC: Server returned bad item data in advise stream (no timestamp)\r\n"));
				}

			// Clear variant for reuse:
			VariantClear (&vtVal);
			}
		}

	//////////////////////////////////////////////////	
	// DATA CHANGE WITH TIMESTAMP NOTIFICATION		//
	//////////////////////////////////////////////////
	else if (pFormatEtc->cfFormat == CF_DATA_CHANGE_TIME)
		{
		OPCGROUPHEADER groupheader;
		OPCITEMHEADER1 itemheader;
		VARIANT vtVal;

		// Read the group header:
		hr = pStream->Read (&groupheader,				// where to put the stream data
							sizeof (OPCGROUPHEADER),	// how much to read
							NULL);						// don't care about how many bytes are read

		// Release stream and return if read failed:
		if (FAILED (hr))
			{
			TRACE (_T("OTC: CKAdviseSink - IStream::Read GROUPHEADER failed.\r\n"));
			pStream->Release ();
			return;
			}

		// Post a "read complete" message if this is a read/refresh transaction
		// complete but not for unsolicited data change events.  Data change 
		// events will have a zero transaction ID.
		if (groupheader.dwTransactionID != 0)
			{
			// The client group handle in the group header structure gives us
			// a pointer to the destination CKGroup:
			CKGroup *pGroup = (CKGroup *) groupheader.hClientGroup;

			// Wrap use of pGroup in exception handler in case pointer is bad:
			try 
				{
				// Log transaction complete status:
				LogMsg (IDS_ASYNC10_READXACT_COMPLETE, groupheader.dwTransactionID, groupheader.dwItemCount,
					pGroup->GetName (), groupheader.hrStatus);
				}
			catch (...)
				{
				ASSERT (FALSE);
				}
			}

		// Initialize a variant to place a data value in:
		VariantInit (&vtVal);

		// Loop over items in the stream:
		for (DWORD dwItem = 0; dwItem < groupheader.dwItemCount; dwItem++)
			{
			// Compute offset of next item:
			dlibMove.HighPart = 0;
			dlibMove.LowPart = sizeof (OPCGROUPHEADER) + (dwItem * sizeof (OPCITEMHEADER1));

			// Seek to next item:
			hr = pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);

			// If seek failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Seek Next Item failed.\r\n"));
				continue;
				}

			// Read the item header:
			hr = pStream->Read (&itemheader, sizeof (OPCITEMHEADER1), NULL);

			// If read failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Read Item Header failed.\r\n"));
				continue;
				}

			// Seek to beginning of data:
			dlibMove.LowPart = itemheader.dwValueOffset;	
			hr = pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);

			// If seek failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Seek Data failed.\r\n"));
				continue;
				}

			// Read the variant data:
			hr = pStream->Read (&vtVal, sizeof (VARIANT), NULL);

			// If read failed, try to get next item:
			if (FAILED (hr))
				{
				TRACE (_T("OTC: CKAdviseSink - IStream::Read Variant failed.\r\n"));
				continue;
				}

			// A little more work for strings and arrays:
			switch (vtVal.vt)
				{
				case VT_BSTR:
					{
					DWORD dwLength = 0;

					// Read string length:
					hr = pStream->Read (&dwLength, sizeof (DWORD), NULL); 
        
					// If read failed, try to get next item:
					if (FAILED (hr))
						{
						TRACE (_T("OTC: CKAdviseSink - IStream::Read string size failed.\r\n"));
						continue;
						}

					if (dwLength == 0)
						vtVal.bstrVal = NULL;
					else 
						{
						// Allocate memory for string.  We will free this once we copy
						// the contents of the string into the item object:
						vtVal.bstrVal = SysAllocStringLen (NULL, dwLength);

						if (vtVal.bstrVal != NULL)
							{
							// Read string data:
							hr = pStream->Read (vtVal.bstrVal, (dwLength + 1) * sizeof (OLECHAR), NULL);

							// If read failed, try to get next item:
							if (FAILED (hr))
								{
								TRACE (_T("OTC: CKAdviseSink - IStream::Read string value failed.\r\n"));
								SysFreeString (vtVal.bstrVal);
								continue;
								}
							}
						}
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
					// Read array parameters:
					SAFEARRAY sa;
					DWORD cdwElements;
					pStream->Read (&sa, sizeof (SAFEARRAY), NULL);

					// 2-D array:
					if (sa.cDims != 1)
						{
						// Read array parameters for second dimension:
						ASSERT (sa.cDims == 2);
						SAFEARRAYBOUND sab [2];

						sab [0] = sa.rgsabound [0];
						pStream->Read (&sab [1], sizeof (SAFEARRAYBOUND), NULL);

						// Compute array size:
						cdwElements = sa.rgsabound [0].cElements * sab [1].cElements;

						// Allocate array (specify basic, i.e. non-array, data type):
						vtVal.parray = SafeArrayCreate (vtVal.vt & ~VT_ARRAY, sa.cDims, sab);
						}

					// 1-D array
					else
						{
						// Compute array size:
						cdwElements = sa.rgsabound [0].cElements;

						// Allocate array (specify basic, i.e. non-array, data type):
						vtVal.parray = SafeArrayCreate (vtVal.vt & ~VT_ARRAY, sa.cDims, sa.rgsabound);
						}

					// Read array data:
					pStream->Read (vtVal.parray->pvData, sa.cbElements * cdwElements, NULL);
					}
					break;

				case VT_BSTR | VT_BYREF:
					ASSERT (FALSE);
					break;
				}

			// The client handle in the item header structure gives us a
			// pointer to the destination CKItem:
			CKItem *pItem = (CKItem *) itemheader.hClient;

			// Wrap use of pItem in exception handler in case pointer is bad:
			try
				{
				// Fill the item's data, quality and timestamp:
				pItem->UpdateData (vtVal, itemheader.wQuality, itemheader.ftTimeStampItem);
				}
			catch (...)
				{
				TRACE (_T("OTC: Server returned bad item data in advise stream (with timestamp)\r\n"));
				}

			// Clear variant for reuse:
			VariantClear (&vtVal);
			}
		}

	//////////////////////////////////////////////////	
	// WRITE COMPLETE NOTIFICATION					//
	//////////////////////////////////////////////////
	else if (pFormatEtc->cfFormat == CF_WRITE_COMPLETE)
		{
		OPCGROUPHEADERWRITE groupheader;
		OPCITEMHEADERWRITE itemheader;

		// Read the group header:
		hr = pStream->Read (&groupheader,					// where to put the stream data
							sizeof (OPCGROUPHEADERWRITE),	// how much to read
							NULL);							// don't care about how many bytes are read

		// Release stream and return if read failed:
		if (FAILED (hr))
			{
			TRACE (_T("OTC: CKAdviseSink - IStream::Read GROUPHEADERWRITE failed.\r\n"));
			pStream->Release ();
			return;
			}
		
		// See which group this write transaction complete is for.  The 
		// client group handle in the group header structure gives us a
		// pointer to the destination CKGroup:
		CKGroup *pGroup = (CKGroup *) groupheader.hClientGroup;

		// Don't do anything with pGroup if it is an invalid pointer (i.e. == NULL):
		if (pGroup != NULL)
			{
			// Log "write complete" with status message:
			LogMsg (IDS_ASYNC10_WRITE_COMPLETE, groupheader.dwTransactionID, groupheader.dwItemCount,
				pGroup->GetName (), groupheader.hrStatus);

			// Loop over items:
			for (DWORD dwItem = 0; dwItem < groupheader.dwItemCount; dwItem++)
				{
				// Compute offset of next item:
				dlibMove.HighPart = 0;
				dlibMove.LowPart = sizeof (OPCGROUPHEADER) + (dwItem * sizeof (OPCITEMHEADERWRITE));

				// Seek to next item:
				hr = pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);

				// If seek failed, try to get next item:
				if (FAILED (hr))
					{
					TRACE (_T("OTC: CKAdviseSink - IStream::Seek Next Item failed.\r\n"));
					continue;
					}

				// read the item header:
				hr = pStream->Read (&itemheader, sizeof (OPCITEMHEADERWRITE), NULL);

				// If read failed, try to get next item:
				if (FAILED (hr))
					{
					TRACE (_T("OTC: CKAdviseSink - IStream::Read Item Header failed.\r\n"));
					continue;
					}

				// Check to see if write failed:
				if (FAILED (itemheader.dwError))
					{
					// The client handle in the item header structure gives us a
					// pointer to the destination CKItem:
					CKItem *pItem = (CKItem *) itemheader.hClient;

					// Log "failed write" message.  Wrap use of pItem with exception
					// handler in case pointer is bad:
					try
						{
						LogMsg (IDS_ASYNC10_WRITE_COMPLETE_FAILURE, groupheader.dwTransactionID,
							pItem->GetItemID (), itemheader.dwError);
						}
					catch (...)
						{
						TRACE (_T("OTC: CKAdviseSink - WriteComplete - Bad item handle\r\n"));
						}
					}
				}
			}
		else
			{
			TRACE (_T("OTC: CKAdviseSink - WriteComplete - Bad group handle\r\n"));
			}
		}

	//////////////////////////////////////////////////////	
	// WE DON'T EXPECT ANY OTHER TYPE OF NOTIFICATION	//
	//////////////////////////////////////////////////////
	else
		{
		ASSERT (FALSE);
		}
		
	// Release our stream on exit:
	pStream->Release ();
	}
