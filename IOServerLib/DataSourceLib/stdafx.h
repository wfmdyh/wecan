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
// TODO: 在此处引用程序需要的其他头文件
//文件操作 
#include <direct.h>
#include <io.h>
#include <mbstring.h>

//STL
#include <string>
#include <vector>
#include <map>
using namespace std;
//COM
#include <OaIdl.h>

//点参数
typedef map<string, string> POINT_PARAM;



