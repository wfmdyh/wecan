
// TestCallPbx.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestCallPbxApp:
// �йش����ʵ�֣������ TestCallPbx.cpp
//

class CTestCallPbxApp : public CWinApp
{
public:
	CTestCallPbxApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestCallPbxApp theApp;