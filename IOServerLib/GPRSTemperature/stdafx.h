// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

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
using namespace std;

//socket
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

