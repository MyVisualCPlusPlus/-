#include "StdAfx.h"
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <atlimage.h>
#include "BingTodayImage.h"
#include "rapidjson/document.h"
#include <vector>
#include <codecvt>

#pragma comment(lib, "ws2_32.lib")

LPCSTR CBingTodayImage::s_pHttpHdr = 
	"GET %s HTTP/1.1\r\n"
	"Proxy-Connection: keep-alive\r\n"
	"Upgrade-Insecure-Requests: 1\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 Safari/537.36\r\n"
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
	"Accept-Language: zh-CN,zh;q=0.9\r\n"
	"Host: cn.bing.com\r\n"
	"Content-Length: 0\r\n"
	"\r\n";


CBingTodayImage::CBingTodayImage(void)
	: m_hBing(INVALID_SOCKET)
	, m_pBuff(nullptr)
{
}


CBingTodayImage::~CBingTodayImage(void)
{
	if(m_hBing != INVALID_SOCKET)
		closesocket(m_hBing);
	if(m_pBuff)
		delete[] m_pBuff;
}

bool CBingTodayImage::GetTodayImage(LPCTSTR lpszSavePath)
{
	static PCSTR s_pHost[] = {
		"www.cn.bing.com",
		"www.bing.com",
		NULL
	};
	static PCSTR s_images = "images";
	static PCSTR s_url = "url";
	static PCSTR s_copyright = "copyright";

	int i = 0;
	while(s_pHost[i] && !ConnectBing(s_pHost[i])) ++i;
	if(!s_pHost[i])
		return false;
	int nLen;
	auto pHttp = HttpGet("/HPImageArchive.aspx?format=js&idx=0&n=1&nc=1421741858945&pid=hp", nLen);
	if(!pHttp)
		return false;

	int nDataLen;
	auto pData = GetHttpData(pHttp, nLen, nDataLen);
	if (!pData)
		return false;
	try {
		rapidjson::Document doc;
		doc.Parse(pData);
		if (!doc.HasMember(s_images) || !doc[s_images].IsArray())
			return false;
		auto& image = (doc[s_images])[0];
		if (!image.HasMember(s_url) || !image[s_url].IsString())
			return false;
		auto url = image[s_url].GetString();
		if (image.HasMember(s_copyright) && image[s_copyright].IsString())
		{
			m_strDesc = image[s_copyright].GetString();
			
			int nPos = m_strDesc.find("(");
			if (nPos != -1)
			{
				m_strDesc = m_strDesc.assign(m_strDesc.c_str(), nPos);
			}
		}
			
		pHttp = HttpGet(url, nLen);
		if (!pHttp)
			return false;
	}
	catch (...) {
		return false;
	}

	
	pData = GetHttpData(pHttp, nLen, nDataLen);
	auto pFileSize = GetHttpItamData(pHttp, "Content-Length");
	if (!pData || !pFileSize)
		return false;
	int nFileSize = atoi(pFileSize);

	std::fstream f(lpszSavePath, std::ios::binary | std::ios::out);
	if(!f)
		return false;
	f.write(pData, nDataLen);
	for(nFileSize -= nDataLen;
		nFileSize > 0 && SOCKET_ERROR != (nDataLen = recv(m_hBing, m_pBuff, BUFF_LEN, 0));
		nFileSize -= nDataLen){
			f.write(m_pBuff, nDataLen);
	}
	f.close();
	CImage image;
	image.Load(lpszSavePath);
	image.Save(lpszSavePath);

	return true;
}

bool CBingTodayImage::ConnectBing(LPCSTR pHost)
{
	ADDRINFO *pRes = nullptr, hints = {};
	hints.ai_family = AF_INET;

	auto nRes = getaddrinfo(pHost, "http", &hints, &pRes);
	if(nRes)
		return false;

	SOCKADDR_IN svrAddr = {};
	for(auto p = pRes; p; p = p->ai_next){
		if(p->ai_addr->sa_family == AF_INET){
			svrAddr = *((SOCKADDR_IN*)p->ai_addr);
			break;
		}
	}

	freeaddrinfo(pRes);
	if(svrAddr.sin_family != AF_INET)
		return false;

	m_hBing = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	if(m_hBing == INVALID_SOCKET)
		return false;

	SOCKADDR_IN localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = 0;

	if(SOCKET_ERROR == bind(m_hBing, (sockaddr*)&localAddr, sizeof(localAddr)))
		return false;

	if(SOCKET_ERROR == connect(m_hBing, (sockaddr*)&svrAddr, sizeof(svrAddr)))
		return false;
	return true;
}

char * CBingTodayImage::HttpGet(LPCSTR lpszFile, int &nDataLen)
{
	bool bRet = false;
	if(!m_pBuff){
		m_pBuff = new char[BUFF_LEN];
		if(!m_pBuff)
			return nullptr;
	}

	sprintf_s(m_pBuff, BUFF_LEN, s_pHttpHdr, lpszFile);

	if(SOCKET_ERROR == send(m_hBing, m_pBuff, strlen(m_pBuff), 0))
		return nullptr;

	nDataLen = recv(m_hBing, m_pBuff, BUFF_LEN, 0);
	char *pData = nullptr;
	if (nDataLen != SOCKET_ERROR && strstr(m_pBuff, "200 OK")) {
		pData = GetHttpItamData(m_pBuff);
		auto pDataLen = GetHttpItamData(m_pBuff, "Content-Length");
		if (pData && pDataLen) {
			int nDataRailLen = atoi(pDataLen);
			int nDataRcvLen = nDataLen - (pData - m_pBuff);
			while (nDataRcvLen < nDataRailLen && nDataLen < BUFF_LEN) {
				int nLen = recv(m_hBing, m_pBuff + nDataLen, BUFF_LEN - nDataLen, 0);
				if (!nLen || nLen == SOCKET_ERROR)
					break;
				nDataRcvLen += nLen;
				nDataLen += nLen;
			}
			bRet = (nDataRcvLen >= nDataRailLen || nDataLen == BUFF_LEN);
		}
	}
	return bRet ? m_pBuff : nullptr;
}

char * CBingTodayImage::GetHttpData(LPSTR pHttp, int nLen, int &nDataLen)
{
	auto pData = GetHttpItamData(pHttp);
	auto pType = GetHttpItamData(pHttp, "Content-Type");
	if(pType && strstr(pType, "charset=utf-8")){
		int nSrcLen = nLen - (pData - pHttp);
		int nDstLen = BUFF_LEN - nLen;
		auto pWide = pHttp + nLen;
		nSrcLen = MultiByteToWideChar(CP_UTF8, 0, pData, nSrcLen, (LPWSTR)pWide, nDstLen / sizeof(WCHAR));
		nDataLen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pWide, nSrcLen, pData, BUFF_LEN - nLen, nullptr, nullptr);
		pData[nDataLen] = 0;
	}else
		nDataLen = nLen - (pData - pHttp);
	return pData;
}
