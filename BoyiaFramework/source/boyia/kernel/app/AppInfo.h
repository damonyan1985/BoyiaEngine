#ifndef AppInfo_h
#define AppInfo_h

#include "JSONParser.h"
#include "UtilString.h"

namespace yanbo {
struct AppCodePath {
public:
    AppCodePath();
    mutable String instructPath;
    mutable String entryCodePath;
    mutable String stringTablePath;
    mutable String symbolTablePath;
    mutable String debugInfoPath;
};

class AppInfo {
public:
    AppInfo();
    ~AppInfo();

    LBool isKey(cJSON* item, const char* key);
    LVoid parse();
    LVoid parseApp(cJSON* appJson);

public:
    LInt id;
    LInt versionCode; // app version code
    String path; // download in local path
    String name; // app name
    String url; // download url
    String cover; // app cover
    LBool isEntry;
    AppCodePath appCodePath;
};
}
#endif
