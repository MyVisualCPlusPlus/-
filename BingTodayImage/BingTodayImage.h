#pragma once

#include <assert.h>
#include <string>
#include <xstring>

#define BUFF_LEN			(1024*512)

class CBingTodayImage
{
public:
	CBingTodayImage(void);
	~CBingTodayImage(void);

	bool GetTodayImage(LPCTSTR lpszSavePath);
	std::string GetDesc(){ return m_strDesc; }

private:
	bool ConnectBing(LPCSTR pHost);
	char *HttpGet(LPCSTR lpszFile, int &nDataLen);
	char *GetHttpItamData(LPSTR pHttpHdr, LPCSTR lpszItemName = nullptr)
	{
		assert(pHttpHdr);
		char szName[50] = {};

		if(!lpszItemName)
			strcpy_s(szName, "\r\n\r\n");
		else
			sprintf_s(szName, "%s: ", lpszItemName);
		auto pData = strstr(pHttpHdr, szName);
		if(pData)
			pData += strlen(szName);
		return pData;
	}
	char * CBingTodayImage::GetHttpData(LPSTR pHttp, int nLen, int &nDataLen);

private:
	SOCKET m_hBing;
	char *m_pBuff;
	std::string m_strDesc;
	static LPCSTR s_pHttpHdr;
};

