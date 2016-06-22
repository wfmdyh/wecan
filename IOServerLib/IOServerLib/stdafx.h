// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
//禁用C函数警告
#pragma warning(disable:4996)

// Windows 头文件:
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <tchar.h>
//文件操作 
#include <direct.h>
#include <io.h>
#include <mbstring.h>
#include <OaIdl.h>

//STL
#include <string>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

//OPC
#include "opcda.h"
#include "opc_ae.h"
//XML
#include "..//xml_lib//tinyxml.h"
//OPC库
#pragma comment(lib,"..\\lib\\WTOPCsvr.lib")