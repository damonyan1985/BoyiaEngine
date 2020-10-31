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
};

class AppInfo {
public:
    AppInfo();
    ~AppInfo();

    LVoid parseApp(cJSON* appJson);

public:
    LInt versionCode; // app version code
    String path; // download in local path
    String name; // app name
    String url; // download url
    LBool isEntry;
    AppCodePath appCodePath;
};
}
#endif