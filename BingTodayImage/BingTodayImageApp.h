
// BingTodayImage.h : PROJECT_NAME Ó¦ÓÃ³ÌÐòµÄÖ÷Í·ÎÄ¼þ
//

#pragma once

#ifndef __AFXWIN_H__
	#error "ÔÚ°üº¬´ËÎÄ¼þÖ®Ç°°üº¬¡°stdafx.h¡±ÒÔÉú³É PCH ÎÄ¼þ"
#endif

#include "resource.h"		// Ö÷·ûºÅ


// CBingTodayImageApp: 
// ÓÐ¹Ø´ËÀàµÄÊµÏÖ£¬Çë²ÎÔÄ BingTodayImage.cpp
//

class CBingTodayImageApp : public CWinApp
{
public:
	CBingTodayImageApp();

// ÖØÐ´
public:
	virtual BOOL InitInstance();

// ÊµÏÖ

	DECLARE_MESSAGE_MAP()
public:
	bool m_bRandom;
	bool m_bAotuRun;
	HANDLE m_hMutex;
	CString m_strModuleDir;
	int m_nDay;
	virtual int ExitInstance();
};

extern CBingTodayImageApp theApp;