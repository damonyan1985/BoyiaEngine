#include "BoyiaHttpEngine.h"
#include "SalLog.h"
#include "CharConvertor.h"
#include "UrlParser.h"

#define FORM_HEADER_VALUE "Content-Type:application/x-www-form-urlencoded"

namespace yanbo {
BoyiaHttpEngine::BoyiaHttpEngine(HttpCallback* callback)
    : m_callback(callback)
    , m_size(0)
{
}

BoyiaHttpEngine::~BoyiaHttpEngine()
{
}

size_t BoyiaHttpEngine::writeCallback(LVoid* buffer, size_t size, size_t membyte, LVoid* param)
{
    return 0;
}
/*
LVoid BoyiaHttpEngine::setHeader(const NetworkMap& headers)
{
}

LVoid BoyiaHttpEngine::setPostData(const OwnerPtr<String>& data)
{
    m_data = data;
}
*/
LVoid BoyiaHttpEngine::request(const char* url, LInt method)
{
	wstring wurl = CharConvertor::CharToWchar(url);
	Uri uri;
	UrlParser::parse(wurl, uri);
	HINTERNET internet = ::InternetOpen(L"WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	
	DWORD dwConnectContext = 0;
	HINTERNET connect = ::InternetConnect(internet, uri.host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, dwConnectContext);

	DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
		INTERNET_FLAG_KEEP_CONNECTION |
		INTERNET_FLAG_NO_AUTH |
		INTERNET_FLAG_NO_COOKIES |
		INTERNET_FLAG_NO_UI |
		// Set https
		INTERNET_FLAG_SECURE |
		INTERNET_FLAG_RELOAD;

	HINTERNET request = HttpOpenRequest(connect, method == NetworkBase::GET ? L"GET" : L"POST", uri.path.c_str(), NULL,
		NULL, NULL,
		dwOpenRequestFlags, dwConnectContext);

	DWORD dwError = 0;
	if (!HttpSendRequest(request, NULL, 0, NULL, 0)) {
		dwError = GetLastError();
		BOYIA_LOG("HttpSendRequest error: ", dwError);
	}

	if (dwError == ERROR_INTERNET_INVALID_CA)
	{
		fprintf(stderr, "HttpSendRequest failed, error: %d (0x%x)/n",
			dwError, dwError);

		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);
		InternetQueryOption(request, INTERNET_OPTION_SECURITY_FLAGS,
			(LPVOID)& dwFlags, &dwBuffLen);

		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		InternetSetOption(request, INTERNET_OPTION_SECURITY_FLAGS,
			&dwFlags, sizeof(dwFlags));

		request = HttpOpenRequest(connect, method == NetworkBase::GET ? L"GET" : L"POST", uri.path.c_str(), NULL,
			NULL, NULL,
			dwOpenRequestFlags, dwConnectContext);
	}

	// Get HTTP Response Header
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = 2048;
	BYTE* pInfoBuffer = (BYTE*)malloc(dwInfoBufferLength + 2);
	while (!HttpQueryInfo(request, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL)) {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER) {
			free(pInfoBuffer);
			pInfoBuffer = (BYTE*)malloc(dwInfoBufferLength + 2);
		} else {
			fprintf(stderr, "HttpQueryInfo failed, error: %d (0x%x)/n",
				GetLastError(), GetLastError());
			break;
		}
	}
	pInfoBuffer[dwInfoBufferLength] = '/0';
	pInfoBuffer[dwInfoBufferLength + 1] = '/0';
	printf("%s", pInfoBuffer);
	free(pInfoBuffer);
	// Get Response Body
	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(request, &dwBytesAvailable, 0, 0)) {
		LByte* buffer = new LByte[dwBytesAvailable + 1];
		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(request, buffer,
			dwBytesAvailable, &dwBytesRead);
		if (!bResult) {
			fprintf(stderr, "InternetReadFile failed, error: %d (0x%x)/n",
				GetLastError(), GetLastError());
			break;
		}
		if (dwBytesRead == 0)
			break; // End of File.
		if (m_callback) {
			m_callback->onDataReceived(buffer, dwBytesRead);
		}
		buffer[dwBytesRead] = '/0';
		printf("%s", buffer);

		delete[] buffer;
	}


	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
}
}