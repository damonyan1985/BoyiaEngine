#include "BoyiaHttpEngine.h"
#include "SalLog.h"
#include "CharConvertor.h"
#include "UrlParser.h"

//#define FORM_HEADER_VALUE "Content-Type:application/x-www-form-urlencoded"

namespace yanbo {
BoyiaHttpEngine::BoyiaHttpEngine(HttpCallback* callback)
    : m_callback(callback)
    , m_size(0)
    , m_header((LUint8)0, 1024)
{
}

BoyiaHttpEngine::~BoyiaHttpEngine()
{
}

LVoid BoyiaHttpEngine::setHeader(const NetworkMap& headers)
{
    if (headers.size()) {
        NetworkMap::Iterator iter = headers.begin();
        NetworkMap::Iterator iterEnd = headers.end();

        for (; iter != iterEnd; ++iter) {
            const String& key = (*iter)->getKey();
            const String& value = (*iter)->getValue();
            //String header = key + _CS(":") + value;
            m_header += key;
            m_header += _CS(":");
            m_header += value;
            m_header += _CS("\r\n");
        }
    }
}

LVoid BoyiaHttpEngine::setPostData(const OwnerPtr<String>& data)
{
    m_data = data;
}

LVoid BoyiaHttpEngine::request(const String& url, LInt method)
{
    BOYIA_LOG("BoyiaHttpEngine---request url: %s callback %d", GET_STR(url), (LIntPtr)m_callback);
    if (!m_callback) {
        return;
    }

    // 转换成宽字符串Url
    wstring wurl = CharConvertor::CharToWchar(GET_STR(url));
    Uri uri;
    UrlParser::parse(wurl, uri);

    // 打开wininet
    HINTERNET internet = ::InternetOpen(L"WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    BOYIA_LOG("BoyiaHttpEngine---request Open Internet %d", 1);

    // 连接网络地址
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

    wstring pathUrl = uri.path;
    if (uri.query.length() > 0) {
        pathUrl += L"?" + uri.query;
    }

    // 创建发送请求
    HINTERNET request = HttpOpenRequest(connect, method == NetworkBase::GET ? L"GET" : L"POST", pathUrl.c_str(), NULL,
	    NULL, NULL,
	    dwOpenRequestFlags, dwConnectContext);

    // 初始化请求同
    if (m_header.GetLength() > 0) {
        wstring header = CharConvertor::CharToWchar(GET_STR(m_header));
        BOOL res = HttpAddRequestHeaders(request, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_COALESCE);
    
        if (!res) {
            m_callback->onLoadError(NetworkClient::kNetworkFileError);
            InternetCloseHandle(request);
            InternetCloseHandle(connect);
            InternetCloseHandle(internet);
            return;
        }
    }

    // 发送请求
    DWORD dwError = 0;
    LByte* dataBuffer = m_data.get() ? m_data->GetBuffer() : NULL;
    LInt dataSize = m_data.get() ? m_data->GetLength() : 0;
    if (!HttpSendRequest(request, NULL, 0, dataBuffer, dataSize)) {
        dwError = GetLastError();
        BOYIA_LOG("HttpSendRequest error: ", dwError);
        m_callback->onLoadError(NetworkClient::kNetworkFileError);
        InternetCloseHandle(request);
        InternetCloseHandle(connect);
        InternetCloseHandle(internet);
        return;
    }

    if (dwError == ERROR_INTERNET_INVALID_CA) {
        fprintf(stderr, "HttpSendRequest failed, error: %d (0x%x)/n",
	        dwError, dwError);

        DWORD dwFlags;
        DWORD dwBuffLen = sizeof(dwFlags);
        InternetQueryOption(request, INTERNET_OPTION_SECURITY_FLAGS,
	        (LPVOID)& dwFlags, &dwBuffLen);

        dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
        InternetSetOption(request, INTERNET_OPTION_SECURITY_FLAGS,
	        &dwFlags, sizeof(dwFlags));

        request = HttpOpenRequest(connect, method == NetworkBase::GET ? L"GET" : L"POST", pathUrl.c_str(), NULL,
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
    //pInfoBuffer[dwInfoBufferLength] = 0;
    //pInfoBuffer[dwInfoBufferLength + 1] = 0;
    //printf("%s", pInfoBuffer);
    free(pInfoBuffer);
	// Get Response Body
    DWORD bufferSize = 1 * KB;
    LByte* buffer = new LByte[bufferSize];
    /*
	while (InternetQueryDataAvailable(request, &dwBytesAvailable, 0, 0)) {
		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(request, buffer,
			dwBytesAvailable, &dwBytesRead);
        
		if (!bResult) {
			fprintf(stderr, "InternetReadFile failed, error: %d (0x%x)/n",
				GetLastError(), GetLastError());
			break;
		}
        if (dwBytesRead == 0) {
            break; // End of File.
        }

        m_callback->onDataReceived(buffer, dwBytesRead);
	}*/

    while (true) {
        DWORD readSize;
        if (!InternetReadFile(request, buffer, bufferSize, &readSize)) {
            m_callback->onLoadError(NetworkClient::kNetworkFileError);
            InternetCloseHandle(request);
            InternetCloseHandle(connect);
            InternetCloseHandle(internet);
            return;
        }

        if (readSize == 0) {
            break;
        }

        m_callback->onDataReceived(buffer, readSize);
    }

    delete[] buffer;
    m_callback->onLoadFinished();

    // Close Http resource
    InternetCloseHandle(request);
    InternetCloseHandle(connect);
    InternetCloseHandle(internet);

    BOYIA_LOG("BoyiaHttpEngine---request end %d", 2);
}
}