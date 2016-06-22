// **************************************************************************
// server.cpp
//
// Description:
//	Implements the CKServer class.  An object of this class is associated with
//	with each OPC Server we may connect to.
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
#include "server.h"
#include "group.h"
#include "item.h"
#include "shutdownsink.h"
#include "mainwnd.h"

// Versions for serializing:
#define VERSION_1					1			// serialized verions
#define CURRENT_VERSION				VERSION_1	// current version for saving

// m_arrMultiQI indices:
#define MQI_IOPCSERVER		0
#define MQI_IOPCCOMMON		1
#define MQI_IOPCCONNPT		2
#define MQI_IOPCITEMPROP	3
#define MQI_IOPCBROWSE		4
#define MQI_IOPCPUBLIC		5
#define MQI_IOPCPERSIST		6

// Satisfy OPCENUM requirements
static const CLSID CLSID_OPCServerList = 
{ 0x13486D51, 0x4821, 0x11D2, { 0xA4, 0x94, 0x3C, 0xB3, 0x06, 0xC1, 0x00, 0x00 } };

/////////////////////////////////////////////////////////////////////////////
// CKServer construction/destruction
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// CKServer ()
//
// Description:
//	Default constructor.
//
// Parameters:
//  none
//
// Returns:
//  none
// **************************************************************************
CKServer::CKServer ()
{
	// Initialize members:
	m_strProgID = cApp.GetDefConnectProgID ();
	m_strRemoteMachine.Empty ();

	ZeroMemory (&m_bfFlags, sizeof (m_bfFlags));

	m_pPrev = NULL;
	m_pNext = NULL;

	m_hTreeItem = NULL;

	m_pGroupHead = NULL;
	m_cdwGroups = 0;

	// Fill our multiple query interface structure with all of the
	// interfaces we can possible request at the server level:
	for (int i = 0; i < sizeof (m_arrMultiQI) / sizeof (MULTI_QI); i++)
	{
		m_arrMultiQI [i].pItf = NULL;
		m_arrMultiQI [i].hr = 0;
	}

	// Interface pointers we will query and maintain:
	m_pIServer				= NULL;
	m_pICommon				= NULL;
	m_pIConnPtContainer		= NULL;
	m_pIItemProps			= NULL;
	m_pIBrowse				= NULL;
	m_pIPublicGroups		= NULL;
	m_pIPersistFile			= NULL;	

	m_pIShutdownSink		= NULL;
	m_dwCookieShutdownSink	= 0;

	m_bConnected = false;
}

// **************************************************************************
// CKServer
//
// Description:
//	Constructor for when OPC Server's prog ID and the machine name it will be
//	running on are known.
//
// Parameters:
//  CString		&strProgID			Prog ID of server.
//	CString		&strRemoteMachine	Name of remote machine.
//
// Returns:
//  none
// **************************************************************************
CKServer::CKServer (CString &strProgID, CString &strRemoteMachine)
{
	ASSERT (!strProgID.IsEmpty ());

	// Initialize members:
	m_strProgID = strProgID;
	m_strRemoteMachine = strRemoteMachine;

	ZeroMemory (&m_bfFlags, sizeof (m_bfFlags));

	m_pPrev = NULL;
	m_pNext = NULL;

	m_hTreeItem = NULL;

	m_pGroupHead = NULL;
	m_cdwGroups = 0;

	// Fill our multiple query interface structure with all of the
	// interfaces we can possible request at the server level:
	for (int i = 0; i < sizeof (m_arrMultiQI) / sizeof (MULTI_QI); i++)
	{
		m_arrMultiQI [i].pItf = NULL;
		m_arrMultiQI [i].hr = 0;
	}

	// Interface pointer we will query and maintain:
	m_pIServer				= NULL;
	m_pICommon				= NULL;
	m_pIConnPtContainer		= NULL;
	m_pIItemProps			= NULL;
	m_pIBrowse				= NULL;
	m_pIPublicGroups		= NULL;
	m_pIPersistFile			= NULL;	

	m_pIShutdownSink		= NULL;
	m_dwCookieShutdownSink	= 0;

	m_bConnected = false;
}

// **************************************************************************
// ~CKServer ()
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
CKServer::~CKServer ()
{
	// Remove all groups:
	RemoveAllGroups ();

	// Disconnect from the OPC Sserver we are connected to:
	Disconnect ();

	// Assert that all interfaces and groups have been properly released:
	ASSERT (m_cdwGroups == 0);
	ASSERT (m_pIServer == NULL);
	ASSERT (m_pICommon == NULL);
	ASSERT (m_pIConnPtContainer == NULL);
	ASSERT (m_pIItemProps == NULL);
	ASSERT (m_pIBrowse == NULL);
	ASSERT (m_pIPublicGroups == NULL);
	ASSERT (m_pIPersistFile == NULL);
	ASSERT (m_pIShutdownSink == NULL);
	ASSERT (m_dwCookieShutdownSink == 0);
}


