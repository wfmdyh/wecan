// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>



// TODO:  在此处引用程序需要的其他头文件
#include <process.h>
#include <time.h>
#include <comutil.h>
#pragma comment(lib,"comsuppw.lib")

//文件操作 
#include <direct.h>
#include <io.h>

#include <stdio.h>
#include <stdarg.h>

//STL
#include <list>
#include <queue>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

//socket
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

//引入ADO 数据库
#import "C:\Program Files (x86)\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF","adoBOF")

//recv的大小
#define DEFAULT_BUFLEN 1024

//缓存大小
#define DATA_BUF_SIZE					(1024*1024)

// 释放句柄宏
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}

#include "DevConcentrator.h"
#include "DeviceConnectCon.h"
#include "DevCollector.h"
#include "DevRepeater.h"
#include "DevConBigMeter.h"
#include "DevBigMeterException.h"
#include "SendCommand.h"
#include "DevCommand.h"
#include "SmallMeter.h"


#include "DataDef.h"
#include "GPRSBigMeter.h"
#include "DeviceConnect.h"
#include "MyTools.h"
#include "channel.h"
#include "MyDB.h"
#include "IOCPModel.h"
#include "TCPModelConcentrator.h"
#include "DataBaseManager.h"
#include "ProtocolManager.h"

