
// BingTodayImageDlg.h : ͷ�ļ�
//

#pragma once
#include <list>

// CBingTodayImageDlg �Ի���
class CBingTodayImageDlg : public CDialogEx
{
// ����
public:
	CBingTodayImageDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BINGTODAYIMAGE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CWinThread *m_pImageThread;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	static UINT s_msgTaskBarRestart;
	NOTIFYICONDATA m_nid;
	CString m_strLastChangeData;
	std::list<CString> m_listWallpaper;
	std::list<CString> m_listCurWallpaper;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void CheckAutoRun();
	void InitWallpaper();
	void SetWallpaper(CString strWallpaper);
	void SetWallpaperText(std::string strWallpaper, std::string strDesc);
	void SetNotifyTips(CString strTips);
	std::string CBingTodayImageDlg::U2G(const char* utf8);
	std::string G2U(const char* gb2312);
	std::wstring utf8_to_wstring(const std::string& str);
	std::wstring s2ws(const std::string& s);
	std::string toUtf8(const std::wstring &str);
	std::string GetWallpaperText(std::string strWallpaper);
	afx_msg LRESULT OnTaskBarRestart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotifyicon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetBingImage(WPARAM wParam, LPARAM lParam);
	void SetChangeImageTimer(SYSTEMTIME &stmNow);
	afx_msg void OnNotifyAutorun();
	afx_msg void OnNotifyExit();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNotifyRandom();
};
