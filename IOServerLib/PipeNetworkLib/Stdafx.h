// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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
//�߳�
#include <process.h>
//Ŀ¼
#include <io.h>
#include <direct.h>
//ʱ���
#include <time.h>

#include "MyTools.h"

