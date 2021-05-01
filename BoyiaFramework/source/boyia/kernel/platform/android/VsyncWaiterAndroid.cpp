#include "VsyncWaiterAndroid.h"
#include "BoyiaPtr.h"
#include "JNIUtil.h"
#include "WeakPtr.h"

namespace yanbo {
const char* kVsyncWaiterClass = "com/boyia/app/core/BoyiaSync";

LVoid VsyncWaiterAndroid::awaitVSync()
{
    JNIUtil::callStaticVoidMethod(kVsyncWaiterClass, "awaitSync", "()V");
}

VsyncWaiter* VsyncWaiter::createVsyncWaiter()
{
    return new VsyncWaiterAndroid();
}
}
