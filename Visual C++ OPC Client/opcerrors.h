// **************************************************************************
// opcerrors.h
//
// Description:
//	Defines OPC errors.  
//
//	File supplied by OPC Foundataion.
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


//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: OPC_E_INVALIDHANDLE
//
// MessageText:
//
//  The value of the handle is invalid
//
#define OPC_E_INVALIDHANDLE              ((HRESULT)0xC0040001L)

//
// MessageId: OPC_E_BADTYPE
//
// MessageText:
//
//  The server cannot convert the data between the requested data type and the canonical data type
//
#define OPC_E_BADTYPE                    ((HRESULT)0xC0040004L)

//
// MessageId: OPC_E_PUBLIC
//
// MessageText:
//
//  The requested operation cannot be done on a public group
//
#define OPC_E_PUBLIC                     ((HRESULT)0xC0040005L)

//
// MessageId: OPC_E_BADRIGHTS
//
// MessageText:
//
//  The Items AccessRights do not allow the operation
//
#define OPC_E_BADRIGHTS                  ((HRESULT)0xC0040006L)

//
// MessageId: OPC_E_UNKNOWNITEMID
//
// MessageText:
//
//  The item is no longer available in the server address space
//
#define OPC_E_UNKNOWNITEMID              ((HRESULT)0xC0040007L)

//
// MessageId: OPC_E_INVALIDITEMID
//
// MessageText:
//
//  The item definition doesn't conform to the server's syntax
//
#define OPC_E_INVALIDITEMID              ((HRESULT)0xC0040008L)

//
// MessageId: OPC_E_INVALIDFILTER
//
// MessageText:
//
//  The filter string was not valid
//
#define OPC_E_INVALIDFILTER              ((HRESULT)0xC0040009L)

//
// MessageId: OPC_E_UNKNOWNPATH
//
// MessageText:
//
//  The item's access path is not known to the server
//
#define OPC_E_UNKNOWNPATH                ((HRESULT)0xC004000AL)

//
// MessageId: OPC_E_RANGE
//
// MessageText:
//
//  The item's access path is not known to the server
//
#define OPC_E_RANGE                      ((HRESULT)0xC004000BL)

//
// MessageId: OPC_E_DUPLICATENAME
//
// MessageText:
//
//  Duplicate name not allowed
//
#define OPC_E_DUPLICATENAME              ((HRESULT)0xC004000CL)

//
// MessageId: OPC_S_UNSUPPORTEDRATE
//
// MessageText:
//
//  The server does not support the requested data rate but will use the closest available rate
//
#define OPC_S_UNSUPPORTEDRATE            ((HRESULT)0x0004000DL)

//
// MessageId: OPC_S_CLAMP
//
// MessageText:
//
//  A value passed to WRITE was accepted but the output was clamped
//
#define OPC_S_CLAMP                      ((HRESULT)0x0004000EL)

//
// MessageId: OPC_S_INUSE
//
// MessageText:
//
//  The operation cannot be completed because the object still has references that exist
//
#define OPC_S_INUSE                      ((HRESULT)0x0004000FL)

//
// MessageId: OPC_E_INVALIDCONFIGFILE
//
// MessageText:
//
//  The server's configuration file is an invalid format
//
#define OPC_E_INVALIDCONFIGFILE          ((HRESULT)0xC0040010L)

//
// MessageId: OPC_E_NOTFOUND
//
// MessageText:
//
//  The server could not locate the requested object
//
#define OPC_E_NOTFOUND                   ((HRESULT)0xC0040011L)

//
// MessageId: OPC_E_INVALID_PID
//
// MessageText:
//
//  The server does not recognise the passed property ID
//
#define OPC_E_INVALID_PID                ((HRESULT)0xC0040203L)

