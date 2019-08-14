#ifndef FileLoader_h
#define FileLoader_h

#include "NetworkBase.h"
#include "UtilString.h"

namespace yanbo {
class FileLoader {
public:
    FileLoader();
    ~FileLoader();

    LVoid load(const String& uri, NetworkClient* client);
};
}

#endif