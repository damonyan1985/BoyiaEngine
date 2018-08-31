#include "Loader.h"
#include "SalLog.h"

class TestLoader : public yanbo::NetworkClient
{
public:
	TestLoader()
	{
		m_loader = new yanbo::Loader();
		m_loader->loadUrl(_CS("https://p99.pstatp.com/list/190x124/pgc-image/153447239909379142c921f"), this);
	}

	virtual LVoid onDataReceived(const String& data)
	{

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

	}

	virtual LVoid onLoadFinished(const String& data)
	{
		String head = data.Mid(0, 30);
        KFORMATLOG("boyia httptext=%s", GET_STR(head));

        LInt pos = data.GetLength() - 50;
		String tail = data.Mid(pos, -1);
        KFORMATLOG("boyia httptext=%s", GET_STR(tail));
	}

private:
	yanbo::Loader* m_loader;
};

extern LVoid TestLoadUrl()
{
	new TestLoader;
}
