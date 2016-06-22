// **************************************************************************
// stdafx.h
//
// Description:
//	This is where we include files we wish to give global scope.
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


#ifndef _OPCTESTCLIENT_STDAFX_H
#define _OPCTESTCLIENT_STDAFX_H


#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxcview.h>
#include <afxmt.h>			// MFC support for multithreaded synchronization objects
#include <afxadv.h>
#include <process.h>
#include "ocidl.h"

#include "resource.h"
#include "globals.h"
#include "statusbartext.h"

#include "fixedsharedfile.h"
#include "editfilters.h"
#include "imagebutton.h"
#include "smarttooltip.h"
#include "safelock.h"
#include "timestmp.h"
#include "safearray.h"
#include "versioninfo.h"

#include "opcda.h"
#include "opccomn.h"
#include "opcerrors.h"
#include "opcprops.h"
#include "opcquality.h"

#include "comcat.h"


#endif // _OPCTESTCLIENT_STDAFX_H