/////////////////////////////////////////////////////////////////////////////
// CKServer group management
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// AddGroup ()
//
// Description:
//	Called to add an OPC Group.  A CKGroup will be added to this object if
//	needed and a request to add a corresponding OPC Group to the OPC Server
//	will be made.
//
// Parameters:
//  CKGroup		*pGroup			Pointer to group object to add.
//	bool		bLoadingProject	Set to true if call is made during project 
//								  load so group can be added to this object's
//								  group list.  Otherwise we will assume group
//								  already exists in list, and only needs to
//								  added to OPC Server.
//
// Returns:
//  void
// **************************************************************************
void CKServer::AddGroup (CKGroup *pGroup, bool bLoadingProject /*= false */)
{
	ASSERT (pGroup != NULL);

	// If the group is new add it to our list:
	if (!bLoadingProject)
	{
		// New groups are added to the head of the linked list.

		// That means new the groups's "next" group is old head of list,
		pGroup->SetNext (m_pGroupHead);

		// and the new group is the old head of list's "previous" group,
		if (m_pGroupHead)	
			m_pGroupHead->SetPrev (pGroup);

		// and that new group is now the new "head" of list.
		m_pGroupHead = pGroup;

		// Don't forget to bump up the group count:
		++m_cdwGroups;
	}

	// If we are connected to OPC Server, issue a request to add this group:
	if (m_bConnected)
	{
		// Initialize arguments for add group request:
		HRESULT hr				= E_FAIL;
		WCHAR *pszName			= NULL;
		long lBias				= pGroup->GetBias ();
		float fDeadband			= pGroup->GetDeadband ();
		LPCTSTR lpszName		= pGroup->GetName ();
		DWORD dwRevUpdateRate	= 0;
		OPCHANDLE hServer		= NULL;
		IUnknown *pUnknown		= NULL;

		// All strings transmitted by COM must be in wide character (Unicode) format.
		// Declare a buffer to contain name string in wide character format.
		WCHAR wchBuffer	[DEFBUFFSIZE];

		// Convert the string format:
		if (lpszName != NULL)
		{
#ifdef _UNICODE
			// String is already in Unicode format, so simply copy into buffer:
			lstrcpyn (wchBuffer, lpszName, sizeof (wchBuffer) / sizeof (WCHAR));
#else
			// String is not in Unicode format, so convert and place result into buffer:
			if (!MultiByteToWideChar (CP_ACP, 0, lpszName, -1, wchBuffer, DEFBUFFSIZE))
			{
				ASSERT (FALSE);
				return;
			}
#endif

			// Reassign name pointer to buffer:
			pszName = wchBuffer;
		}

		// Issue add OPC Group request using IOPCServer interface.  Pointer to 
		// the IOPCServer interface, m_pIServer, should have been set in Connect().
		hr = m_pIServer->AddGroup (
			pszName,					// [in] group name
			pGroup->IsActive (),		// [in] active state
			pGroup->GetUpdateRate (),	// [in] requested update rate
			(OPCHANDLE) pGroup,			// [in] our handle to this group
			&lBias,						// [in] time bias
			&fDeadband,					// [in] percent deadband
			pGroup->GetLanguageID (),	// [in] requested language ID
			&hServer,					// [out] server handle to this group
			&dwRevUpdateRate,			// [out] revised update rate
			IID_IUnknown,				// [in] request an IUknown pointer
			&pUnknown);

		// OPC Group was successfully added:
		if (SUCCEEDED (hr))
		{
			// Log success:
			LogMsg (IDS_GROUP_ADD_SUCCESS, pGroup->GetName (), GetProgID ());

			// Since OPC Group was successfully added, we can go ahead an initialize
			// the associated CKGroup object.

			// We can now consider group valid:
			pGroup->SetValid (TRUE);

			// We should have gotten a valid pointer to the OPC Groups's IUnknown interface.
			// Set some things that only make sence if we have a vaild IUnknown pointer:
			if (pUnknown)
			{
				// Set the OPC Server's handle for this group:
				pGroup->SetServerHandle (hServer);

				// Reset update rate if OPC Server does not support requested rate:
				if (pGroup->GetUpdateRate () != dwRevUpdateRate)
					pGroup->SetUpdateRate (dwRevUpdateRate);

				// Initialize the CKGroup object, which will include getting necessary
				// interface pointers from IUnknown:
				pGroup->Initialize (pUnknown);

				// We can release the IUnknown pointer since initialized group
				// should have gotten the interface pointers it needs from it.
				pUnknown->Release ();
			}
			else
			{
				TRACE (_T("OTC: Warning added group %s to OPC server, but IUnknown is invalid.\r\n"),
					pGroup->GetName ());
			}
		}

		// OPC Group was not successfully added:
		else
		{
			// Log message that says add OPC Group request failed:
			LogMsg (IDS_GROUP_ADD_FAILURE, pGroup->GetName (), GetProgID (), hr);
		}
	}

	// No connection:
	else
	{
		// Log message that says we can't add the OPC Group because we are not
		// connected to OPC Server:
		LogMsg (IDS_SERVER_ADD_GROUP_FAILED_NOCONNECTION, pGroup->GetName (), GetProgID ());
	}
}

// **************************************************************************
// AddClonedGroup ()
//
// Description:
//	Add cloned group to list.  
//
//	This is called from CKGroup::Clone (), which takes care of instantiating
//  the new CKGroup object and adding its associated OPC Group to the OPC
//  Server.  All that's left to do here is add the clone to the group list. 
//
// Parameters:
//  CKGroup		*pClone		Pointer to cloned group.
//
// Returns:
//  void
// **************************************************************************
void CKServer::AddClonedGroup (CKGroup *pClone)
{
	// New groups are added to the head of the linked list.

	// That means the clone's "next" group is old head of list,
	pClone->SetNext (m_pGroupHead);

	// and the clone is the old head of list's "previous" group,
	if (m_pGroupHead)	
		m_pGroupHead->SetPrev (pClone);

	// and that clone is now the new "head" of list.
	m_pGroupHead = pClone;

	// Don't forget to bump up the group count:
	++m_cdwGroups;
}

