// **************************************************************************
// globals.h
//
// Description:
//	This is where various items that have global scope are declared.
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


// useful defines
#define DEFBUFFSIZE					_MAX_PATH		// default buffer size

// OPC_XX_DATAXX defines are based on an index in a listbox (do not modify) 
#define OPC_10_DATACHANGE			0	// OnDataChange 1.0 no timestamp request 
#define OPC_10_DATATIMECHANGE		1	// OnDataChange 1.0 with timestamp request 
#define OPC_20_DATACHANGE			2	// OnDataChange 2.0 (always a timestamp) request 

// group defaults
#define GROUP_DEFAULT_ACTIVESTATE	TRUE
#define GROUP_DEFAULT_LANGUAGEID	MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US)
#define GROUP_DEFAULT_NAME			_T("")
#define GROUP_DEFAULT_DEADBAND		0.0f
#define GROUP_DEFAULT_TIMEBIAS		0
#define GROUP_DEFAULT_UPDATERATE	100
#define GROUP_DEFAULT_UPDATEMETHOD	OPC_20_DATACHANGE
#define GROUP_DEFAULT_FORCEDELETE	FALSE
#define GROUP_DEFAULT_REMOVEITEMSONDELETE	TRUE
#define GROUP_DEFAULT_ACTIVATEITEMSONSELECT	TRUE	

// item defaults
#define ITEM_DEFAULT_ACCESSPATH		_T("")
#define ITEM_DEFAULT_ITEMID			_T("")
#define ITEM_DEFAULT_ACTIVESTATE	TRUE
#define ITEM_DEFAULT_DATATYPE		VT_EMPTY

// view update interval defaults/defines
#define VIEW_DEFAULT_INTERVAL		250
#define VIEW_MIN_INTERVAL			10
#define VIEW_MAX_INTERVAL			30000

// user defined hints
#define HINT_ADD_SERVER		1000
#define HINT_REMOVE_SERVER	1001
#define HINT_SELECT_SERVER	1002

#define HINT_ADD_GROUP		1020
#define HINT_REMOVE_GROUP	1021
#define HINT_SELECT_GROUP	1022
#define HINT_ADD_SERVER_AND_GROUPS 1023

#define HINT_ADD_ITEM		1040
#define HINT_REMOVE_ITEM	1041
#define HINT_READD_ITEM		1042

#define HINT_LOAD_PROJECT		1060
#define HINT_CLOSE_PROJECT		1061
#define HINT_REFRESH_GROUPVIEW	1062
#define HINT_REFRESH_ITEMVIEW	1063

// user defined messages
#define UM_ONUPARROW			(WM_USER + 1)
#define UM_ONDNARROW			(WM_USER + 2)
#define UM_LISTEDIT_ITEMCHANGE	(WM_USER + 3)
#define UM_SERVER_SHUTDOWN		(WM_USER + 4)
#define UM_SELECT_GROUP			(WM_USER + 5)
#define UM_REFRESH_ITEMVIEW		(WM_USER + 6)
#define UM_PASTE_ITEMS			(WM_USER + 7)
#define UM_ITEM_READD			(WM_USER + 8)
#define UM_UPDATEEVENTLOG		(WM_USER + 9)
#define UM_CHANGEVIEW			(WM_USER + 10)

// event defines/types
#define EVENT_FIRSTINFO		15000
#define EVENT_LASTINFO		15999
#define EVENT_FIRSTERROR	16000
#define EVENT_LASTERROR		16999

typedef enum
	{
	tEventInformation = 0,	// define resource IDs between EVENT_FIRSTINFO and LASTINFO
	tEventError				// define resource IDs between EVENT_FIRSTERROR and LASTERROR
	} EVENTTYPE;

// clipboard formats for data access 1.0 streams
extern UINT CF_DATA_CHANGE;
extern UINT CF_DATA_CHANGE_TIME;
extern UINT CF_WRITE_COMPLETE;

// clipboard formats for cut/copy/paste
extern UINT CF_SERVER;
extern UINT CF_GROUP;
extern UINT CF_ITEM;

// So we can generically handle TCHAR arrays
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

// Debug versus release mode stuff that must be setup prior to
// inclusion of other header files
#ifdef _DEBUG

	// Generic malloc failure message
	extern LPCTSTR _dbg_szMallocFailedMsg;
	#define TRACEMALLOCFAILED() TRACE (_dbg_szMallocFailedMsg, _T(__FILE__), __LINE__)

	extern LPCTSTR _dbg_szExceptionMsg;
	#define EXCEPTIONMSG() TRACE (_dbg_szExceptionMsg, _T(__FILE__), __LINE__)

#else

	#define TRACEMALLOCFAILED()
	#define EXCEPTIONMSG()

#endif // _DEBUG

// function prototypes
void LogMsg (UINT nResID, ...);
void UpdateItemCount (int nDelta);

VARTYPE VartypeFromString (LPCTSTR lpszType);
void StringFromVartype (VARTYPE vtType, CString &strType);

// the one and only app
#include "opctestclient.h"
extern CKApp cApp;

#define STR_OK _T("确定")
#define STR_CANCEL _T("取消")
#define STR_APPLY _T("应用")