#include "ImageAndroid.h"
#include "ImageView.h"
#include "JNIUtil.h"
#include "LColorUtil.h"
//#include "TextureCache.h"
#include "AppManager.h"
#include "SalLog.h"
#include <android/bitmap.h>

namespace util {

struct JBitmapAndroid {
    jweak m_obj;
    jmethodID m_loadImage;
    jmethodID m_getBitmap;
    jmethodID m_drawText;
    jmethodID m_recycle;
    AutoJObject object(JNIEnv* env)
    {
        return getRealObject(env, m_obj);
    }
};

ImageAndroid::ImageAndroid()
    : m_privateBitmap(kBoyiaNull)
    , m_image(kBoyiaNull)
    , m_pixels(kBoyiaNull)
{
    KLOG("BitmapAndroid::BitmapAndroid()");
    //JNIEnv* env = getJNIEnv();
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("BitmapAndroid::BitmapAndroid()0");
    //jclass clazz = env->FindClass("com/mini/app/core/BoyiaBitmap");
    jclass clazz = yanbo::JNIUtil::getJavaClassID("com/boyia/app/core/BoyiaImage");

    KLOG("BitmapAndroid::BitmapAndroid()1");
    jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
        "()V");

    KLOG("BitmapAndroid::BitmapAndroid()5");
    jobject obj = env->NewObject(clazz, constructMethod);

    init(env, clazz, obj);
}

void ImageAndroid::init(JNIEnv* env, jclass clazz, jobject obj)
{
    KLOG("BitmapAndroid::init");
    if (!m_privateBitmap) {
        m_privateBitmap = new JBitmapAndroid;
    }

    KLOG("BitmapAndroid::init1");
    //jclass clazz = env->GetObjectClass(obj);
    m_privateBitmap->m_obj = env->NewGlobalRef(obj);
    m_privateBitmap->m_loadImage = GetJMethod(env, clazz, "loadImage", "(Ljava/lang/String;JII)V");
    m_privateBitmap->m_getBitmap = GetJMethod(env, clazz, "getBitmap", "()Landroid/graphics/Bitmap;");
    m_privateBitmap->m_drawText = GetJMethod(env, clazz, "drawText", "(Ljava/lang/String;IIIII)V");
    m_privateBitmap->m_recycle = GetJMethod(env, clazz, "recycle", "()V");

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(obj);
}

ImageAndroid::~ImageAndroid()
{
    if (m_privateBitmap) {
        JNIEnv* env = yanbo::JNIUtil::getEnv();
        env->CallVoidMethod(
            m_privateBitmap->m_obj,
            m_privateBitmap->m_recycle);

        env->DeleteGlobalRef(m_privateBitmap->m_obj);
        delete m_privateBitmap;
    }
}

LVoid ImageAndroid::load(const String& path, LVoid* image)
{
    KLOG("BitmapAndroid::load");
    m_image = (yanbo::HtmlView*)image;
    yanbo::ImageView* item = (yanbo::ImageView*)m_image;

    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateBitmap->object(env);
    if (!javaObject.get())
        return;

    //yanbo::UIThreadClientMap::instance()->registerClient(this);
    // 不同的父类引用子类，指针地址值不一样
    // 多重继承下，子类指针转换成父类指针时，指针值会变化
    //ImageClient* client = static_cast<ImageClient*>(this);
    //KFORMATLOG("ImageAndroid::load this=%ld", (long)client);
    jstring strPath = strToJstring(env, path);
    KFORMATLOG("Image::load path=%s imageItem width=%d height=%d", (const char*)path.GetBuffer(), item->getWidth(), item->getHeight());
    env->CallVoidMethod(javaObject.get(), m_privateBitmap->m_loadImage,
        strPath, (jlong)getClientId(), item->getWidth(), item->getHeight());

    env->DeleteLocalRef(strPath);
}

LImage* LImage::create(LVoid* item)
{
    ImageAndroid* image = new ImageAndroid();
    image->setItem(static_cast<yanbo::HtmlView*>(item));
    return image;
}

AutoJObject ImageAndroid::getJavaBitmap() const
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    return m_privateBitmap->object(env);
}

AutoJObject ImageAndroid::getBitmapObject() const
{
    KLOG("BitmapAndroid::getBitmapObject begin");
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateBitmap->object(env);
    KLOG("BitmapAndroid::getBitmapObject end1");
    jobject jobj = env->CallObjectMethod(javaObject.get(), m_privateBitmap->m_getBitmap);
    KLOG("BitmapAndroid::getBitmapObject end");
    AutoJObject object = getRealObject(env, jobj);
    env->DeleteLocalRef(jobj);
    return object;
}

void ImageAndroid::drawText(const String& text,
    const LRect& rect,
    const LFont& font,
    const LColor& penColor)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateBitmap->object(env);
    if (!javaObject.get())
        return;

    LUint value = LColorUtil::rgb(penColor.m_red, penColor.m_green, penColor.m_blue, penColor.m_alpha);

    KFORMATLOG("BitmapAndroid drawText color=%x text=%s", value, (const char*)text.GetBuffer());
    KFORMATLOG("BitmapAndroid drawText text=%s width=%d height=%d", (const char*)text.GetBuffer(), rect.GetWidth(), rect.GetHeight());
    jstring strText = strToJstring(env, text);
    env->CallVoidMethod(
        javaObject.get(),
        m_privateBitmap->m_drawText,
        strText,
        rect.GetWidth(),
        rect.GetHeight(),
        font.getFontSize(),
        //(jint)LColorUtil::rgb(penColor.m_red, penColor.m_green, penColor.m_blue, penColor.m_alpha),
        (jint)value,
        (jint)font.getFontStyle());
    env->DeleteLocalRef(strText);
}

// setLoaded在uithread线程中执行
LVoid ImageAndroid::setLoaded(LBool loaded)
{
    LImage::setLoaded(loaded);
    if (m_image && loaded) {
        unlockPixels();
        //yanbo::AppManager::instance()->getLoader()->repaint(m_image);
        yanbo::AppManager::instance()->uiThread()->drawUI(m_image);
    }
}

LVoid* ImageAndroid::item() const
{
    return m_image;
}

LVoid ImageAndroid::setItem(yanbo::HtmlView* item)
{
    m_image = item;
}

LVoid ImageAndroid::unlockPixels()
{
    util::AutoJObject javaBitmap = getBitmapObject();
    if (!javaBitmap.get()) {
        return;
    }

    AndroidBitmapInfo info;
    memset(&info, 0, sizeof(info));
    AndroidBitmap_getInfo(javaBitmap.env(), javaBitmap.get(), &info);

    if (info.width <= 0 || info.height <= 0 || (info.format != ANDROID_BITMAP_FORMAT_RGB_565 && info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)) {
        return;
    }

    int res = AndroidBitmap_lockPixels(javaBitmap.env(), javaBitmap.get(), &m_pixels);
    //	if (pixels == NULL)
    //	{
    //		return NULL;
    //	}

    m_width = info.width;
    m_height = info.height;

    AndroidBitmap_unlockPixels(javaBitmap.env(), javaBitmap.get());
}

const String& ImageAndroid::url() const
{
    //yanbo::ImageView* item = (yanbo::ImageView*) m_image;
    //return item->url();
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

LVoid* ImageAndroid::pixels() const
{
    return m_pixels;
}

LVoid ImageAndroid::onClientCallback()
{
    setLoaded(LTrue);
}
}
