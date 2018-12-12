#include "BoyiaHttpEngine.h"
#include "SalLog.h"
#include <CallStack.h>

namespace yanbo
{
BoyiaHttpEngine::BoyiaHttpEngine(HttpCallback* callback)
    : m_callback(callback)
    , m_size(0)
{
	m_curl = curl_easy_init();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
}

BoyiaHttpEngine::~BoyiaHttpEngine()
{
	if (m_curl)
	{
		curl_easy_cleanup(m_curl);
		m_curl = NULL;
	}
}

size_t BoyiaHttpEngine::writeCallback(LVoid* buffer, size_t size, size_t membyte, LVoid* param)
{
	size_t len  = size * membyte;
	BoyiaHttpEngine* engine = (BoyiaHttpEngine*) param;
	if (engine && engine->m_callback)
	{
	    engine->m_callback->onDataRecevied((const LByte*)buffer, len);
	}

	engine->m_size += len;

	KFORMATLOG("boyia httptext getdatasize=%d", engine->m_size);
	return len;
}

LVoid BoyiaHttpEngine::setHeader(const NetworkMap& headers)
{
	if (headers.size())
	{
		struct curl_slist* headerList = NULL;
		NetworkMap::Iterator iter = headers.begin();
		NetworkMap::Iterator iterEnd = headers.end();
		for(; iter != iterEnd; ++iter)
		{
			const String& key = (*iter)->getKey();
			const String& value = (*iter)->getValue();
			String header = key + _CS(":") + value;

			curl_slist_append(headerList, GET_STR(header));
		}

		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headerList);
	}
}

LVoid BoyiaHttpEngine::request(const String& url, LInt method)
{
//	android::CallStack callstack;
//	callstack.update();
//	callstack.log("BoyiaUI_Stack");
	if (m_curl)
	{
		// 设置请求方式
		curl_easy_setopt(m_curl, method == EHTTP_GET ? CURLOPT_HTTPGET : CURLOPT_POST, 1);
		// 设置请求的url地址
		curl_easy_setopt(m_curl, CURLOPT_URL, GET_STR(url));
		// 设置HTTP头
		curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
		// 设置发送超时时间
		curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 1000);

		curl_easy_setopt(m_curl, CURLOPT_FORBID_REUSE, 1);

		curl_easy_setopt(m_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);

		LInt resultCode = curl_easy_perform(m_curl);

        KFORMATLOG("boyia curl_easy_perform code=%d", resultCode);
		if (m_callback && resultCode == CURLcode::CURLE_OK)
		{
			LInt statusCode = 0;
			curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &statusCode);
			m_callback->onStatusCode(statusCode);

//			LInt contentLength = 0;
//			curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
//
//			m_callback->onContentLength(contentLength);
			if (statusCode == 200)
			{
				m_callback->onLoadFinished();
			}
			else
			{
				m_callback->onLoadError(statusCode);
			}

		}
	}
}
}
