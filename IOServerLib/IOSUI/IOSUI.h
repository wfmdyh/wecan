
// IOSUI.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CIOSUIApp: 
// �йش����ʵ�֣������ IOSUI.cpp
//

class CIOSUIApp : public CWinApp
{
private:
	HMODULE m_hIOServerMod;
public:
	CIOSUIApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
public:
	//����IOServer��̬��
	BOOL LoadIOSLibrary();
};

extern CIOSUIApp theApp;