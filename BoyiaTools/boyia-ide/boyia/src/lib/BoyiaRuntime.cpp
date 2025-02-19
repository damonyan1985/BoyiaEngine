#include "BoyiaRuntime.h"
#include "BoyiaCore.h"
#include "BoyiaMemory.h"
#include "SalLog.h"
#include "BoyiaAsyncEvent.h"
#include "SystemUtil.h"
#include "StringUtils.h"
#include "FileUtil.h"


const LInt kMemoryPoolSize = (6 * MB);
const LInt kGcMemorySize = (8 * KB);
//const LInt kGcMemorySize = (2 * MB);
const LInt kNativeFunctionCapacity = 100;

extern LVoid* CreateGC(LVoid* vm);
extern LVoid DestroyGC(LVoid* vm);
extern LVoid GCollectGarbage(LVoid* vm);

namespace boyia {

class PlatformApiCallback : public BoyiaAsyncEvent {
public:
    PlatformApiCallback(BoyiaValue* obj, BoyiaValue* cb, BoyiaRuntime* runtime)
        : BoyiaAsyncEvent(obj, runtime)
    {
        if (cb != kBoyiaNull) {
            ValueCopy(&m_cb, cb);
        } else {
            m_cb.mValue.mObj.mPtr = kBoyiaNull;
        }
    }
    
    virtual LVoid callback()
    {
        
    }
    
private:
    BoyiaValue m_cb;
};

class BoyiaCompileInfo {
public:
    BoyiaCompileInfo(BoyiaRuntime* runtime)
        : m_runtime(runtime) {}

    LVoid compileFile(const String& path)
    {
        String currentScriptPath;
        currentScriptPath.DeepAssign(m_currentScriptPath);

        m_currentScriptPath = path;
        if (m_programSet.get(path)) {
            return;
        }
        String source;
        FileUtil::readFile(path, source);
        if (source.GetLength()) {
            compile(source);
            m_programSet.put(path, LTrue);
        }

        m_currentScriptPath.DeepAssign(currentScriptPath);
    }

    LVoid compile(const String& script)
    {
        CompileCode((char*)script.GetBuffer(), m_runtime->vm());
    }

