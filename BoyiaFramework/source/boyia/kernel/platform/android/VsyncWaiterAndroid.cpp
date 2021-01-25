#include "VsyncWaiterAndroid.h"
#include "BoyiaPtr.h"
#include "JNIUtil.h"
#include "WeakPtr.h"

namespace yanbo {
const char* kVsyncWaiterClass = "com/boyia/app/core/BoyiaSync";

LVoid VsyncWaiterAndroid::awaitVSync()
{
    WeakPtr<VsyncWaiter>* weakThis = new WeakPtr<VsyncWaiter>(BoyiaPtr<VsyncWaiter>(this));
    JNIUtil::callStaticVoidMethod(kVsyncWaiterClass, "awaitSync", "(J)V", (jlong)weakThis);
}

VsyncWaiter* VsyncWaiter::createVsyncWaiter()
{
    return new VsyncWaiterAndroid();
}
}