// **************************************************************************
// RemoveGroup ()
//
// Description:
//	Remove an OPC Group from OPC server, and delete associated CKGroup object
//	if asked.
//
// Parameters:
//  CKGroup		*pGroup			Pointer to CKGroup associated with OPC Group
//								  to remove.
//	bool		bDelete			Delete CKGroup object after removing
//								  associated OPC Group. true by default.
//
// Returns:
//  void
// **************************************************************************
void CKServer::RemoveGroup (CKGroup *pGroup, bool bDelete /* = true */)
{
	ASSERT (pGroup != NULL);

	HRESULT hr;

	// Uninitialize group with OPC server.  This means all interfaces it
	// is using are released.  It is good practice to do this BEFORE
	// issuing a remove group request.  Pass the bDelete value so the 
	// group knows to delete its items if any.
	pGroup->Uninitialize (bDelete);

	// remove the group from the OPC server
	if (pGroup->IsValid ())
	{
		ASSERT (m_pIServer != NULL);

		// Issue remove OPC Group request using IOPCServer interface.  Pointer to 
		// the IOPCServer interface, m_pIServer, should have been set in Connect().
		hr = m_pIServer->RemoveGroup (
			pGroup->GetServerHandle (),		// server handle for this group.
			pGroup->IsForceDeletion ());	// force a delete if this client has not released the group properly.

		// Log success or failure:
		if (SUCCEEDED (hr))
			LogMsg (IDS_GROUP_REMOVE_SUCCESS, pGroup->GetName (), GetProgID ());
		else
			LogMsg (IDS_GROUP_REMOVE_FAILURE, pGroup->GetName (), GetProgID (), hr);
	}

	// Delete the group if asked:
	if (bDelete)
	{
		// We need to remove the group from the linked list before we delete it.

		// To remove a link, we must first get pointers to the adjacent links:
		CKGroup *pPrev = pGroup->GetPrev ();
		CKGroup *pNext = pGroup->GetNext ();

		// If there is a "previous" link, then its "next" is removed link's "next",
		if (pPrev)
			pPrev->SetNext (pNext);

		// and if there is a "next" link, then its "previous" is removed link's "previous",
		if (pNext)
			pNext->SetPrev (pPrev);

		// and if removed link was the "head", then the new head is removed link's "next".
		if (pGroup == m_pGroupHead)
			m_pGroupHead = pNext;

		// Decrement the group count:
		--m_cdwGroups;

		// Delete the group object:
		delete pGroup;
	}
	else
	{
		// Since CKGroup object was not deleted, it should be flagged as invalid
		// becuase it no longer has an associated OPC Group in OPC Server.
		pGroup->SetValid (false);
	}
}

// **************************************************************************
// RemoveAllGroups ()
//
// Description:
//	Remove all OPC Groups from OPC server, and delete associated CKGroup
//  objects if asked.
//
// Parameters:
//  bool		bDelete		Delete group objects after removing.  true by 
//							 default.
//
// Returns:
//  void
// **************************************************************************
void CKServer::RemoveAllGroups (bool bDelete /* = true */)
{
	// Start with the head of linked list and work our way up chain:
	CKGroup *pGroup = m_pGroupHead;

	// Remove OPC Groups and delete assoicated CKGroup objects:
	if (bDelete)
	{
		// Work our way up linked list.  pGroup will be NULL when we reach
		// end, and we will then break out of loop.
		while (pGroup)
		{
			// Get the group's next item before we delete group.  The
			// next item will be the new head of linked list.
			m_pGroupHead = pGroup->GetNext ();

			// Remove and delete:
			RemoveGroup (pGroup, bDelete);

			// Next group to processes is the new head of the list:
			pGroup = m_pGroupHead;
		}
	}

	// Just remove the OPC Groups:
	else
	{
		// Work our way up linked list.  pGroup will be NULL when we reach
		// end, and we will then break out of loop.
		while (pGroup)
		{					
			// Remove the group:
			RemoveGroup (pGroup, bDelete);

			// Process next group in list:
			pGroup = pGroup->GetNext ();
		}
	}
}

#define MAXGROUPNAMELEN	32	// impose a limit when generating a name


// **************************************************************************
// GenerateGroupName ()
//
// Description:
//	Check to see if a group name is in use.  If it is, then modify name so
//	that it is unique to this server.
//
// Parameters:
//  CString		&strName		Group name.  Must not be input as empty.
//								  Will be modified if needed.
//
// Returns:
//  bool - true if success
// **************************************************************************
bool CKServer::GenerateGroupName (CString &strName)
{
	ASSERT (!strName.IsEmpty ());

	// If FindGroup() returns true, then name is in use and we need to
	// generate a new group name unique to this server:
	if (FindGroup (strName))
	{
		TCHAR szName [MAXGROUPNAMELEN + 1];
		TCHAR szSuffix [16];
		LPTSTR pch;
		int nSuffix;

		// Make a copy of the group name.  Use a TCHAR array so it will be
		// fast & easy to check each character using pointer math.
		lstrcpyn (szName, strName, _countof (szName));

		// Point to the last character (the -1 is needed since arrays are 0-based):
		pch = &szName [lstrlen (szName) - 1];


		//////////////////////////////////////////////////////////////
		// Create a numerical suffix that should make name unique	//
		//////////////////////////////////////////////////////////////

		// If the last character is a number, then we will interpret all digits
		// a end of string as a suffix.  Once that suffix is parsed off, we will
		// increment it for the new name.
		if (_istdigit (*pch))
		{
			// Back up until we hit a non-digit:
			while (_istdigit (*pch))
				--pch;

			// Adjust forward one character so we now point to first
			// digit in numerical suffix:
			++pch;

			// Determine the current suffix and increment:
			nSuffix = _ttol (pch) + 1;
		}

		// Otherwise simply start a new series with the suffix "1":
		else
		{
			// Move forwared one character so we'll be pointing at
			// string's NULL terminator:
			++pch;
			nSuffix = 1;
		}


		//////////////////////////////////////////////////////////
		// Add new numerical suffix and validate resulting name	//
		//////////////////////////////////////////////////////////

		do
		{
			// Trim any existing numeric suffix from the name. (pch should
			// be pointing at first digit of suffix it it exists, or NULL
			// terminator of string if not.)
			*pch = 0;

			// Convert the suffix we just created to a string:
			wsprintf (szSuffix, _T("%d"), nSuffix);

			// Determine the length of name minus existing numerical suffix, if any:
			int nNameLen = lstrlen (szName);

			// Determin the length of the suffix we just created:
			int nSuffixLen = lstrlen (szSuffix);

			// If the name with new suffix will be longer than allowable maximum
			// (for logging) then truncate:
			if (nNameLen + nSuffixLen > MAXGROUPNAMELEN)
			{
				// Compute the number of characters we need to hack off:
				int nDif = MAXGROUPNAMELEN - (nNameLen + nSuffixLen);

				ASSERT (nDif < 0);

				// Adjust the length in preparation to truncate:
				nNameLen += nDif;
				pch += nDif;

				// Check for remaining chars and return false if we just can't fit
				// the suffix:
				if (nNameLen < 1)
					return (false);

				// If we make it here, then we can go ahead and truncate the name
				// by replacing the nNameLen'th character with a NULL terminator:
				szName [nNameLen] = 0;
			}

			// Append the suffix to the name:
			lstrcat (szName, szSuffix);

			// Validate the resulting name.  If FindGroup returns false, then
			// we know the new name is not currently in use.
			if (!FindGroup (szName))
			{
				// New name is not in use.  Assing it to strName for output and 
				// return true to indicate successful processing.
				strName = szName;
				return (true);
			}

			// If we make it here, we know new name is no good.  Increment the suffix
			// and try again.  We will continue to loop unitl the suffix results in a
			// good name or string length becomes too long.
		} while (++nSuffix > 0);

		// If we make it to here (not likely), then we've run out of
		// options.  Return false to indicate that we had problems.
		ASSERT (FALSE);
		return (false);
	}

	// If we made it here, then specified name is not presently in use.
	// No need to change it.  Just return true to indicate we successfully
	// processed name.
	return (true);
}