    const String& getCurrentScript() const {
        return m_currentScriptPath;
    }
private:
    HashMap<HashString, LBool> m_programSet;
    String m_currentScriptPath;
    BoyiaRuntime* m_runtime;
};

#define GEN_ID(key) (m_idCreator->genIdentByStr(key, StringUtils::StringSize(key)))

BoyiaRuntime::BoyiaRuntime()
    : m_memoryPool(InitMemoryPool(kMemoryPoolSize))
    , m_idCreator(new util::IDCreator())
    , m_nativeFunTable(0, kNativeFunctionCapacity)
    , m_nativeSize(0)
    , m_vm(InitVM(this))
    , m_gc(CreateGC(m_vm))
    , m_isGcRuning(LFalse)
    , m_eventManager(new BoyiaAsyncEventManager())
    , m_compileInfo(new BoyiaCompileInfo(this))
    , m_isLoadExeFile(LFalse)
{
}

BoyiaRuntime::~BoyiaRuntime()
{
    DestroyGC(m_vm);
    DestroyVM(m_vm);
    FreeMemoryPool(m_memoryPool);
}

LVoid* BoyiaRuntime::memoryPool() const
{
    return m_memoryPool;
}

LVoid* BoyiaRuntime::garbageCollect() const
{
    return m_gc;
}

LVoid BoyiaRuntime::collectGarbage()
{
}

LVoid BoyiaRuntime::setGcRuning(LBool isRuning)
{
    m_isGcRuning = isRuning;
}

// TODO GC目前还有点问题，暂不执行gc
LBool BoyiaRuntime::needCollect() const
{
    // return GetUsedMemory(m_memoryPool) >= kGcMemorySize && !m_isGcRuning;
    //return GetUsedMemory(m_memoryPool) >= kMemoryPoolSize / 2 && !m_isGcRuning;
    //return LTrue;

    return LTrue;
}

// Prepare delete the object
LVoid BoyiaRuntime::prepareDelete(LVoid* ptr)
{
    // Clear all callbacks which bind on object
    m_eventManager->removeAllEvent(reinterpret_cast<LIntPtr>(ptr));
}

LVoid BoyiaRuntime::init()
{
    // if code entry cache exist, use cache to load program
    if (FileUtil::isExist(yanbo::PlatformBridge::getInstructionEntryPath())) {
        LoadVMCode(vm());
        m_isLoadExeFile = LTrue;
    }
    // begin builtins id
    GEN_ID("this");
    GEN_ID("super");
    GEN_ID("String");
    GEN_ID("Array");
    GEN_ID("Map");
    GEN_ID("MicroTask");
    // end builtins id

    initNativeFunction();

    // add builtin classes
    BuiltinStringClass(m_vm);
    BuiltinMapClass(m_vm);
    BuiltinMicroTaskClass(m_vm);
}

LVoid BoyiaRuntime::appendNative(LUintPtr id, NativePtr ptr)
{
    m_nativeFunTable[m_nativeSize++] = { id, ptr };
}

LVoid BoyiaRuntime::initNativeFunction()
{
    appendNative(GEN_ID("new"), CreateObject);
    appendNative(GEN_ID("BY_ReadFile"), getFileContent);
    appendNative(GEN_ID("BY_WriteFile"), writeFileContent);
    // Array Api Begin
    appendNative(GEN_ID("BY_GetFromArray"), getElementFromVector);
    appendNative(GEN_ID("BY_AddInArray"), addElementToVector);
    appendNative(GEN_ID("BY_GetArraySize"), getVectorSize);
    appendNative(GEN_ID("BY_ClearArray"), clearVector);
    appendNative(GEN_ID("BY_RemoveWidthIndex"), removeElementWidthIndex);
    appendNative(GEN_ID("BY_RemoveFromArray"), removeElementFromVector);
    // Array Api End
    appendNative(GEN_ID("BY_Log"), logPrint);
    appendNative(GEN_ID("BY_Json"), jsonParseWithCJSON);
    appendNative(GEN_ID("BY_toJson"), toJsonString);
    appendNative(GEN_ID("BY_CreateDocument"), createBoyiaDocument);
    appendNative(GEN_ID("BY_AppendView"), appendView);
    appendNative(GEN_ID("BY_RemoveView"), removeView);
    appendNative(GEN_ID("BY_GetRootDocument"), getRootDocument);
    appendNative(GEN_ID("BY_SetDocument"), setDocument);
    appendNative(GEN_ID("BY_RemoveDocument"), removeDocument);
    appendNative(GEN_ID("BY_SetXpos"), setViewXpos);
    appendNative(GEN_ID("BY_SetYpos"), setViewYpos);
    appendNative(GEN_ID("BY_DrawView"), drawView);
    appendNative(GEN_ID("BY_GetViewXpos"), getViewXpos);
    appendNative(GEN_ID("BY_GetViewYpos"), getViewYpos);
    appendNative(GEN_ID("BY_GetViewWidth"), getViewWidth);
    appendNative(GEN_ID("BY_GetViewHeight"), getViewHeight);
    appendNative(GEN_ID("BY_SetViewStyle"), setViewStyle);
    appendNative(GEN_ID("BY_LoadData"), loadDataFromNative);
    appendNative(GEN_ID("BY_StartScale"), startScale);
    appendNative(GEN_ID("BY_StartOpacity"), startOpacity);
    appendNative(GEN_ID("BY_CallStaticMethod"), callStaticMethod);
    appendNative(GEN_ID("BY_StartTranslate"), startTranslate);
    appendNative(GEN_ID("BY_GetHtmlItem"), getHtmlItem);
    appendNative(GEN_ID("BY_LoadImage"), loadImageByUrl);
    appendNative(GEN_ID("BY_SetViewText"), setViewGroupText);
    appendNative(GEN_ID("BY_SetInputViewText"), setInputViewText);
    appendNative(GEN_ID("BY_AddEventListener"), addEventListener);
    appendNative(GEN_ID("BY_SetToNativeView"), setToNativeView);
    appendNative(GEN_ID("BY_InstanceOfClass"), instanceOfClass);
    appendNative(GEN_ID("BY_CreateViewGroup"), createViewGroup);
    appendNative(GEN_ID("BY_SetImageUrl"), setImageUrl);
    appendNative(GEN_ID("BY_ViewCommit"), viewCommit);
    appendNative(GEN_ID("BY_SetViewVisible"), setViewVisible);
    appendNative(GEN_ID("BY_GetPlatformType"), getPlatformType);
    appendNative(GEN_ID("BY_CreateSocket"), createSocket);
    appendNative(GEN_ID("BY_SendSocketMsg"), sendSocketMsg);
    appendNative(GEN_ID("BY_Require"), requireFile);
    // End
    appendNative(0, kBoyiaNull);
}

LBool BoyiaRuntime::isLoadExeFile() const
{
    return m_isLoadExeFile;
}

LVoid BoyiaRuntime::runExeFile()
{
    ExecuteGlobalCode(vm());
}

LVoid BoyiaRuntime::compile(const String& script)
{
    m_compileInfo->compile(script);
}

LVoid BoyiaRuntime::compileFile(const String& path)
{
    m_compileInfo->compileFile(path);
}

const String& BoyiaRuntime::getCurrentScript() const {
    return m_compileInfo->getCurrentScript();
}

LInt BoyiaRuntime::findNativeFunc(LUintPtr key) const
{
    LInt idx = -1;
    while (m_nativeFunTable[++idx].mAddr) {
        if (m_nativeFunTable[idx].mNameKey == key)
            return idx;
    }

    return -1;
}

LInt BoyiaRuntime::callNativeFunction(LInt idx) const
{
    return (*m_nativeFunTable[idx].mAddr)(m_vm);
}

LVoid* BoyiaRuntime::vm() const
{
    return m_vm;
}

util::IDCreator* BoyiaRuntime::idCreator() const
{
    return m_idCreator;
}

BoyiaAsyncEventManager* BoyiaRuntime::eventManager() const
{
    return m_eventManager;
}

LVoid* BoyiaRuntime::createMemoryBackup()
{
    return InitMemoryPool(kMemoryPoolSize);
}

LVoid BoyiaRuntime::changeMemoryPool(LVoid* pool)
{
    FreeMemoryPool(pool);
    m_memoryPool = pool;
}

LVoid BoyiaRuntime::callPlatformApi(const String& params, BoyiaValue* propCB)
{
}

LVoid BoyiaRuntime::consumeMicroTask()
{
    ConsumeMicroTask(m_vm);
}
}
