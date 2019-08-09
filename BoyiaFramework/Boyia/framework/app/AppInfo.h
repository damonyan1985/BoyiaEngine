#ifndef AppInfo_h
#define AppInfo_h

#include "JSONParser.h"
#include "UtilString.h"

namespace yanbo {
class AppInfo {
public:
    AppInfo();
    ~AppInfo();

    LVoid parseApps(cJSON* appsJson);

public:
    LInt versionCode; // app version code
    String path; // download in local path
    String name; // app name
    String url; // download url
    LBool isEntry;
};
}
#endif