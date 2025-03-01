#include "BoyiaHttpEngine.h"
#include "SalLog.h"
#include "CharConvertor.h"
#include "UrlParser.h"
#include "KVector.h"
#include <windows.h>
#include <WinInet.h>

//#define FORM_HEADER_VALUE "Content-Type:application/x-www-form-urlencoded"
namespace yanbo {
inline LVoid CloseHandles(HINTERNET request, HINTERNET connect, HINTERNET internet)
{
    InternetCloseHandle(request);
    InternetCloseHandle(connect);
    InternetCloseHandle(internet);
}

inline LBool SendRequest(
    HINTERNET request,
    HINTERNET connect,
    HINTERNET internet,
    const String& url, 
    LByte* buffer, 
    LInt size, 
    HttpCallback* callback)
{
    DWORD dwError = 0;
    BOOL result = HttpSendRequest(request, NULL, 0, buffer, size);
    if (result) {
        return LTrue;
    }

    dwError = GetLastError();
    BOYIA_LOG("HttpSendRequest error: %d %s", dwError, GET_STR(url));
    if (dwError != ERROR_INTERNET_INVALID_CA) {
        callback->onLoadError(NetworkClient::kNetworkFileError);
        CloseHandles(request, connect, internet);
        return LFalse;
    }

    fprintf(stderr, "HttpSendRequest failed, error: %d (0x%x)/n",
        dwError, dwError);

    // 忽略证书
    DWORD dwFlags;
    DWORD dwBuffLen = sizeof(dwFlags);
    InternetQueryOption(request, INTERNET_OPTION_SECURITY_FLAGS,
        (LPVOID)&dwFlags, &dwBuffLen);

    dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    InternetSetOption(request, INTERNET_OPTION_SECURITY_FLAGS,
        &dwFlags, sizeof(dwFlags));

    if (!HttpSendRequest(request, NULL, 0, buffer, size)) {
        dwError = GetLastError();
        fprintf(stderr, "HttpSendRequest ignore ssl failed, error: %d (0x%x)/n",
            dwError, dwError);
        callback->onLoadError(NetworkClient::kNetworkFileError);
        CloseHandles(request, connect, internet);
        return LFalse;
    }
    
    return LTrue;
}

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
    BOYIA_LOG("BoyiaHttpEngine---request url: %s callback %ld", GET_STR(url), (LIntPtr)m_callback);
    if (!m_callback) {
        return;
    }

    // 转换成宽字符串Url
    wstring wurl = CharConvertor::CharToWchar(GET_STR(url));
    //Uri uri;
    //UrlParser::parse(wurl, uri);

