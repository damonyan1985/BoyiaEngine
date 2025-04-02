#include "PlatformBridge.h"
#include "SalLog.h"
#include "ZipEntry.h"

#if ENABLE(BOYIA_IOS)
#include "AppManager.h"
#include "FileUtil.h"
#include "IOSRenderer.h"
#include "BoyiaBridge.h"

@interface BoyiaHandlerCallbackImpl : NSObject<BoyiaApiHandlerCB>
-(instancetype)initWithCallbackId:(long)callbackId;
-(long)callbackId;
-(void)callback:(NSString*)json;
@end

@implementation BoyiaHandlerCallbackImpl {
    int _callbackId;
}

-(long)callbackId {
    return _callbackId;
}

-(void)callback:(NSString*)json {
    
}

-(instancetype)initWithCallbackId:(int)callbackId {
    self = [super init];
    if (self) {
        _callbackId = callbackId;
    }
    
    return self;
}

@end

namespace yanbo {
// TODO
const char* kZipPassword = "123456";

static String sAppPath((LUint8)0, 256);
static String sBoyiaJsonPath((LUint8)0, 256);
static String sSdkPath((LUint8)0, 256);
static String sAppRootPath((LUint8)0, 256);

bool PlatformBridge::unzip(const String& zipFile, const String& dir)
{
    BOYIA_LOG("PlatformBridge---unzip---src: %s, dest: %s", GET_STR(zipFile), GET_STR(dir));

    if (dir.EndWith(_CS("sdk"))) {
        return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kBoyiaNull);
    }

    return ZipEntry::unzip(GET_STR(zipFile), GET_STR(dir), kZipPassword);
}

const char* PlatformBridge::getAppPath()
{
    if (!sAppPath.GetLength()) {
        sAppPath = _CS(getAppRoot());
        sAppPath += _CS("apps/");
    }

    BOYIA_LOG("PlatformBridge---getAppPath: %s", GET_STR(sAppPath));
    return GET_STR(sAppPath);
    //return "/data/data/com.boyia.app/files/apps/";
}

const char* PlatformBridge::getBoyiaJsonPath()
{
    if (!sBoyiaJsonPath.GetLength()) {
        sBoyiaJsonPath = _CS(getAppRoot());
        sBoyiaJsonPath += _CS("boyia.json");
    }

    BOYIA_LOG("PlatformBridge---getBoyiaJsonPath: %s", GET_STR(sBoyiaJsonPath));
    return GET_STR(sBoyiaJsonPath);
    //return "/data/data/com.boyia.app/files/boyia.json";
}

const char* PlatformBridge::getSdkPath()
{
    if (!sSdkPath.GetLength()) {
        sSdkPath = _CS(getAppRoot());
        sSdkPath += _CS("sdk/");
    }

    BOYIA_LOG("PlatformBridge---getSdkPath: %s", GET_STR(sSdkPath));
    return GET_STR(sSdkPath);
    //return "/data/data/com.boyia.app/files/sdk/";
}

const char* PlatformBridge::getAppRoot()
{
    if (sAppRootPath.GetLength()) {
        return GET_STR(sAppRootPath);
    }
    
//    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
//    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
//    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
//    //NSString* appDir = [NSBundle bundleWithIdentifier:@"com.boyia.metal"].bundlePath;
////    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
////    NSString* docDir = [paths objectAtIndex:0];
////    NSString* appDir = [docDir stringByAppendingString:@"/boyia/"];
////    NSFileManager* fileManager = [NSFileManager defaultManager];
////    if (![fileManager fileExistsAtPath:appDir]) {
////        [fileManager createDirectoryAtPath:appDir withIntermediateDirectories:YES attributes:nil error:nil];
////    }
//    NSString* boyiaAppDir = [appBundle.bundlePath stringByAppendingString:@"/"];
    
    NSString* boyiaAppDir = [BoyiaBridge getAppRoot];
    sAppRootPath = (const LUint8*)([boyiaAppDir UTF8String]);
    return GET_STR(sAppRootPath);
}

const char* PlatformBridge::getBoyiaJsonUrl()
{
    return "boyia://net_boyia.json";
    //return "https://damonyan1985.github.io/app/boyia.json";
}

const LInt PlatformBridge::getTextSize(const String& text)
{
    NSString* str = STR_TO_OCSTR(text);
    return (LInt)str.length;
}

const LReal PlatformBridge::getDisplayDensity()
{
    // TODO
    return 0;
}

PlatformBridge::PlatformType PlatformBridge::getPlatformType()
{
    return kPlatformIos;
}

void PlatformBridge::handleApi(const String& params, LIntPtr callback)
{
    [BoyiaBridge handleApi:STR_TO_OCSTR(params)
                  callback:[[BoyiaHandlerCallbackImpl alloc]initWithCallbackId:callback]];
}
}

#endif
