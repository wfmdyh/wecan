#pragma once

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ

#include <Windows.h>

//STL
#include <vector>
#include <list>
using namespace std;

#include <comutil.h>
#pragma comment(lib,"comsuppw.lib")

//C#
#include <stdlib.h>
#include <msclr\marshal.h>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;
using namespace DatabaseServer;
using namespace WaterMeterBLL;
using namespace msclr::interop;

//#include <msclr\marshal.h>
//#include <MMSystem.h>
//#pragma comment(lib, "winmm.lib") 