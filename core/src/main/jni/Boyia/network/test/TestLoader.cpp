#include "BoyiaLoader.h"
#include "SalLog.h"
#include "StringBuffer.h"
#include "FileUtil.h"
#include "StartupLoader.h"
#include <stdio.h>

#define APP_LOAD_URL "https://raw.githubusercontent.com/damonyan1985/BoyiaApp/master/boyia.json"
#define APP_PATH "/data/data/com.boyia.app/files/boyia.json"

namespace yanbo
{
class TestLoader : public NetworkClient
{
public:
	TestLoader()
	{
		m_file = fopen(APP_PATH, "wb+");
		String url(_CS(APP_LOAD_URL), LFalse, LStrlen((LUint8*)APP_LOAD_URL));

		m_loader = new BoyiaLoader();
		m_loader->loadUrl(url, this);
	}

	virtual LVoid onDataReceived(const LByte* data, LInt size)
	{
		//KFORMATLOG("boyia httptext recvlen=%d", size);
		//String buffer(data, size);
		//KFORMATLOG("boyia httptext content=%s", GET_STR(buffer));
		//m_buffer.append(data, 0, size, LFalse);
		//m_buffer.append(buffer);
		//buffer.ReleaseBuffer();
		fwrite(data, size, 1, m_file);
	}

	virtual LVoid onStatusCode(LInt statusCode)
	{
		KFORMATLOG("boyia httptext code=%d", statusCode);
	}

	virtual LVoid onFileLen(LInt len)
	{
		KFORMATLOG("boyia httptext length=%d", len);
	}

	virtual LVoid onRedirectUrl(const String& redirectUrl)
	{

	}

	virtual LVoid onLoadError(LInt error)
	{
		fclose(m_file);
	}

	virtual LVoid onLoadFinished(const String& data)
	{
	    //KRefPtr<String> contentPtr = m_buffer.toString();
	    //String& content = *contentPtr.get();
	    //KFORMATLOG("boyia content=%s", GET_STR(content));
		fclose(m_file);

		String content;

		String path(_CS(APP_PATH), LFalse, LStrlen((LUint8*)APP_PATH));
		util::FileUtil::readFile(path, content);
		KFORMATLOG("boyia app content=%s", GET_STR(content));
	}

private:
    BoyiaLoader* m_loader;
    //StringBuffer m_buffer;

    FILE* m_file;
};

}

extern LVoid TestLoadUrl()
{
	yanbo::StartupLoader* loader = new yanbo::StartupLoader;
	loader->startLoad();
	//new yanbo::TestLoader;
}
