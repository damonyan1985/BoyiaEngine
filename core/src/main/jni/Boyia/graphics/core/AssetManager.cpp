#include "AssetManager.h"
#include <jni.h>
#include <string.h>

static AAssetManager* gAAssetManager = NULL;
void nativeGetAssetManager(JNIEnv* env, jobject obj,jobject assetManager)
{
	gAAssetManager = AAssetManager_fromJava(env, assetManager);
}

#define BUFF_LEN 256

namespace yanbo
{
AssetManager* AssetManager::s_assetManager = NULL;

AssetManager* AssetManager::getAssetManager()
{
    if(s_assetManager == NULL)
    	s_assetManager = new AssetManager();

    return s_assetManager;
}

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

//void AssetManager::readFile(const CString& name, int mode, CString& output)
//{
//	if(gAAssetManager == NULL)
//		return;
//    do{
//        AAsset * pAsset = AAssetManager_open(gAAssetManager, name.GetBuffer(), AASSET_MODE_UNKNOWN);
//        if( pAsset == NULL ) break;
//        size_t size = AAsset_getLength(pAsset);
//        if( size > 0 )
//        {
//            char* pData = new char[size];
//            int iRet = AAsset_read( pAsset, pData, size);
//            if( iRet <= 0 )
//            {
//                delete [] pData;
//                pData = NULL;
//            }
//
//            output = pData;
//        }
//        AAsset_close(pAsset);
//    } while(0);
//}

bool AssetManager::open(const CString& name, int mode)
{
	if(gAAssetManager == NULL)
		return false;

	off_t start, length;
	m_asset = AAssetManager_open(gAAssetManager, name, AASSET_MODE_UNKNOWN);
	if(m_asset == NULL)
		return false;
	int fd = AAsset_openFileDescriptor(m_asset,&start,&length);
	if(fd < 0)
		return false;

	m_file = fdopen(fd,"rb");
	return true;
}

bool AssetManager::readLine(String& name)
{
	if(m_file == NULL)
		return false;

    char* str = new char[BUFF_LEN];
    LMemset(str, 0, BUFF_LEN);
    bool result = fgets(str, BUFF_LEN, m_file) != NULL ? true : false;
    if(!result)
    	return false;

    int i = strlen(str);
    str[i] = 0;
    name = (const LUint8*)str;
    return result;
}

void AssetManager::close()
{
	if(m_file != NULL)
	    fclose(m_file);
	m_file = NULL;

	if(m_asset != NULL)
		AAsset_close(m_asset);
	m_asset = NULL;
}

}
