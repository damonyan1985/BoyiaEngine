#include "BoyiaRuntime.h"
#include "AppManager.h"
#include "Application.h"
#include "BoyiaCore.h"
#include "BoyiaMemory.h"
#include "SalLog.h"
#include "BoyiaAsyncEvent.h"

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

template<int n>
constexpr int StringSize(const char(&s)[n])
{
    return n-1;
}

//static LVoid GenId(util::IDCreator* creator, const LInt8* idName)
//{
//    creator->genIdentByStr(idName, StringSize(idName));
//}

LVoid BoyiaRuntime::init()
{
    
    //int n = StringSize("super");
    // begin builtins id
    m_idCreator->genIdentByStr("this", StringSize("this"));
    //GenId(m_idCreator, "this");
    m_idCreator->genIdentByStr("super", StringSize("super"));
    m_idCreator->genIdentByStr("String", StringSize("String"));
    m_idCreator->genIdentByStr("Array", StringSize("Array"));
    m_idCreator->genIdentByStr("Map", StringSize("Map"));
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
    appendNative(m_idCreator->genIdentByStr("new", StringSize("new")), CreateObject);
    appendNative(m_idCreator->genIdentByStr("BY_Content", StringSize("BY_Content")), getFileContent);
    // Array Api Begin
    appendNative(m_idCreator->genIdentByStr("BY_GetFromArray", StringSize("BY_GetFromArray")), getElementFromVector);
    appendNative(m_idCreator->genIdentByStr("BY_AddInArray", StringSize("BY_AddInArray")), addElementToVector);
    appendNative(m_idCreator->genIdentByStr("BY_GetArraySize", StringSize("BY_GetArraySize")), getVectorSize);
    appendNative(m_idCreator->genIdentByStr("BY_ClearArray", StringSize("BY_ClearArray")), clearVector);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveWidthIndex", StringSize("BY_RemoveWidthIndex")), removeElementWidthIndex);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveFromArray", StringSize("BY_RemoveFromArray")), removeElementFromVector);
    // Array Api End
    appendNative(m_idCreator->genIdentByStr("BY_Log", StringSize("BY_Log")), logPrint);
    appendNative(m_idCreator->genIdentByStr("BY_Json", StringSize("BY_Json")), jsonParseWithCJSON);
    appendNative(m_idCreator->genIdentByStr("BY_toJson", StringSize("BY_toJson")), toJsonString);
    appendNative(m_idCreator->genIdentByStr("BY_CreateDocument", StringSize("BY_CreateDocument")), createJSDocument);
    appendNative(m_idCreator->genIdentByStr("BY_AppendView", StringSize("BY_AppendView")), appendView);
    appendNative(m_idCreator->genIdentByStr("BY_GetRootDocument", StringSize("BY_GetRootDocument")), getRootDocument);
    appendNative(m_idCreator->genIdentByStr("BY_SetDocument", StringSize("BY_SetDocument")), setDocument);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveDocument", StringSize("BY_RemoveDocument")), removeDocument);
    appendNative(m_idCreator->genIdentByStr("BY_SetXpos", StringSize("BY_SetXpos")), setViewXpos);
    appendNative(m_idCreator->genIdentByStr("BY_SetYpos", StringSize("BY_SetYpos")), setViewYpos);
    appendNative(m_idCreator->genIdentByStr("BY_DrawView", StringSize("BY_DrawView")), drawView);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewXpos", StringSize("BY_GetViewXpos")), getViewXpos);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewYpos", StringSize("BY_GetViewYpos")), getViewYpos);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewWidth", StringSize("BY_GetViewWidth")), getViewWidth);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewHeight", StringSize("BY_GetViewHeight")), getViewHeight);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewStyle", StringSize("BY_SetViewStyle")), setViewStyle);
    appendNative(m_idCreator->genIdentByStr("BY_LoadData", StringSize("BY_LoadData")), loadDataFromNative);
    appendNative(m_idCreator->genIdentByStr("BY_StartScale", StringSize("BY_StartScale")), startScale);
    appendNative(m_idCreator->genIdentByStr("BY_StartOpacity", StringSize("BY_StartOpacity")), startOpacity);
    appendNative(m_idCreator->genIdentByStr("BY_CallStaticMethod", StringSize("BY_CallStaticMethod")), callStaticMethod);
    appendNative(m_idCreator->genIdentByStr("BY_StartTranslate", StringSize("BY_StartTranslate")), startTranslate);
    appendNative(m_idCreator->genIdentByStr("BY_GetHtmlItem", StringSize("BY_GetHtmlItem")), getHtmlItem);
    appendNative(m_idCreator->genIdentByStr("BY_LoadImage", StringSize("BY_LoadImage")), loadImageByUrl);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewText", StringSize("BY_SetViewText")), setViewGroupText);
    appendNative(m_idCreator->genIdentByStr("BY_SetInputViewText", StringSize("BY_SetInputViewText")), setInputViewText);
    appendNative(m_idCreator->genIdentByStr("BY_AddEventListener", StringSize("BY_AddEventListener")), addEventListener);
    appendNative(m_idCreator->genIdentByStr("BY_SetToNativeView", StringSize("BY_SetToNativeView")), setToNativeView);
    appendNative(m_idCreator->genIdentByStr("BY_InstanceOfClass", StringSize("BY_InstanceOfClass")), instanceOfClass);
    appendNative(m_idCreator->genIdentByStr("BY_CreateViewGroup", StringSize("BY_CreateViewGroup")), createViewGroup);
    appendNative(m_idCreator->genIdentByStr("BY_SetImageUrl", StringSize("BY_SetImageUrl")), setImageUrl);
    appendNative(m_idCreator->genIdentByStr("BY_ViewCommit", StringSize("BY_ViewCommit")), viewCommit);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewVisible", StringSize("BY_SetViewVisible")), setViewVisible);
    appendNative(m_idCreator->genIdentByStr("BY_GetPlatformType", StringSize("BY_GetPlatformType")), getPlatformType);
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