    // 打开wininet
    HINTERNET internet = ::InternetOpen(L"WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    BOYIA_LOG("BoyiaHttpEngine---request Open Internet %d", 1);

    // 获取网络地址相关参数
    WCHAR hostName[INTERNET_MAX_HOST_NAME_LENGTH] = { 0 };
    WCHAR userName[INTERNET_MAX_USER_NAME_LENGTH] = { 0 };
    WCHAR password[INTERNET_MAX_PASSWORD_LENGTH] = { 0 };
    WCHAR urlPath[INTERNET_MAX_URL_LENGTH] = { 0 };
    WCHAR schema[INTERNET_MAX_SCHEME_LENGTH] = { 0 };

    URL_COMPONENTS urlComponents;
    ZeroMemory(&urlComponents, sizeof(urlComponents));

    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
    urlComponents.lpszHostName = hostName;
    urlComponents.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
    urlComponents.lpszUserName = userName;
    urlComponents.dwUserNameLength = INTERNET_MAX_USER_NAME_LENGTH;
    urlComponents.lpszPassword = password;
    urlComponents.dwPasswordLength = INTERNET_MAX_PASSWORD_LENGTH;
    urlComponents.lpszUrlPath = urlPath;
    urlComponents.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
    urlComponents.lpszScheme = schema;
    urlComponents.dwSchemeLength = INTERNET_MAX_SCHEME_LENGTH;

    BOOL result = InternetCrackUrl(wurl.data(), 0, NULL, &urlComponents);
    if (!result) {
        InternetCloseHandle(internet);
        m_callback->onLoadError(NetworkClient::kNetworkFileError);
        return;
    }

    // 连接网络地址 INTERNET_DEFAULT_HTTPS_PORT
    DWORD dwConnectContext = 0;
    HINTERNET connect = ::InternetConnect(internet, 
        urlComponents.lpszHostName, 
        urlComponents.nPort, 
        urlComponents.lpszUserName, 
        urlComponents.lpszPassword, 
        INTERNET_SERVICE_HTTP, 0, dwConnectContext);

    DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
	    INTERNET_FLAG_KEEP_CONNECTION |
	    INTERNET_FLAG_NO_AUTH |
	    INTERNET_FLAG_NO_COOKIES |
	    INTERNET_FLAG_NO_UI |
	    // Set https
	    INTERNET_FLAG_SECURE |
        INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
	    INTERNET_FLAG_RELOAD;

    /*
    wstring pathUrl = uri.path;
    if (uri.query.length() > 0) {
        pathUrl += L"?" + uri.query;
    }*/

    // 创建发送请求
    HINTERNET request = HttpOpenRequest(connect, method == NetworkBase::GET ? L"GET" : L"POST", urlComponents.lpszUrlPath, NULL,
	    NULL, NULL,
	    dwOpenRequestFlags, dwConnectContext);

    // 初始化请求同
    if (m_header.GetLength() > 0) {
        wstring header = CharConvertor::CharToWchar(GET_STR(m_header));
        BOOL result = HttpAddRequestHeaders(request, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_COALESCE);
        
        if (!result) {
            m_callback->onLoadError(NetworkClient::kNetworkFileError);
            CloseHandles(request, connect, internet);
            return;
        }
    }

    // 发送请求
    LByte* dataBuffer = m_data.get() ? m_data->GetBuffer() : NULL;
    LInt dataSize = m_data.get() ? m_data->GetLength() : 0;

    if (!SendRequest(request, connect, internet, url, dataBuffer, dataSize, m_callback)) {
        return;
    }

    // Get HTTP Response Header
    DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
    DWORD dwInfoBufferLength = 2 * KB;
    BYTE* pInfoBuffer = (BYTE*)malloc(dwInfoBufferLength + 2);
    while (!HttpQueryInfo(request, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL)) {
        DWORD dwError = GetLastError();
        //  如果缓冲区容量不够，清除缓冲区，同时申请2倍大小的容量
        if (dwError == ERROR_INSUFFICIENT_BUFFER) {
            free(pInfoBuffer);
            dwInfoBufferLength *= 2;
            pInfoBuffer = (BYTE*)malloc(dwInfoBufferLength + 2);
        } else {
            fprintf(stderr, "HttpQueryInfo failed, error: %d (0x%x)/n",
	            GetLastError(), GetLastError());
            break;
        }
    }

    if (pInfoBuffer) {
        pInfoBuffer[dwInfoBufferLength] = 0;
        pInfoBuffer[dwInfoBufferLength + 1] = 0;
        printf("%s", pInfoBuffer);
        free(pInfoBuffer);
    }
	// Get Response Body
    KVector<LByte> buffer(0, 1 * KB);
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
        if (!InternetReadFile(request, buffer.getBuffer(), buffer.capacity(), &readSize)) {
            m_callback->onLoadError(NetworkClient::kNetworkFileError);
            CloseHandles(request, connect, internet);
            return;
        }

        if (readSize == 0) {
            break;
        }

        m_callback->onDataReceived(buffer.getBuffer(), readSize);
    }

    m_callback->onLoadFinished();

    // Close Http resource
    CloseHandles(request, connect, internet);

    BOYIA_LOG("BoyiaHttpEngine---request end %s", GET_STR(url));
}
}