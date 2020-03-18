
// BingTodayImageDlg.cpp : ÊµÏÖÎÄ¼þ
//

#include "stdafx.h"
#include "BingTodayImageApp.h"
#include "BingTodayImageDlg.h"
#include "afxdialogex.h"
#include "BingTodayImage.h"
#include "locale.h"
#include <exiv2/exiv2.hpp>  
#include <codecvt>
#pragma comment(lib,"libexiv2.lib")  
#pragma comment(lib,"xmpsdk.lib")  
#pragma comment(lib,"libexpat.lib")  
#pragma comment(lib,"zlib1.lib")  

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_GETBINGIMAGE				(WM_USER + 1024)
#define TIMER_DOWNLOAD 1
#define TIMER_SWITCH 2

void IncDay(SYSTEMTIME &stm)
{
	if (stm.wMonth == 2) {
		if (stm.wDay == 29 || (stm.wDay == 28 && (stm.wDay % 400 && (stm.wYear % 4 || !(stm.wYear % 100))))) {
			stm.wDay = 1;
			stm.wMonth += 1;
		}
		else {
			stm.wDay += 1;
		}
	}
	else if (stm.wDay == 30 && (stm.wMonth == 4 || stm.wMonth == 6 || stm.wMonth == 9 || stm.wMonth == 11)) {
		stm.wDay = 1;
		stm.wMonth += 1;
	}
	else if (stm.wDay == 31 && (stm.wMonth == 1 || stm.wMonth == 3 || stm.wMonth == 5 || stm.wMonth == 7 ||
		stm.wMonth == 8 || stm.wMonth == 10 || stm.wMonth == 12)) {
		stm.wDay = 1;
		if (stm.wMonth == 12) {
			stm.wMonth = 1;
			stm.wYear += 1;
		}
		else
			stm.wMonth += 1;

	}
	else {
		stm.wDay += 1;
	}
}

void WINAPI APCFunc(ULONG_PTR dwParam)
{

}

UINT GetBingTodayImage(void *pParam)
{
	auto pdlg = (CBingTodayImageDlg*)pParam;
	CString strDay = _T("");
	strDay.Format(_T("%d"), theApp.m_nDay);
	auto strDir = theApp.m_strModuleDir + TEXT("\\wallpaper\\");
	auto strTemp = strDir + strDay + TEXT(".jpg"); 
	for (;;) {
		CBingTodayImage bti;
		if (bti.GetTodayImage(strTemp)) {
			pdlg->m_listWallpaper.push_back(strTemp);
			pdlg->m_listCurWallpaper.push_back(strTemp);
			
			USES_CONVERSION;
			pdlg->SetWallpaperText(T2A(strTemp.GetBuffer(0)), bti.GetDesc());
			CString strDesc(bti.GetDesc().c_str());
			pdlg->SendMessage(WM_GETBINGIMAGE, (WPARAM)strDesc.GetBuffer());
			strDesc.ReleaseBuffer();
			pdlg->SetWallpaper(strTemp);
			CString strNextDay;
			strNextDay.Format(TEXT("%d"), theApp.m_nDay + 1);
			auto strConfig = theApp.m_strModuleDir + TEXT("\\config.ini");
			::WritePrivateProfileString(TEXT("DayInfo"), TEXT("day"), strNextDay, strConfig);
			break;
		}
		if (WAIT_IO_COMPLETION == SleepEx(3000, TRUE))
			break;
	}

	return 0;
}

std::string CBingTodayImageDlg::toUtf8(const std::wstring &str)
{
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len, NULL, NULL);
	}
	return ret;
}

void CBingTodayImageDlg::InitWallpaper()
{
	m_listWallpaper.clear();
	m_listCurWallpaper.clear();
	auto strDir = theApp.m_strModuleDir + TEXT("\\wallpaper\\");
	
	// 获取目录下的文件列表
	CFileFind find;
	auto findFilter = strDir + _T("/*.*");
	BOOL IsFind = find.FindFile(findFilter);

	while (IsFind)
	{
		IsFind = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}
		else
		{
			CString fullname = strDir + find.GetFileName();
			m_listWallpaper.push_back(fullname);
			m_listCurWallpaper.push_back(fullname);
		}
	}
}

void CBingTodayImageDlg::SetWallpaper(CString strWallpaper)
{
#ifndef DEBUG
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)strWallpaper.GetString(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
#endif // !DEBUG
}

