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

LVoid BoyiaRuntime::init()
{
    // begin builtins id
    m_idCreator->genIdentByStr("this", 4);
    m_idCreator->genIdentByStr("super", 5);
    m_idCreator->genIdentByStr("String", 6);
    m_idCreator->genIdentByStr("Array", 5);
    // end builtins id

    initNativeFunction();

    // add builtin classes
    BuiltinStringClass(m_vm);
}

LVoid BoyiaRuntime::appendNative(LUintPtr id, NativePtr ptr)
{
    m_nativeFunTable[m_nativeSize++] = { id, ptr };
}

LVoid BoyiaRuntime::initNativeFunction()
{
    appendNative(m_idCreator->genIdentByStr("new", 3), CreateObject);
    appendNative(m_idCreator->genIdentByStr("BY_Content", 10), getFileContent);
    // Array Api Begin
    appendNative(m_idCreator->genIdentByStr("BY_GetFromArray", 15), getElementFromVector);
    appendNative(m_idCreator->genIdentByStr("BY_AddInArray", 13), addElementToVector);
    appendNative(m_idCreator->genIdentByStr("BY_GetArraySize", 15), getVectorSize);
    appendNative(m_idCreator->genIdentByStr("BY_ClearArray", 13), clearVector);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveWidthIndex", 19), removeElementWidthIndex);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveFromArray", 18), removeElementFromVector);
    // Array Api End
    appendNative(m_idCreator->genIdentByStr("BY_Log", 6), logPrint);
    appendNative(m_idCreator->genIdentByStr("BY_Json", 7), jsonParseWithCJSON);
    appendNative(m_idCreator->genIdentByStr("BY_toJson", 9), toJsonString);
    appendNative(m_idCreator->genIdentByStr("BY_CreateDocument", 17), createJSDocument);
    appendNative(m_idCreator->genIdentByStr("BY_AppendView", 13), appendView);
    appendNative(m_idCreator->genIdentByStr("BY_GetRootDocument", 18), getRootDocument);
    appendNative(m_idCreator->genIdentByStr("BY_SetDocument", 14), setDocument);
    appendNative(m_idCreator->genIdentByStr("BY_RemoveDocument", 17), removeDocument);
    appendNative(m_idCreator->genIdentByStr("BY_SetXpos", 10), setViewXpos);
    appendNative(m_idCreator->genIdentByStr("BY_SetYpos", 10), setViewYpos);
    appendNative(m_idCreator->genIdentByStr("BY_DrawView", 11), drawView);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewXpos", 14), getViewXpos);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewYpos", 14), getViewYpos);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewWidth", 15), getViewWidth);
    appendNative(m_idCreator->genIdentByStr("BY_GetViewHeight", 16), getViewHeight);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewStyle", 15), setViewStyle);
    appendNative(m_idCreator->genIdentByStr("BY_LoadData", 11), loadDataFromNative);
    appendNative(m_idCreator->genIdentByStr("BY_StartScale", 13), startScale);
    appendNative(m_idCreator->genIdentByStr("BY_StartOpacity", 15), startOpacity);
    appendNative(m_idCreator->genIdentByStr("BY_CallStaticMethod", 19), callStaticMethod);
    appendNative(m_idCreator->genIdentByStr("BY_StartTranslate", 17), startTranslate);
    appendNative(m_idCreator->genIdentByStr("BY_GetHtmlItem", 14), getHtmlItem);
    appendNative(m_idCreator->genIdentByStr("BY_LoadImage", 12), loadImageByUrl);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewText", 14), setViewGroupText);
    appendNative(m_idCreator->genIdentByStr("BY_SetInputViewText", 19), setInputViewText);
    appendNative(m_idCreator->genIdentByStr("BY_AddEventListener", 19), addEventListener);
    appendNative(m_idCreator->genIdentByStr("BY_SetToNativeView", 18), setToNativeView);
    appendNative(m_idCreator->genIdentByStr("BY_InstanceOfClass", 18), instanceOfClass);
    appendNative(m_idCreator->genIdentByStr("BY_CreateViewGroup", 18), createViewGroup);
    appendNative(m_idCreator->genIdentByStr("BY_SetImageUrl", 14), setImageUrl);
    appendNative(m_idCreator->genIdentByStr("BY_ViewCommit", 13), viewCommit);
    appendNative(m_idCreator->genIdentByStr("BY_SetViewVisible", 17), setViewVisible);
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
