// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

//socket
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

//����ADO ���ݿ�
#import "C:\Program Files (x86)\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF","adoBOF")

//recv�Ĵ�С
#define DEFAULT_BUFLEN 1024

//�����С
#define DATA_BUF_SIZE					(1024*1024)

// �ͷž����
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