void CBingTodayImageDlg::SetWallpaperText(std::string strWallpaper, std::string strDesc)
{
	if (!Exiv2::fileExists(strWallpaper))
	{
		return;
	}
	Exiv2::Image::AutoPtr &image = Exiv2::ImageFactory::open(strWallpaper);
	if (image.get() == nullptr) return;

	image->readMetadata();
	Exiv2::ExifData &exifData = image->exifData();
	exifData["Exif.Photo.UserComment"] = strDesc;
	std::cout << exifData["Exif.Photo.UserComment"] << std::endl;
	image->writeMetadata();
	
	return;
}

std::string CBingTodayImageDlg::GetWallpaperText(std::string strWallpaper)
{
	if (!Exiv2::fileExists(strWallpaper))
	{
		return "";
	}
	Exiv2::Image::AutoPtr &image = Exiv2::ImageFactory::open(strWallpaper);
	if (image.get() == nullptr) return "";

	image->readMetadata();
	Exiv2::ExifData &exifData = image->exifData();
	return exifData["Exif.Photo.UserComment"].toString();

}

// CBingTodayImageDlg 任务栏

UINT CBingTodayImageDlg::s_msgTaskBarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

CBingTodayImageDlg::CBingTodayImageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BINGTODAYIMAGE_DIALOG, pParent)
	, m_pImageThread(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBingTodayImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBingTodayImageDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_NOTIFYICON, &CBingTodayImageDlg::OnNotifyicon)
	ON_REGISTERED_MESSAGE(s_msgTaskBarRestart, &CBingTodayImageDlg::OnTaskBarRestart)
	ON_COMMAND(ID_NOTIFY_AUTORUN, &CBingTodayImageDlg::OnNotifyAutorun)
	ON_COMMAND(ID_NOTIFY_EXIT, &CBingTodayImageDlg::OnNotifyExit)
	ON_MESSAGE(WM_GETBINGIMAGE, &CBingTodayImageDlg::OnGetBingImage)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_NOTIFY_RANDOM, &CBingTodayImageDlg::OnNotifyRandom)
END_MESSAGE_MAP()


// CBingTodayImageDlg ÏûÏ¢´¦Àí³ÌÐò

BOOL CBingTodayImageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ½«¡°¹ØÓÚ...¡±²Ëµ¥ÏîÌí¼Óµ½ÏµÍ³²Ëµ¥ÖÐ¡£

	// IDM_ABOUTBOX ±ØÐëÔÚÏµÍ³ÃüÁî·¶Î§ÄÚ¡£
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	setlocale(LC_CTYPE, "chs");
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ÉèÖÃ´Ë¶Ô»°¿òµÄÍ¼±ê¡£  µ±Ó¦ÓÃ³ÌÐòÖ÷´°¿Ú²»ÊÇ¶Ô»°¿òÊ±£¬¿ò¼Ü½«×Ô¶¯
	//  Ö´ÐÐ´Ë²Ù×÷
	SetIcon(m_hIcon, TRUE);			// ÉèÖÃ´óÍ¼±ê
	SetIcon(m_hIcon, FALSE);		// ÉèÖÃÐ¡Í¼±ê

	m_nid.cbSize = sizeof(m_nid);
	m_nid.uID = 20;
	m_nid.hWnd = GetSafeHwnd();
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME));
	m_nid.uCallbackMessage = WM_NOTIFYICON;
	_tcscpy_s(m_nid.szTip, TEXT("必应每日壁纸"));
	Shell_NotifyIcon(NIM_ADD, &m_nid);

	CheckAutoRun();

	m_strLastChangeData = theApp.GetProfileString(TEXT("BingTodayImage"), TEXT("lastChangeData"));
	InitWallpaper();
	SetTimer(TIMER_DOWNLOAD, 1000, nullptr);
	
	ShowWindow(SW_HIDE);

	return TRUE;  // ³ý·Ç½«½¹µãÉèÖÃµ½¿Ø¼þ£¬·ñÔò·µ»Ø TRUE
}


// Èç¹ûÏò¶Ô»°¿òÌí¼Ó×îÐ¡»¯°´Å¥£¬ÔòÐèÒªÏÂÃæµÄ´úÂë
//  À´»æÖÆ¸ÃÍ¼±ê¡£  ¶ÔÓÚÊ¹ÓÃÎÄµµ/ÊÓÍ¼Ä£ÐÍµÄ MFC Ó¦ÓÃ³ÌÐò£¬
//  Õâ½«ÓÉ¿ò¼Ü×Ô¶¯Íê³É¡£

void CBingTodayImageDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ÓÃÓÚ»æÖÆµÄÉè±¸ÉÏÏÂÎÄ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ê¹Í¼±êÔÚ¹¤×÷Çø¾ØÐÎÖÐ¾ÓÖÐ
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// »æÖÆÍ¼±ê
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//µ±ÓÃ»§ÍÏ¶¯×îÐ¡»¯´°¿ÚÊ±ÏµÍ³µ÷ÓÃ´Ëº¯ÊýÈ¡µÃ¹â±ê
//ÏÔÊ¾¡£
HCURSOR CBingTodayImageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CBingTodayImageDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CBingTodayImageDlg::CheckAutoRun()
{
	HKEY hKey;
	TCHAR szPath[MAX_PATH] = {};
	DWORD dwLen = MAX_PATH * sizeof(TCHAR);
	LPCTSTR lpszName = TEXT("BingTodayImage");
	auto nRes = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		0, KEY_ALL_ACCESS, &hKey);
	if (nRes != ERROR_SUCCESS) {
		ShowErrMsg(TRUE);
		return;
	}
	nRes = RegQueryValueEx(hKey, lpszName, NULL, NULL, (LPBYTE)szPath, &dwLen);
	if (theApp.m_bAotuRun && nRes != ERROR_SUCCESS) {
		GetModuleFileName(NULL, szPath, MAX_PATH);
		RegSetValueEx(hKey, lpszName, 0, REG_SZ, (LPBYTE)szPath, (_tcslen(szPath) + 1) * sizeof(TCHAR));
	}
	else if (!theApp.m_bAotuRun && nRes == ERROR_SUCCESS) {
		RegDeleteValue(hKey, lpszName);
	}
	RegCloseKey(hKey);
}

LRESULT CBingTodayImageDlg::OnTaskBarRestart(WPARAM wParam, LPARAM lParam)
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	m_nid.uFlags &= ~NIF_INFO;
	Shell_NotifyIcon(NIM_ADD, &m_nid);
	return LRESULT();
}

LRESULT CBingTodayImageDlg::OnNotifyicon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case WM_RBUTTONUP:
	{
		CPoint pt;
		GetCursorPos(&pt);
		SetForegroundWindow(); //

		CMenu menu;
		menu.LoadMenu(IDR_NOTIFY);
		auto pSubMenu = menu.GetSubMenu(0);
		pSubMenu->CheckMenuItem(ID_NOTIFY_RANDOM, theApp.m_bRandom ? MF_CHECKED : MF_UNCHECKED);
		pSubMenu->CheckMenuItem(ID_NOTIFY_AUTORUN, theApp.m_bAotuRun ? MF_CHECKED : MF_UNCHECKED);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		menu.DestroyMenu();
	}
	break;
	default:
		break;
	}
	return 0;
	return LRESULT();
}


void CBingTodayImageDlg::SetNotifyTips(CString strTips)
{
	m_nid.uFlags |= NIF_INFO;
	m_nid.dwInfoFlags = NIIF_INFO;
	m_nid.uTimeout = 1000;
	_tcscpy_s(m_nid.szInfoTitle, TEXT("必应每日壁纸"));
	_tcscpy_s(m_nid.szInfo,strTips.GetBuffer(0));
	strTips.ReleaseBuffer();
	_tcscpy_s(m_nid.szTip, strTips.GetBuffer(0));
	strTips.ReleaseBuffer();
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

LRESULT CBingTodayImageDlg::OnGetBingImage(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam);
	SetNotifyTips((LPCTSTR)wParam);

	SYSTEMTIME stm;
	GetLocalTime(&stm);
#ifndef DEBUG
	m_strLastChangeData.Format(TEXT("%u%02u%02u"), stm.wYear, stm.wMonth, stm.wDay);
	theApp.WriteProfileString(TEXT("BingTodayImage"), TEXT("lastChangeData"), m_strLastChangeData);
#endif // !DEBUG

	SetChangeImageTimer(stm);
	return 0;
}

void CBingTodayImageDlg::SetChangeImageTimer(SYSTEMTIME &stmNow)
{
	DWORD dwElapse;
	LARGE_INTEGER liNow, liNext;

	SystemTimeToFileTime(&stmNow, (PFILETIME)&liNow);
	stmNow.wHour = 8;
	stmNow.wMinute = stmNow.wSecond = stmNow.wMilliseconds = 0;
	IncDay(stmNow);
	SystemTimeToFileTime(&stmNow, (PFILETIME)&liNext);
	dwElapse = (liNext.QuadPart - liNow.QuadPart) / 10000;
	SetTimer(TIMER_DOWNLOAD, dwElapse, nullptr);
	SetTimer(TIMER_SWITCH, 1000, nullptr);
}

