// EM9318B_VC_Socket.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CEM9318B_VC_SocketApp:
// �йش����ʵ�֣������ EM9318B_VC_Socket.cpp
//

class CEM9318B_VC_SocketApp : public CWinApp
{
public:
	CEM9318B_VC_SocketApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CEM9318B_VC_SocketApp theApp;