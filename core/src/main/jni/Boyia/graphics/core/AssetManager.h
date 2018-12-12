#ifndef AssetManager_h
#define AssetManager_h

#include "UtilString.h"
#include <stdio.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace yanbo
{
class AssetManager
{
public:
	static AssetManager* getAssetManager();
	~AssetManager();

    //void readFile(const CString& name, int mode, CString& output);

    bool open(const CString& name, int mode);
    bool readLine(String& name);
    void close();

private:
	AssetManager();
	static AssetManager* s_assetManager;

	FILE* m_file;
	AAsset* m_asset;
};
}
#endif
