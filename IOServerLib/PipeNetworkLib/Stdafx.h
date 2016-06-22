// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>



// TODO:  在此处引用程序需要的其他头文件
//socket
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

//STL
#include <list>
#include <queue>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
//COM
#include <comutil.h>
#pragma comment(lib,"comsuppw.lib")
//线程
#include <process.h>
//目录
#include <io.h>
#include <direct.h>
//时间戳
#include <time.h>

#include "MyTools.h"

