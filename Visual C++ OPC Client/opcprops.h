// **************************************************************************
// opcprops.h
//
// Description:
//	Defines OPC properites.  
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

/*++
Module Name:
 OPCProps.h
Author:
OPC Task Force

Revision History:
Release 2.0
     Created
--*/

/*
Property ID Code Assignements:
  0000 to 4999 are reserved for OPC use 

*/

#ifndef __OPCPROPS_H
#define __OPCPROPS_H

#define OPC_PROP_CDT        1
#define OPC_PROP_VALUE      2
#define OPC_PROP_QUALITY    3
#define OPC_PROP_TIME       4
#define OPC_PROP_RIGHTS     5
#define OPC_PROP_SCANRATE   6

#define OPC_PROP_UNIT       100
#define OPC_PROP_DESC       101
#define OPC_PROP_HIEU       102
#define OPC_PROP_LOEU       103
#define OPC_PROP_HIRANGE    104
#define OPC_PROP_LORANGE    105
#define OPC_PROP_CLOSE      106
#define OPC_PROP_OPEN       107
#define OPC_PROP_TIMEZONE   108

#define OPC_PROP_FGC        200
#define OPC_PROP_BGC        201
#define OPC_PROP_BLINK      202
#define OPC_PROP_BMP        203
#define OPC_PROP_SND        204
#define OPC_PROP_HTML       205
#define OPC_PROP_AVI        206

#define OPC_PROP_ALMSTAT    300
#define OPC_PROP_ALMHELP    301
#define OPC_PROP_ALMAREAS   302
#define OPC_PROP_ALMPRIMARYAREA  303
#define OPC_PROP_ALMCONDITION    304
#define OPC_PROP_ALMLIMIT   305
#define OPC_PROP_ALMDB      306
#define OPC_PROP_ALMHH      307
#define OPC_PROP_ALMH       308
#define OPC_PROP_ALML       309
#define OPC_PROP_ALMLL      310
#define OPC_PROP_ALMROC     311
#define OPC_PROP_ALMDEV     312

#endif