void CBingTodayImageDlg::OnNotifyRandom()
{
	theApp.m_bRandom = !theApp.m_bRandom;
	theApp.WriteProfileInt(TEXT("BingTodayImage"), TEXT("Random"), theApp.m_bRandom);
}

void CBingTodayImageDlg::OnNotifyAutorun()
{
	theApp.m_bAotuRun = !theApp.m_bAotuRun;
	theApp.WriteProfileInt(TEXT("BingTodayImage"), TEXT("AutoRun"), theApp.m_bAotuRun);
	CheckAutoRun();
}


void CBingTodayImageDlg::OnNotifyExit()
{
	SendMessage(WM_CLOSE);
}


void CBingTodayImageDlg::OnOK()
{
}


void CBingTodayImageDlg::OnCancel()
{
	DestroyWindow();
}


void CBingTodayImageDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_pImageThread) {
		QueueUserAPC(APCFunc, m_pImageThread->m_hThread, 0);
		WaitForSingleObject(m_pImageThread->m_hThread, INFINITE);
		delete m_pImageThread;
	}
	KillTimer(TIMER_DOWNLOAD);
	KillTimer(TIMER_SWITCH);
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}


void CBingTodayImageDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_DOWNLOAD) {
		KillTimer(nIDEvent);
		SYSTEMTIME stm;
		TCHAR szText[12];
		GetLocalTime(&stm);
		_stprintf_s(szText, TEXT("%u%02u%02u"), stm.wYear, stm.wMonth, stm.wDay);
		if (m_strLastChangeData != szText) {
			if (m_pImageThread)
				delete m_pImageThread;
			m_pImageThread = AfxBeginThread(GetBingTodayImage, this, 0, 0, CREATE_SUSPENDED);
			if (m_pImageThread) {
				m_pImageThread->m_bAutoDelete = FALSE;
				m_pImageThread->ResumeThread();
			}
		}
		else
			SetChangeImageTimer(stm);
	} else if (nIDEvent == TIMER_SWITCH)
	{
		// 随机设置壁纸
		if (m_listWallpaper.empty())
		{
			KillTimer(nIDEvent);
			SetTimer(TIMER_DOWNLOAD, 1000, nullptr);
			return;
		}
		if (m_listCurWallpaper.empty())
		{
			std::copy(m_listWallpaper.begin(), m_listWallpaper.end(), m_listCurWallpaper.begin());
		}
		srand(time(NULL));
		auto wallPaperIndex = rand() % m_listCurWallpaper.size();
		CString strWallpaper;
		std::list<CString>::iterator iter = m_listCurWallpaper.begin();
		for (int i = 0; i < wallPaperIndex; iter++, i++);

		strWallpaper = (*iter);
		USES_CONVERSION;
		std::string strWallText = GetWallpaperText(T2A(strWallpaper.GetBuffer(0)));
		
		CString strDesc(strWallText.c_str());
		SetNotifyTips(strDesc);
		SetWallpaper(strWallpaper);
		m_listCurWallpaper.erase(iter); // 移除元素，后面该迭代器不能再用
		// 重新设定一个随机时间定时器
		KillTimer(nIDEvent);
		const int arrayElapse[] = { 60, 300, 600, 1800, 3600, 7200, 86400}; // 1分钟，5分钟，10分钟，30分钟，1小时，2小时，1天
		int arraySize = sizeof(arrayElapse) / sizeof(int);
		int deltaIndex = rand() % (arraySize);
		SetTimer(TIMER_SWITCH, arrayElapse[deltaIndex] * 1000, NULL);
	}
	CDialogEx::OnTimer(nIDEvent);
}

std::wstring CBingTodayImageDlg::s2ws(const std::string& s)
{
	size_t i;
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const char* _source = s.c_str();
	size_t _dsize = s.size() + 1;
	wchar_t* _dest = new wchar_t[_dsize];
	wmemset(_dest, 0x0, _dsize);
	mbstowcs_s(&i, _dest, _dsize, _source, _dsize);
	std::wstring result = _dest;
	delete[] _dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}


// convert UTF-8 string to wstring  
std::wstring CBingTodayImageDlg::utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

//GB2312到UTF-8的转换
std::string CBingTodayImageDlg::G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string strResult(str);
	delete[] str;

	return strResult;
}

std::string CBingTodayImageDlg::U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string strResult(str);
	delete[] str;

	return strResult;
}