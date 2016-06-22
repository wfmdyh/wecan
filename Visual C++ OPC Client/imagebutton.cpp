// **************************************************************************
// imagebutton.cpp
//
// Description:
//	Implements a "smart" bitmap button.  The default implementation supplied
//	by Windows is very nice if you don't need to gray out the bitmap.  This
//	subclass of a CButton accepts two bitmaps (one for enabled and one for
//	disabled) and assigns them according to the enabled state of the button.
//	Furthermore, the enabled state is checked on BN_CLICKED and if disabled
//	the notification will not go to the parent window.
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
#include "imagebutton.h"


// **************************************************************************
// Initialize ()
//
// Description:
//	Initialize object.	Load bitmaps and sets button to enabled state.
//
// Parameters:
//  UINT		uBMIDEnabled		Enabled bitmap resource.
//	UINT		uBMIDDisabled		Disabled bitmap resource.
//
// Returns:
//  void
// **************************************************************************
void CImageButton::Initialize (UINT uBMIDEnabled, UINT uBMIDDisabled)
	{
	// Load the enabled bitmap resource into our bitmap member variable:
	m_cbmEnabled.LoadBitmap (uBMIDEnabled);

	// Load the disabled bitmap resource into our bitmap member variable 
	// and attach it to window:
	m_cbmDisabled.Attach (::LoadImage (AfxGetResourceHandle (), MAKEINTRESOURCE (uBMIDDisabled),
		IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT));

	// Set initial state to enabled:
	m_bEnabled = true;
	}

// **************************************************************************
// SubclassDlgItem ()
//
// Description:
//	Call to dynamically subclass an instance of this class.
//
// Parameters:
//  UINT		nID			The control’s ID.
//	CWnd		*pParent	The control’s parent (usually a dialog box).
//
// Returns:
//  BOOL - TRUE if success.
// **************************************************************************
BOOL CImageButton::SubclassDlgItem (UINT nID, CWnd *pParent)
	{
	// Perform default processing.  Return FALSE if fail:
	if (!CButton::SubclassDlgItem (nID, pParent))
		{
		// Bad nID??
		ASSERT (FALSE);
		return (FALSE);
		}

	// Make sure Initialize was called (debug only):
	ASSERT (m_bEnabled);
	ASSERT (GetWindowLong (m_hWnd, GWL_STYLE) & BS_BITMAP);

	// Set the appropriate bitmap:
	if (IsWindowEnabled ())
		{
		SetBitmap (m_cbmEnabled);
		m_bEnabled = true;
		}
	else
		{
		SetBitmap (m_cbmDisabled);
		m_bEnabled = false;
		}
	
	// Return TRUE to indicate success:
	return (TRUE);
	}

// **************************************************************************
// EnableWindow ()
//
// Description:
//	Sets enabled state of image button.
//
// Parameters:
//  bool		bState		Set to true for enabled, false for disabled.
//
// Returns:
//  BOOL - Previous enabled state.
// **************************************************************************
BOOL CImageButton::EnableWindow (bool bState)
	{
	// Make sure subclassdlgitem was called (debug only):
	ASSERT (::IsWindow (m_hWnd));

	// Save previous state:
	BOOL bPreviousState = m_bEnabled;

	// If requested state is different than current, toggle bitmap and
	// window enabled state:
	if (m_bEnabled != bState)
		{
		// Save new state:
		m_bEnabled = bState;

		// Set bitmap based on state:
		SetBitmap ((bState) ? m_cbmEnabled : m_cbmDisabled);

		// Enable or disable window based on state:
		::EnableWindow (m_hWnd, bState);
		}

	// Return previous state:
	return (bPreviousState);
	}

