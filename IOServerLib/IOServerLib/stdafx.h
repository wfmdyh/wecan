// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
//����C��������
#pragma warning(disable:4996)

// Windows ͷ�ļ�:
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <tchar.h>
//�ļ����� 
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
//OPC��
#pragma comment(lib,"..\\lib\\WTOPCsvr.lib")