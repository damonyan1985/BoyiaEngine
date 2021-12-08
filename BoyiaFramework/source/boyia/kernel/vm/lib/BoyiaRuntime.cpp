#include "BoyiaRuntime.h"
#include "AppManager.h"
#include "Application.h"
#include "BoyiaCore.h"
#include "BoyiaMemory.h"
#include "SalLog.h"
#include "BoyiaAsyncEvent.h"
#include "SystemUtil.h"
#include "StringUtils.h"

const LInt kMemoryPoolSize = 1024 * 1024 * 6;
const LInt kGcMemorySize = 1024 * 8;
//const LInt kGcMemorySize = 1024 * 1024 * 2;
const LInt kNativeFunctionCapacity = 100;

extern LVoid* CreateGC(LVoid* vm);
extern LVoid GCollectGarbage(LVoid* vm);

namespace boyia {
class GCEvent : public yanbo::UIEvent {
public:
    GCEvent(BoyiaRuntime* runtime)
        : m_runtime(runtime)
    {
    }

    virtual LVoid run()
    {
        GCollectGarbage(m_runtime->vm());
        m_runtime->setGcRuning(LFalse);
    }

private:
    BoyiaRuntime* m_runtime;
};

#define GEN_ID(key) m_idCreator->genIdentByStr(key, StringUtils::StringSize(key))

BoyiaRuntime::BoyiaRuntime(yanbo::Application* app)
    : m_app(app)
    , m_memoryPool(InitMemoryPool(kMemoryPoolSize))
    , m_idCreator(new util::IDCreator())
    , m_nativeFunTable(new NativeFunction[kNativeFunctionCapacity])
    , m_nativeSize(0)
    , m_vm(InitVM(this))
    , m_gc(CreateGC(m_vm))
    , m_isGcRuning(LFalse)
    , m_eventManager(new BoyiaAsyncEventManager())
    , m_domMap(new BoyiaDomMap())
{
}

BoyiaRuntime::~BoyiaRuntime()
{
    FreeMemoryPool(m_memoryPool);
    DestroyVM(m_vm);
    delete[] m_nativeFunTable;
    delete m_idCreator;
    delete m_eventManager;
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
    yanbo::AppManager::instance()->uiThread()->sendUIEvent(new GCEvent(this));
}

LVoid BoyiaRuntime::setGcRuning(LBool isRuning)
{
    m_isGcRuning = isRuning;
}

// TODO GC目前还有点问题，暂不执行gc
LBool BoyiaRuntime::needCollect() const
{
    return GetUsedMemory(m_memoryPool) >= kGcMemorySize && !m_isGcRuning;
    // GetUsedMemory(m_memoryPool) >= kMemoryPoolSize / 2 && !m_isGcRuning;
    //return LFalse;
}

// Prepare delete the object
LVoid BoyiaRuntime::prepareDelete(LVoid* ptr)
{
    // Clear all callbacks which bind on object
    m_eventManager->removeAllEvent(reinterpret_cast<LIntPtr>(ptr));
}

LVoid BoyiaRuntime::init()
{
    // begin builtins id
    GEN_ID("this");
    GEN_ID("super");
    GEN_ID("String");
    GEN_ID("Array");
    GEN_ID("Map");
    // end builtins id

    initNativeFunction();

    // add builtin classes
    BuiltinStringClass(m_vm);
    BuiltinMapClass(m_vm);
}

LVoid BoyiaRuntime::appendNative(LUintPtr id, NativePtr ptr)
{
    m_nativeFunTable[m_nativeSize++] = { id, ptr };
}

LVoid BoyiaRuntime::initNativeFunction()
{
    appendNative(GEN_ID("new"), CreateObject);
    appendNative(GEN_ID("BY_Content"), getFileContent);
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
    appendNative(GEN_ID("BY_CreateDocument"), createJSDocument);
    appendNative(GEN_ID("BY_AppendView"), appendView);
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
    // End
    appendNative(0, kBoyiaNull);
}

LVoid BoyiaRuntime::compile(const String& script)
{
    CompileCode((char*)script.GetBuffer(), m_vm);
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

yanbo::UIView* BoyiaRuntime::view() const
{
    return m_app ? m_app->view() : kBoyiaNull;
}

util::IDCreator* BoyiaRuntime::idCreator() const
{
    return m_idCreator;
}

BoyiaAsyncEventManager* BoyiaRuntime::eventManager() const
{
    return m_eventManager;
}

BoyiaDomMap* BoyiaRuntime::domMap() const
{
    return m_domMap;
}
}
