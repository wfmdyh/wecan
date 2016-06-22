// **************************************************************************
// opcquality.h
//
// Description:
//	Defines OPC data qualities.  
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

// OpcQuality.h

// The quality and substatus bit fields have been combined
// into their respective constants. The Limit Bit Field may
// be or-ed with the quality/substatus constant to give the
// complete quality flag.

#define OPC_NO_QUALITY_NO_VALUE          0xFF

#define OPC_QUALITY_BAD_NON_SPECIFIC     0x0
#define OPC_QUALITY_BAD_CONFIG_ERR0R     0x04
#define OPC_QUALITY_BAD_NOT_CONNECTED    0x08
#define OPC_QUALITY_BAD_DEVICE_FAILURE   0x0C
#define OPC_QUALITY_BAD_SENSOR_FAILURE   0x10
#define OPC_QUALITY_BAD_LAST_KNOWN_VALUE 0x14
#define OPC_QUALITY_BAD_COMM_FAILURE     0x18
#define OPC_QUALITY_BAD_OUT_OF_SERVICE   0x1C

#define OPC_QUALITY_UNCERTAIN_NON_SPECIFIC         0x40
#define OPC_QUALITY_UNCERTAIN_LAST_USABLE_VALUE    0x44
#define OPC_QUALITY_UNCERTAIN_SENSOR_NOT_ACCURATE  0x50
#define OPC_QUALITY_UNCERTAIN_EU_UNITS_EXCEEDED    0x54
#define OPC_QUALITY_UNCERTAIN_SUB_NORMAL           0x58

#define OPC_QUALITY_GOOD_NON_SPECIFIC    0xC0
#define OPC_QUALITY_GOOD_LOCAL_OVERRIDE  0xD8

#define OPC_QUALITY_LIMITFIELD_NOT       0x0
#define OPC_QUALITY_LIMITFIELD_LOW       0x1
#define OPC_QUALITY_LIMITFIELD_HIGH      0x2
#define OPC_QUALITY_LIMITFIELD_CONSTANT  0x3