// **************************************************************************
// FindGroup ()
//
// Description:
//	Search list of groups attached to this server for one with specified name.
//
// Parameters:
//  LPCTSTR		lpszName	Pointer to name string.
//
// Returns:
//  bool - true if group found.
// **************************************************************************
bool CKServer::FindGroup (LPCTSTR lpszName)
{
	// Start with first group in linked list and work our way up the chain:
	CKGroup *pGroup = m_pGroupHead;

	// Keep looping we hit the end of the link list (where pGroup will be NULL):
	while (pGroup)			
	{
		// If the name of the current group is the same as lpszName, then
		// we can break out of loop.  Return true to indicate that group
		// was found.
		if (lstrcmpi (lpszName, pGroup->GetName ()) == 0)
			return (true);

		// Look at next group in list next time around:
		pGroup = pGroup->GetNext ();
	}

	// If we make it here, then we have looked at all of the groups in the
	// list and didn't find one with specified name.  Return false to
	// indicate group was not found.
	return (false);
}

// **************************************************************************
// GetGroup ()
//
// Description:
//	Get pointer to group with specified name.
//
// Parameters:
//  LPCTSTR		lpszName	Pointer to name string.
//
// Returns:
//  CKGroup* - Pointer to group object, NULL if group not found.
// **************************************************************************
CKGroup* CKServer::GetGroup (LPCTSTR lpszName)
{
	// Start with first group in linked list and work our way up the chain:
	CKGroup *pGroup = m_pGroupHead;

	// Keep looping we hit the end of the link list (where pGroup will be NULL):
	while (pGroup)			
	{
		// If the name of the current group is the same as lpszName, then
		// we can break out of loop.  Return pointer to group.
		if (lstrcmpi (lpszName, pGroup->GetName ()) == 0)
			return (pGroup);

		// Look at next group in list next time around:
		pGroup = pGroup->GetNext ();
	}

	// If we make it here, then we have looked at all of the groups in the
	// list and didn't find one with specified name.  Return NULL to indicate
	// group was not found.
	return (NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CKServer manipulators
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// SetProgID ()
//
// Description:
//	Set Prog ID of OPC Server we will be associated with.
//
// Parameters:
//	CString		&strProgID		Prog ID.
//
// Returns:
//  void
// **************************************************************************
void CKServer::SetProgID (CString &strProgID)
{
	ASSERT (!strProgID.IsEmpty ());

	// Save Prog ID string as a member variable:
	m_strProgID = strProgID;
}

// **************************************************************************
// SetRemoteMachine ()
//
// Description:
//	Set name of machine associated OPC Server will be running on.
//
// Parameters:
//  CString		&strRemoteMachine	Name of remote machine, NULL if local.
//
// Returns:
//  void
// **************************************************************************
void CKServer::SetRemoteMachine (CString &strRemoteMachine)
{
	// Save machine name string as a member variables.  This string will be
	// empty if server will be running on the local machine.
	m_strRemoteMachine = strRemoteMachine;
}


/////////////////////////////////////////////////////////////////////////////
// CKServer serialization
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// Serialize ()
//
// Description:
//	Save or load server properties.
//
// Parameters:
//  CArchive		&ar			The archive to save or load server properties.
//
// Returns:
//  void
// **************************************************************************
void CKServer::Serialize (CArchive &ar)
{
	// Save server properties:
	if (ar.IsStoring ())
	{
		//////////////////////////////
		// Save archive properties	//
		//////////////////////////////

		// Output current archive version.  If we add server properties that
		// must be made persistent, then we will have to update the archive
		// version so we know how to read the additional data.
		ar << CURRENT_VERSION;


		//////////////////////////////
		// Save server properties	//
		//////////////////////////////

		// Output info about associated OPC Server:
		ar << m_strProgID << m_strRemoteMachine;

		// Output flags in bit field:
		ar.Write (&m_bfFlags, sizeof (m_bfFlags));

		// Output number of groups so we will know how many block of group 
		// data follows when we read this archive later:
		ar << m_cdwGroups;


		//////////////////////////////////
		// Serialize all of our groups	//
		//////////////////////////////////

		//Start with head of linked list:
		CKGroup *pGroup = m_pGroupHead;

		// Keep looping we hit the end of the link list (where pGroup will be NULL):
		while (pGroup)			
		{
			// Call group's serialize function so that it can add its properties
			// to the archive:
			pGroup->Serialize (ar);

			// Process next group next time around:
			pGroup = pGroup->GetNext ();
		}
	}

	// Load server properties:
	else
	{
		DWORD dwSchema;
		DWORD dwIndex;


		//////////////////////////////
		// Load archive properties	//
		//////////////////////////////
		ar >> dwSchema;

		// dwSchema is really the archive version.
		switch (dwSchema)
		{
		case VERSION_1:
			//////////////////////////////
			// Load server properties	//
			//////////////////////////////

			// Input info about associated OPC Server:
			ar >> m_strProgID >> m_strRemoteMachine;

			// Input flags in bit field:
			ar.Read (&m_bfFlags, sizeof (m_bfFlags));

			// Input number of groups so we will know how many block of group 
			// data follows:
			ar >> m_cdwGroups;


			//////////////////////////////////
			// Serialize all of our groups	//
			//////////////////////////////////

			// Start with an empty linked list:
			ASSERT (m_pGroupHead == NULL);

			// Loop over expected number of groups:
			for (dwIndex = 0; dwIndex < m_cdwGroups; dwIndex++)
			{
				CKGroup *pGroup = NULL;

				// Wrap group serialization with exception handler in case
				// there is a problem, such as bad archive version etc.
				try
				{
					// Instantiate a new CKGroup (with this server as it's parent):
					pGroup = new CKGroup (this);

					// Call the group's serialize function so that it can read
					// its properties from the archive:
					pGroup->Serialize (ar);

					// New groups are added to the head of the linked list.

					// That means new the groups's "next" group is old head of list,
					pGroup->SetNext (m_pGroupHead);

					// and the new group is the old head of list's "previous" group,
					if (m_pGroupHead)	
						m_pGroupHead->SetPrev (pGroup);

					// and that new group is now the new "head" of list.
					m_pGroupHead = pGroup;
				}

				catch (...)
				{
					// There was a problem reading group properties.  Delete
					// this object and throw another exception.  This exception
					// must be handled by the calling function.
					delete this;
					AfxThrowArchiveException (CArchiveException::generic);
				}
			}
			break;

		default:
			// Unexpected archive version.  Delete this object and
			// throw an exception that indicates this.  This exception
			// must be handled by the calling function.
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
//	Connect to OPC Server, then add and start all its OPC Groups.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServer::Start ()
{
	// Attempt to connect to the OPC Server:
	Connect ();

	// Regardless of connection success, add the groups.  (They will be added
	// invalid (i.e, for edits) if the server is not connected.)

	// Start with head of linked list:
	CKGroup *pGroup = m_pGroupHead;

	// Keep looping we hit the end of the link list (where pGroup will be NULL):
	while (pGroup)
	{
		// Add the group to OPC Server.  (Second argument is "true" which will
		// prevent the group from being added to our group list again.):
		AddGroup (pGroup, true);

		// Start the group:
		pGroup->Start ();

		// Look at next group in list next time around:
		pGroup = pGroup->GetNext ();
	}
}

// **************************************************************************
// Stop ()
//
// Description:
//	Remove all OPC Groups from OPC Server and disconnect.  Our CKGroup
//	objects will not be deleted at this point.  We will delete them when
//	they are actually removed from project.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServer::Stop ()
{
	// Remove all OPC Groups.  Passing "false" will prevent our CKGroups
	// from being deleted.
	RemoveAllGroups (false);

	// Disconnect from the OPC Server we are connected to:
	Disconnect ();
}

// **************************************************************************
// Copy ()
//
// Description:
//	Copy server properties to shared memory file.
//
// Parameters:
//  CFixedSharedFile	&sf	Shared memory file to copy properties to.
//
// Returns:
//  void
// **************************************************************************
void CKServer::Copy (CFixedSharedFile &sf)
{
	int nLen;

	// Since we are saving data to a shared memory file, we can not take
	// advantage of the "smarts" built into the CArchive class like we do
	// in Serialize().  Consequently, to save a string, we must first save
	// its length as an integer, followed by the characters in the string
	// as TCHAR's.

	// Save the prog ID string:
	nLen = m_strProgID.GetLength ();
	sf.Write (&nLen, sizeof (nLen));
	sf.Write (m_strProgID, nLen * sizeof (TCHAR));

	// Save the remote machine name string:
	nLen = m_strRemoteMachine.GetLength ();
	sf.Write (&nLen, sizeof (nLen));
	sf.Write (m_strRemoteMachine, nLen * sizeof (TCHAR));

	// Save the flag bit field:
	sf.Write (&m_bfFlags, sizeof (m_bfFlags));

	// Save the number of groups so we'll know how many to read later:
	sf.Write (&m_cdwGroups, sizeof (m_cdwGroups));

	// Add all of our groups' properties:

	// Start with the head of linked list and work our way through the chain:
	CKGroup *pGroup = m_pGroupHead;

	// Keep looping until we reach end of linked list (pGroup will be NULL):
	while (pGroup)
	{
		// Call group's copy function so it will add its properties to
		// shared memory file:
		pGroup->Copy (sf);

		// Process next group in list next time around:
		pGroup = pGroup->GetNext ();
	}
}

// **************************************************************************
// Paste ()
//
// Description:
//	Assign properties from shared memory file.
//
// Parameters:
//  CFixedSharedFile	&sf	Shared memory file to get properties from.
//
// Returns:
//  void
// **************************************************************************
void CKServer::Paste (CFixedSharedFile &sf)
{
	int nLen;

	// Since we are reading data from a shared memory file, we can not take
	// advantage of the "smarts" built into the CArchive class like we do
	// in Serialize().  Consequently, to read a string, we must first read
	// its length as an integer, the read that many characters as TCHAR's.

	// Read the prog ID string:
	sf.Read (&nLen, sizeof (nLen));
	sf.Read (m_strProgID.GetBufferSetLength (nLen), nLen * sizeof (TCHAR));

	// Read the remote machine name string:
	sf.Read (&nLen, sizeof (nLen));
	sf.Read (m_strRemoteMachine.GetBufferSetLength (nLen), nLen * sizeof (TCHAR));

	// Read the flag bit field:
	sf.Read (&m_bfFlags, sizeof (m_bfFlags));

	// Read the group count:
	sf.Read (&m_cdwGroups, sizeof (m_cdwGroups));

	// Add the groups:

	// Start with an empty linked list:
	ASSERT (m_pGroupHead == NULL);

	// Loop over expected number of groups:
	for (DWORD dwIndex = 0; dwIndex < m_cdwGroups; dwIndex++)
	{
		CKGroup *pGroup = NULL;

		// Wrap group paste with exception handler in case there is a problem:
		try
		{
			// Instantiate a new CKGroup (with this server as it's parent):
			pGroup = new CKGroup (this);

			// Call the group's paste function so that it can read its 
			// properties from the shared memory file:
			pGroup->Paste (sf);

			// New groups are added to the head of the linked list. 

			// That means new the groups's "next" group is old head of list,
			pGroup->SetNext (m_pGroupHead);

			// and the new group is the old head of list's "previous" group,
			if (m_pGroupHead)	
				m_pGroupHead->SetPrev (pGroup);

			// and that new group is now the new "head" of list.
			m_pGroupHead = pGroup;
		}

		catch (...)
		{
			// There was a problem reading group properties.
			ASSERT (FALSE);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CKServer OPC Specifics
/////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// GetCLSID ()
//
// Description:
//	Get the Class ID of server.
//
// Parameters:
//  CLSID		&clsid		Class ID.
//
// Returns:
//  HRESULT - use FAILED or SUCCEEDED macro to test.
// **************************************************************************
HRESULT CKServer::GetCLSID (CLSID &clsid)
{
	// Local CLSID
	if (m_strRemoteMachine.IsEmpty ())
	{
#ifdef UNICODE
		return (CLSIDFromProgID (m_strProgID, &clsid));
#else
		WCHAR wszProgID [DEFBUFFSIZE];

		if (!MultiByteToWideChar (CP_ACP, 0, m_strProgID, -1, wszProgID, sizeof (wszProgID) / sizeof (WCHAR)))
		{
			ASSERT (FALSE);
		}

		return (CLSIDFromProgID (wszProgID, &clsid));
#endif
	}

	// Remote CLSID - Use OPCENUM.EXE
	else
	{
		COSERVERINFO tServerInfo;
		ZeroMemory (&tServerInfo, sizeof (tServerInfo));

#ifdef _UNICODE
		tServerInfo.pwszName = (LPTSTR)(LPCTSTR)m_strRemoteMachine;
#else
		int nSize = _tcslen (m_strRemoteMachine) + 1;

		tServerInfo.pwszName = (WCHAR *)CoTaskMemAlloc (sizeof (WCHAR) * nSize);
		mbstowcs (tServerInfo.pwszName, m_strRemoteMachine, nSize);
#endif
		// Interfaces of interest
		MULTI_QI arrMultiQI [1] = {&IID_IOPCServerList, NULL, 0};

		// Connect to the OPC server browser
		HRESULT hr = CoCreateInstanceEx (CLSID_OPCServerList, NULL, CLSCTX_SERVER, &tServerInfo, _countof (arrMultiQI), arrMultiQI);

#ifndef _UNICODE
		// Free ansi conversion buffer
		if (tServerInfo.pwszName)
			CoTaskMemFree (tServerInfo.pwszName);
#endif
		if (FAILED (hr))
			return (hr);

		IOPCServerList *pIOPCServerList = (IOPCServerList *)arrMultiQI [0].pItf;
		arrMultiQI [0].pItf = NULL;

		// Retrieve the CLSID
#ifdef UNICODE
		hr = pIOPCServerList->CLSIDFromProgID (m_strProgID, &clsid);
#else
		WCHAR wszProgID [DEFBUFFSIZE];

		if (!MultiByteToWideChar (CP_ACP, 0, m_strProgID, -1, wszProgID, sizeof (wszProgID) / sizeof (WCHAR)))
		{
			ASSERT (FALSE);
		}

		hr = pIOPCServerList->CLSIDFromProgID (wszProgID, &clsid);
#endif

		// Disconnect from the OPC server browser
		pIOPCServerList->Release ();

		// Return result code
		return (hr);
	}
}

// **************************************************************************
// IsAlive ()
//
// Description:
//	Called to test connection to server.
//
// Parameters:
//  none
//
// Returns:
//  bool - true if connection is live.
// **************************************************************************
bool CKServer::IsAlive ()
{
	// Assume server connection is dead:
	bool bAlive = false;

	// If we don't think we are connected, i.e. m_bConnected or m_pIServer is
	// NULL, then there is no need to test connection.
	if (m_bConnected && m_pIServer)
	{
		// We think we are connected.  Let's test that...

		// Wrap our test in exception handler just in case we get a bad pointer:
		try
		{
			OPCSERVERSTATUS *pStatus;

			// Use the Get status member function of the IOPCServer interface
			// to test connection.  If this succeeds, then we know connection
			// is alive.
			if (SUCCEEDED (m_pIServer->GetStatus (&pStatus)))
			{
				// COM requires us to free memory allocated for [out] and [in/out]
				// arguments (i.e. vender info string and pStatus)
				if (pStatus->szVendorInfo)
					CoTaskMemFree (pStatus->szVendorInfo);

				CoTaskMemFree (pStatus);

				// Don't forget to set the return status to "true":
				bAlive = true;
			}
		}
		catch (...)
		{
			ASSERT (FALSE);
			bAlive = false;
		}
	}

	return (bAlive);
}

// **************************************************************************
// Connect ()
//
// Description:
//	Connect to OPC Server.  OPC Server's ProgID and machine	name must have
//  been previously specified.
//
// Parameters:
//  none
//
// Returns:
//  bool - true if success.
// **************************************************************************
bool CKServer::Connect ()
{
	// Program ID of OPC Server should have been defined by now:
	ASSERT (!m_strProgID.IsEmpty ());

	// Assume we are not connecting to KEPServerEx:
	m_bfFlags.bIsKepServerEx = false;

	// Perform any necessary cleanup from a previous connection:
	Disconnect ();

	// Obtain the Class ID of the OPC Server.  (GetCLSID() will need the
	// OPC Server's Program ID to succeed.  That's why we checked it above.)
	CLSID clsid;
	if (SUCCEEDED (GetCLSID (clsid)))
	{
		HRESULT hr;

		// Re-intialize Multi-Query Interface:
		for (int i = 0; i < sizeof (m_arrMultiQI) / sizeof (MULTI_QI); i++)
		{
			m_arrMultiQI [i].pItf = NULL;
			m_arrMultiQI [i].hr = 0;
		}

		// Load up the Interface ID's we hope to get pointers for when we
		// call CoCreateInstanceEx():
		m_arrMultiQI [MQI_IOPCSERVER].pIID		= &IID_IOPCServer;
		m_arrMultiQI [MQI_IOPCCOMMON].pIID		= &IID_IOPCCommon;
		m_arrMultiQI [MQI_IOPCCONNPT].pIID		= &IID_IConnectionPointContainer;
		m_arrMultiQI [MQI_IOPCITEMPROP].pIID	= &IID_IOPCItemProperties;
		m_arrMultiQI [MQI_IOPCBROWSE].pIID		= &IID_IOPCBrowseServerAddressSpace;
		m_arrMultiQI [MQI_IOPCPUBLIC].pIID		= &IID_IOPCServerPublicGroups;
		m_arrMultiQI [MQI_IOPCPERSIST].pIID		= &IID_IPersistFile;
		
		// Connect to the OPC Server and query all possible interfaces:
		if (m_strRemoteMachine.IsEmpty ())
		{
			// Since m_strRemoteMachine is empty, we will try to instantiate
			// the OPC Server on our local machine.  

			// CoCreateInstanceEx will launch the OPC Server if necessary, and
			// call its QueryInterface for us (bumping its reference count):
			hr = CoCreateInstanceEx (
				clsid,										// CLSID
				NULL,										// No aggregation
				CLSCTX_SERVER,								// connect to local, inproc and remote servers
				NULL,										// remote machine name 
				sizeof (m_arrMultiQI) / sizeof (MULTI_QI),	// number of IIDS to query		
				m_arrMultiQI);								// array of IID pointers to query
		}
		else
		{
			// Since m_strRemoteMachine is not empty, we will assume it contains
			// a valid remote machine name.  We will try to instantiate the OPC
			// Server object on the machine with that name.

			// First we need to initialize a server info structure:
			COSERVERINFO tCoServerInfo;
			ZeroMemory (&tCoServerInfo, sizeof (tCoServerInfo));

			// Allocate memory for the machine name string:
			int nSize = m_strRemoteMachine.GetLength () * sizeof (WCHAR);
			tCoServerInfo.pwszName = new WCHAR [nSize];

			// Check validity of pointer.  If it's bad, there's no point in continuing:
			if (!tCoServerInfo.pwszName)
			{
				ASSERT (FALSE);
				return (false);
			}

			// Copy the machine name string into the server info structure:
#ifdef _UNICODE
			// For Unicode builds, the contents of m_strRemoteMachine will
			// already be in wide character format, as demanded by COM, so
			// copy it as is.
			lstrcpyn (tCoServerInfo.pwszName, m_strRemoteMachine, nSize);
#else 
			// For ANSI builds, the contents of m_strRemoteMachine will not
			// be in wide character format, as demanded by COM, so we need
			// to reformat:
			mbstowcs (tCoServerInfo.pwszName, m_strRemoteMachine, nSize);
#endif//_UNICODE

			// CoCreateInstanceEx will launch the OPC Server if necessary, and
			// call its QueryInterface for us (bumping its reference count):
			hr = CoCreateInstanceEx (
				clsid,										// CLSID
				NULL,										// No aggregation
				CLSCTX_REMOTE_SERVER,						// connect to remote servers
				&tCoServerInfo,								// remote machine name 
				sizeof (m_arrMultiQI) / sizeof (MULTI_QI),	// number of IIDS to query		
				m_arrMultiQI);								// array of IID pointers to query

			// COM requires us to free memory allocated for [out] and [in/out]
			// arguments (i.e. name string).
			delete [] tCoServerInfo.pwszName;
		}

		// If CoCreateInstanceEx succeeded, we can check the returned 
		// interface pointers and save them as member variables:
		if (SUCCEEDED (hr))
		{
			TRACE (_T("OTC: Initializing server %s interfaces.\r\n"), GetProgID ());

			// Check IOPCServer interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCSERVER].hr))
			{
				m_pIServer = (IOPCServer *)m_arrMultiQI [MQI_IOPCSERVER].pItf;

				if (m_pIServer == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCSERVER].pItf != NULL)
				{
					// Warning failure but pointer not set to null
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IOPCServer (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCSERVER].hr); 
			}

			// Check IOPCCommon interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCCOMMON].hr))
			{
				m_pICommon = (IOPCCommon *)m_arrMultiQI [MQI_IOPCCOMMON].pItf;

				if (m_pICommon == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCCOMMON].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IOPCCommon (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCCOMMON].hr); 
			}

			// Check IConnectionPointContainer interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCCONNPT].hr))
			{
				m_pIConnPtContainer = 
					(IConnectionPointContainer *)m_arrMultiQI [MQI_IOPCCONNPT].pItf;

				if (m_pIConnPtContainer == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCCONNPT].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IConnectionPoint (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCCONNPT].hr); 
			}

			// Check IOPCItemProperties interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCITEMPROP].hr))
			{
				m_pIItemProps = 
					(IOPCItemProperties *)m_arrMultiQI [MQI_IOPCITEMPROP].pItf;

				if (m_pIItemProps == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCITEMPROP].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IOPCItemProperties (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCITEMPROP].hr); 				
			}

			// Check IOPCBrowseServerAddressSpace interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCBROWSE].hr))
			{
				m_pIBrowse = 
					(IOPCBrowseServerAddressSpace *)m_arrMultiQI [MQI_IOPCBROWSE].pItf;

				if (m_pIBrowse == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCBROWSE].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IOPCBrowseServerAddressSpace (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCBROWSE].hr); 				
			}

			// Check IOPCServerPublicGroups interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCPUBLIC].hr))
			{
				m_pIPublicGroups = 
					(IOPCServerPublicGroups *)m_arrMultiQI [MQI_IOPCPUBLIC].pItf;

				if (m_pIPublicGroups == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCPUBLIC].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IOPCServerPublicGroups (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCPUBLIC].hr); 				
			}

			// Check IPersistFile interface pointer:
			if (SUCCEEDED (m_arrMultiQI [MQI_IOPCPERSIST].hr))
			{
				m_pIPersistFile = 
					(IPersistFile *)m_arrMultiQI [MQI_IOPCPERSIST].pItf;

				if (m_pIPersistFile == NULL)
				{
					// Warning success but no valid pointer:
					ASSERT (FALSE);
				}
			}
			else
			{
				if (m_arrMultiQI [MQI_IOPCPERSIST].pItf != NULL)
				{
					// Warning failure but pointer not set to null:
					ASSERT (FALSE);
				}

				TRACE (_T("OTC: Failed to query IPersistsFile (%08X).\r\n"), 
					m_arrMultiQI [MQI_IOPCPERSIST].hr); 				
			}

			// Check IConnectionPointContainer interface pointer:
			if (m_pIConnPtContainer != NULL)
			{
				// If the server supports the shutdown interface, provide a sink 
				// to the server.

				// Get connection point pointer:
				IConnectionPoint *pCP = NULL;
				hr = m_pIConnPtContainer->FindConnectionPoint (IID_IOPCShutdown, &pCP);

				// If we got the connection point, instantiate our shutdown sink:
				if (SUCCEEDED (hr))
				{
					try
					{
						// Instantiate the shutdown sink and add us to its reference count:
						m_pIShutdownSink = new IKShutdownSink (this);
						m_pIShutdownSink->AddRef ();

						// Give the connection point a pointer to our shutdown sink:
						// (m_dwCookieShutdownSink is a returned token that uniquely
						// identifies this connection.)
						hr = pCP->Advise (m_pIShutdownSink, &m_dwCookieShutdownSink);

						// We are done with the connection point, so release our reference:
						pCP->Release ();
					}

					catch (...)
					{
						// If we find ourselves here, either "new" failed or pCP is bad.
						ASSERT (FALSE);
						hr = E_FAIL;
					}
				}
			}

			// We will base our success on the validity of the IOPCServer interface
			// pointer.  If it is invalid, then we won't be able do do anyting:
			m_bConnected = (m_pIServer != NULL);

			// Log success or failure:
			if (m_bConnected)
				LogMsg (IDS_SERVER_CONNECT_SUCCESS, GetProgID ());
			else
				LogMsg (IDS_SERVER_REQUIRED_IID_UNSUPPORTED, GetProgID (), hr);
		}

		// CoCreateInstanceEx failed:
		else
		{
			// log failure
			LogMsg (IDS_SERVER_CONNECT_FAILURE, GetProgID (), hr);
		}
	}

	// Failed to get Class ID:
	else
	{
		// Log failure:
		LogMsg (IDS_SERVER_UNABLE_TO_GET_CLSID, GetProgID ());
	}

	// Return connected state:
	return (m_bConnected);
}

// **************************************************************************
// Connect ()
//
// Description:
//	Attempts to connect to the OPC Server with specified ProgID and machine name.
//
// Parameters:
//  CString		&strProgID			ProgID of OPC server.
//	CString		&strRemoteMachine	Machine name OPC server is on.
//
// Returns:
//  bool - true if success.
// **************************************************************************
bool CKServer::Connect (CString &strProgID, CString &strRemoteMachine)
{
	ASSERT (m_bConnected == FALSE);

	// Update our prog ID and machine name member variables:
	SetProgID (strProgID);
	SetRemoteMachine (strRemoteMachine);

	// Connect:
	return (Connect ());
}

// **************************************************************************
// Disconnect ()
//
// Description:
//	Called to disconnect from OPC server.
//
// Parameters:
//  none
//
// Returns:
//  void
// **************************************************************************
void CKServer::Disconnect ()
{
	// Log success if we were truly connected:
	if (m_bConnected == true)
		LogMsg (IDS_SERVER_DISCONNECTED, GetProgID ());

	// Reset member variable:
	m_bConnected = false;

	// Release all of our server interface references:
	if (m_pIServer)
	{
		m_pIServer->Release ();
		m_pIServer = NULL;
	}

	if (m_pICommon)
	{
		m_pICommon->Release ();
		m_pICommon = NULL;
	}

	if (m_pIConnPtContainer)
	{
		// Unadvise shutdown notifications:
		if (m_dwCookieShutdownSink != 0)
		{
			HRESULT hr = E_FAIL;
			IConnectionPoint *pCP = NULL;

			hr = m_pIConnPtContainer->FindConnectionPoint (IID_IOPCShutdown, &pCP);

			if (SUCCEEDED (hr))
			{
				hr = pCP->Unadvise (m_dwCookieShutdownSink);
				pCP->Release ();
			}

			if (FAILED (hr))
			{
				TRACE (_T("OTC: CKServer::Disconnect () - failed to unadvise shutdown notifications\r\n"));
			}

			m_dwCookieShutdownSink = 0;
		}

		if (m_pIShutdownSink != NULL)
		{
			m_pIShutdownSink->Release ();
			m_pIShutdownSink = NULL;
		}

		m_pIConnPtContainer->Release ();
		m_pIConnPtContainer = NULL;
	}

	if (m_pIItemProps)
	{
		m_pIItemProps->Release ();
		m_pIItemProps = NULL;
	}

	if (m_pIBrowse)
	{
		m_pIBrowse->Release ();
		m_pIBrowse = NULL;
	}

	if (m_pIPublicGroups)
	{
		m_pIPublicGroups->Release ();
		m_pIPublicGroups = NULL;
	}

	if (m_pIPersistFile)
	{
		m_pIPersistFile->Release ();
		m_pIPersistFile = NULL;
	}
}

// **************************************************************************
// ShutdownRequest ()
//
// Description:
//	2.0 OPC servers provide a notification if they are shutting down while 
//	we are still connected.  The notification comes through our IKShutdownSink
//  and is passed on to the server object through this function.
//
// Parameters:
//  LPCTSTR		lpszReason		String sent from OPC server describing reason
//								  for shutdown.  Could be NULL string.
//
// Returns:
//  void
// **************************************************************************
void CKServer::ShutdownRequest (LPCTSTR lpszReason)
{
	// Log "server requested shutdown" message:
	if (lpszReason)
		LogMsg (IDS_SERVER_REQUESTED_SHUTDOWN, GetProgID (), lpszReason);

	// Notify the document that an object has been shutdown (i.e., invalidated):

	// First get a pointer to the application's main window:
	CKMainWnd *pWnd = (CKMainWnd *) AfxGetMainWnd ();

	// Post the user defined "UM_SERVER_SHUTDOWN" message to the main window.
	// It will pass the message along to the docoument object.  It will expect
	// a pointer to this server object passed a the lParam.
	if (pWnd)
		pWnd->PostMessage (UM_SERVER_SHUTDOWN, 0, (LPARAM) this);
}
