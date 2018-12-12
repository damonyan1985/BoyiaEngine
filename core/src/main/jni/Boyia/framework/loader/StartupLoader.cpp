#include "StartupLoader.h"
#include "FileUtil.h"
#include "JNIUtil.h"
#include "AutoObject.h"
#include "UIViewThread.h"
#include "JSONParser.h"
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define MAX_APPS_SIZE 20
#define APP_LOAD_URL "https://raw.githubusercontent.com/damonyan1985/BoyiaApp/master/boyia.json"
#define APPS_JSON_PATH "/data/data/com.boyia.app/files/apps/boyia.json"
#define APPS_PATH "/data/data/com.boyia.app/files/apps/"
#define APP_JSON "/app.json"

namespace yanbo
{
static LVoid uiViewThreadLoad(const char* entry)
{
	String url = _CS(entry);
	UIViewThread::instance()->load(url);
	url.ReleaseBuffer();
}

class AppInfo
{
public:
    AppInfo()
        : isEntry(LFalse)
        , versionCode(0)
	{
	}

	LVoid parseApps(cJSON* appsJson)
	{
		cJSON* appJson = appsJson->child;
		while (appJson)
		{
			cJSON* item = appJson->child;
			while (item)
			{
	            if (0 == strcmp(item->string, "versionCode"))
	            {
	            	versionCode = item->valueint;
	            	KFORMATLOG("boyia app version=%d", item->valueint);
	            }
	            else if (0 == strcmp(item->string, "path"))
	            {
	            	path = _CS(item->valuestring);
	            	KFORMATLOG("boyia app path=%s", item->valuestring);
	            }
	            else if (0 == strcmp(item->string, "url"))
	            {
	            	url = _CS(item->valuestring);
	            	KFORMATLOG("boyia app url=%s", item->valuestring);
	            }
	            else if (0 == strcmp(item->string, "name"))
				{
	            	name = _CS(item->valuestring);
	            	KFORMATLOG("boyia app name=%s", item->valuestring);
				}
	            else if (0 == strcmp(item->string, "isEntry"))
				{
	            	isEntry = item->type == cJSON_True ? LTrue : LFalse;
	            	KFORMATLOG("boyia app isEntry=%d", item->type);
				}

	            item = item->next;
			}

			appJson = appJson->next;
		}
	}

public:
	LInt versionCode;// app version code
    String path; // download in local path
    String name; // app name
    String url;  // download url
    LBool isEntry;
};

class AppHandler : public NetworkClient
{
public:
	AppHandler(const String& name)
	{
		m_appDir = _CS(APPS_PATH) + name;
		m_appFilePath = m_appDir + _CS("_tmp.zip");

		KFORMATLOG("boyia app AppHandler name=%s", GET_STR(m_appFilePath));
		m_appFile = fopen(GET_STR(m_appFilePath), "wb+");
	}

	virtual void onDataReceived(const LByte* data, LInt size)
	{
		fwrite(data, size, 1, m_appFile);
		//fflush(m_appFile);
	}

	virtual void onStatusCode(LInt statusCode)
	{
	}

	virtual void onRedirectUrl(const String& redirectUrl)
	{
	}

	virtual void onLoadError(LInt error)
	{
		fclose(m_appFile);
		delete this;
	}

	virtual void onLoadFinished(const String& data)
	{
		fclose(m_appFile);
		JNIUtil::unzip(m_appFilePath, m_appDir);
		FileUtil::printAllFiles("/data/data/com.boyia.app/files/");

		String appJsonPath = m_appDir + _CS(APP_JSON);
		boyia::JSONParser parser(appJsonPath);

		cJSON* entry = parser.get("entry");
		if (entry)
		{
			KFORMATLOG("boyia app AppHandler entry=%s", entry->valuestring);
			uiViewThreadLoad(entry->valuestring);
		}

		delete this;
	}

	virtual void onFileLen(LInt len)
	{
		KFORMATLOG("boyia app AppHandler size=%d", len);
	}

private:
	FILE* m_appFile;
	String m_appDir;
	String m_appFilePath;
};

StartupLoader::StartupLoader()
    : m_appInfos(0, MAX_APPS_SIZE)
{
}

LVoid StartupLoader::startLoad()
{
	if (!FileUtil::isExist(APPS_PATH))
	{
		mkdir(APPS_PATH, S_IRWXU);
	}

    m_file = fopen(APPS_JSON_PATH, "wb+");

    String url(_CS(APP_LOAD_URL), LFalse, LStrlen((LUint8*)APP_LOAD_URL));
    m_loader.loadUrl(url, this);
    KFORMATLOG("boyia app StartupLoader m_file=%d", (LInt)m_file);
}

LVoid StartupLoader::onDataReceived(const LByte* data, LInt size)
{
	if (m_file)
	{
		fwrite(data, size, 1, m_file);
	}

}

LVoid StartupLoader::onStatusCode(LInt statusCode)
{
}

LVoid StartupLoader::onFileLen(LInt len)
{

}

LVoid StartupLoader::onRedirectUrl(const String& redirectUrl)
{

}

LVoid StartupLoader::onLoadError(LInt error)
{
	if (m_file)
	{
		fclose(m_file);
	}
}

LVoid StartupLoader::onLoadFinished(const String& data)
{
	if (!m_file)
	{
		return;
	}

	fclose(m_file);
	parseConfig();
	startLoadApp();
}

LVoid StartupLoader::parseConfig()
{
	String content;

	String path(_CS(APPS_JSON_PATH), LFalse, LStrlen((LUint8*)APPS_JSON_PATH));
	util::FileUtil::readFile(path, content);
	KFORMATLOG("boyia app content=%s", GET_STR(content));

	cJSON* json = cJSON_Parse(GET_STR(content));
	if (json->type == cJSON_Object)
	{
		cJSON* child = json->child;
		while (child)
		{
			if (child->type == cJSON_Array
					&& 0 == strcmp(child->string, "apps"))
			{
				AppInfo* appInfo = new AppInfo;
				appInfo->parseApps(child);
				m_appInfos.addElement(appInfo);
			}
			else if (child->type == cJSON_String
					&& 0 == strcmp(child->string, "retCode"))
			{
			}

			child = child->next;
		}
	}

	cJSON_Delete(json);
}

LVoid StartupLoader::startLoadApp()
{
	LInt size = m_appInfos.size();
	for (LInt id = 0; id < size; ++id)
	{
		String appDir = _CS(APPS_PATH) + m_appInfos[id]->name;
		String appJsonPath = appDir + _CS(APP_JSON);
		LInt versionCode = 0;
		if (FileUtil::isExist(GET_STR(appDir)))
		{
			// Get App Json Info
			KFORMATLOG("boyia app content=%s", GET_STR(appJsonPath));
			boyia::JSONParser parser(appJsonPath);
			// Get App Version
			cJSON* version = parser.get("versionCode");
			if (version)
			{
				versionCode = version->valueint;
			}
		}

		// If the versionCode boyia.json greater than the version
		// which in local app.json
		if (m_appInfos[id]->versionCode > versionCode)
		{
			if (FileUtil::isExist(GET_STR(appDir)))
			{
				FileUtil::deleteFile(GET_STR(appDir));
			}

			m_loader.loadUrl(m_appInfos[id]->url, new AppHandler(m_appInfos[id]->name));
		}
		else
		{
			boyia::JSONParser parser(appJsonPath);
			cJSON* entry = parser.get("entry");
			if (entry)
			{
				uiViewThreadLoad(entry->valuestring);
			}
		}
	}
}
}
