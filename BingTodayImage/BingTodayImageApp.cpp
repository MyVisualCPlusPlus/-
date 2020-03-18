
// BingTodayImage.cpp : ¶¨ÒåÓ¦ÓÃ³ÌÐòµÄÀàÐÐÎª¡£
//

#include "stdafx.h"
#include "BingTodayImageApp.h"
#include "BingTodayImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBingTodayImageApp

BEGIN_MESSAGE_MAP(CBingTodayImageApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBingTodayImageApp ¹¹Ôì

CBingTodayImageApp::CBingTodayImageApp()
{
	// Ö§³ÖÖØÐÂÆô¶¯¹ÜÀíÆ÷
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ÔÚ´Ë´¦Ìí¼Ó¹¹Ôì´úÂë£¬
	// ½«ËùÓÐÖØÒªµÄ³õÊ¼»¯·ÅÖÃÔÚ InitInstance ÖÐ
}


// Î¨Ò»µÄÒ»¸ö CBingTodayImageApp ¶ÔÏó

CBingTodayImageApp theApp;


// CBingTodayImageApp ³õÊ¼»¯

BOOL CBingTodayImageApp::InitInstance()
{
	m_hMutex = CreateMutex(NULL, FALSE, TEXT("{D5426629-9583-4231-B439-394DA68B2F28}"));
	if (ERROR_ALREADY_EXISTS == GetLastError())
		return FALSE;
	// Èç¹ûÒ»¸öÔËÐÐÔÚ Windows XP ÉÏµÄÓ¦ÓÃ³ÌÐòÇåµ¥Ö¸¶¨Òª
	// Ê¹ÓÃ ComCtl32.dll °æ±¾ 6 »ò¸ü¸ß°æ±¾À´ÆôÓÃ¿ÉÊÓ»¯·½Ê½£¬
	//ÔòÐèÒª InitCommonControlsEx()¡£  ·ñÔò£¬½«ÎÞ·¨´´½¨´°¿Ú¡£
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ½«ËüÉèÖÃÎª°üÀ¨ËùÓÐÒªÔÚÓ¦ÓÃ³ÌÐòÖÐÊ¹ÓÃµÄ
	// ¹«¹²¿Ø¼þÀà¡£
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//³õÊ¼»¯Winsock¿â
	if (!AfxSocketInit()) {
		AfxMessageBox(TEXT("Winsock¿â³õÊ¼»¯Ê§°Ü£¡"));
		return FALSE;
	}

	AfxEnableControlContainer();

	// ¼¤»î¡°Windows Native¡±ÊÓ¾õ¹ÜÀíÆ÷£¬ÒÔ±ãÔÚ MFC ¿Ø¼þÖÐÆôÓÃÖ÷Ìâ
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ±ê×¼³õÊ¼»¯
	// Èç¹ûÎ´Ê¹ÓÃÕâÐ©¹¦ÄÜ²¢Ï£Íû¼õÐ¡
	// ×îÖÕ¿ÉÖ´ÐÐÎÄ¼þµÄ´óÐ¡£¬ÔòÓ¦ÒÆ³ýÏÂÁÐ
	// ²»ÐèÒªµÄÌØ¶¨³õÊ¼»¯Àý³Ì
	// ¸ü¸ÄÓÃÓÚ´æ´¢ÉèÖÃµÄ×¢²á±íÏî
	// TODO: Ó¦ÊÊµ±ÐÞ¸Ä¸Ã×Ö·û´®£¬
	// ÀýÈçÐÞ¸ÄÎª¹«Ë¾»ò×éÖ¯Ãû
	SetRegistryKey(_T("Soon"));
	TCHAR szText[MAX_PATH];
	GetModuleFileName(NULL, szText, MAX_PATH);
	auto p = _tcsrchr(szText, TEXT('\\'));
	if (p)
		*p = 0;
	m_strModuleDir = szText;

	m_bAotuRun = GetProfileInt(TEXT("setting"), TEXT("AutoRun"), 0);
	m_bRandom = GetProfileInt(TEXT("setting"), TEXT("Random"), 0);
	// 导入程序运行天数
	auto strConfig = theApp.m_strModuleDir + TEXT("\\config.ini");
	m_nDay = ::GetPrivateProfileInt(TEXT("DayInfo"), TEXT("day"), 1, strConfig);
	auto pDlg = new CBingTodayImageDlg;
	m_pMainWnd = pDlg;
	pDlg->Create(IDD_BINGTODAYIMAGE_DIALOG);

	return TRUE;
}



int CBingTodayImageApp::ExitInstance()
{
	if (m_hMutex)
		CloseHandle(m_hMutex);
	return CWinApp::ExitInstance();
}

// ÏÔÊ¾´íÎóÐÅÏ¢
// ²ÎÊý£º
//	BOOL bUseMsgBox£ºÊÇ·ñÒÔMessageBoxµÄ·½Ê½ÏÔÊ¾´íÎó£¬falseÊ±ÒÔµ÷ÊÔÐÅÏ¢·½Ê½ÏÔÊ¾
//	LONG *lpErrCode£º´æ·Å´íÎó´úÂëµÄ±äÁ¿Ö¸Õë£¬Îª¿ÕÊ±º¯ÊýÊ¹ÓÃGetLastError()»ñÈ¡
void ShowErrMsg(BOOL bUseMsgBox, CString strPreMsg/* = TEXT("")*/, LONG *lpErrCode/* = NULL*/)
{
	LONG lErrCode;
	LPVOID lpMsgBuf;
	if (lpErrCode)
		lErrCode = *lpErrCode;
	else
		lErrCode = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		lErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	strPreMsg += (LPTSTR)lpMsgBuf;
	if (bUseMsgBox)
		AfxMessageBox(strPreMsg);
	else
		TRACE(strPreMsg);
	LocalFree(lpMsgBuf);
}
