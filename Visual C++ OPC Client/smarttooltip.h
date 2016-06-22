// **************************************************************************
// smarttooltip.h
//
// Description:
//	Defines a CToolTipCtrl wrapper class.
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


#ifndef _SMARTTOOLTIP_H
#define _SMARTTOOLTIP_H


// **************************************************************************
class CSmartToolTip	: public CToolTipCtrl
	{
	public:
		BOOL AddWindowTool (CWnd *pWnd);
		BOOL AddWindowTool (CWnd *pWnd, int nStringID);
		BOOL AddWindowTool (CWnd *pWnd, LPCTSTR lpsz);

	};


#endif	// _SMARTTOOLTIP_H
