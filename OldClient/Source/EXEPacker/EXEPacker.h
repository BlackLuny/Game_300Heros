
// EXEPacker.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEXEPackerApp:
// �йش����ʵ�֣������ EXEPacker.cpp
//

class CEXEPackerApp : public CWinApp
{
public:
	CEXEPackerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEXEPackerApp theApp;