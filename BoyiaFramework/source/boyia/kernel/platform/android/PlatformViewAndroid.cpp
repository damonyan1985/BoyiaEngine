#include "AutoObject.h"
#include "JNIUtil.h"
#include "PixelRatio.h"
#include "PlatformView.h"
#include "TextureCache.h"

namespace yanbo {
class ViewInfo {
public:
    ViewInfo(const String& id, const String& type)
        : viewId(id)
        , viewType(type)
        , isInit(LFalse)
    {
    }

    LVoid initView(LInt width, LInt height)
    {
        texture = new Texture();
        texture->initExternal(width, height);

        JNIEnv* env = JNIUtil::getEnv();

        jstring id = util::strToJstring(JNIUtil::getEnv(), viewId);
        jstring type = util::strToJstring(JNIUtil::getEnv(), viewType);
        JNIUtil::callStaticVoidMethod(
            "com/boyia/app/core/view/PlatformViewNative",
            "createPlatformView",
            "(Ljava/lang/String;Ljava/lang/String;III)V",
            id, type,
            (jint)(width * PixelRatio::ratio()), // 屏幕真实宽度
            (jint)(height * PixelRatio::ratio()), // 屏幕真实高度
            // width,
            // height,
            texture->texId);

        isInit = LTrue;

        env->DeleteLocalRef(id);
        env->DeleteLocalRef(type);
    }

    String viewId;
    String viewType;
    BoyiaPtr<Texture> texture;
    LBool isInit;
};

PlatformView::PlatformView(
    const String& id,
    LBool selectable,
    const String& viewType)
    : BlockView(id, selectable)
    , m_viewInfo(new ViewInfo(id, viewType))
{
}

PlatformView::~PlatformView()
{
    if (m_viewInfo) {
        delete m_viewInfo;
    }
}

void PlatformView::layout()
{
    BlockView::layout();
}

void PlatformView::paint(LGraphicsContext& gc)
{
    BlockView::paint(gc);

    gc.drawPlatform(LRect(m_x, m_y, getWidth(), getHeight()), this);
    if (!m_viewInfo->isInit) {
        m_viewInfo->initView(getWidth(), getHeight());
    }
}

Texture* PlatformView::texture()
{
    return m_viewInfo->texture.get();
}

}