// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>

#include <process.h>
#include <time.h>
#include <comutil.h>
#pragma comment(lib,"comsuppw.lib")

//�ļ����� 
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

