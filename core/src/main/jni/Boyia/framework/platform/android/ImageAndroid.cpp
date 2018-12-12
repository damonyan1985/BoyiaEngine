#include "ImageAndroid.h"
#include "AutoObject.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "LColor.h"
#include "LGdi.h"
#include "ImageView.h"
#include "UIView.h"
#include "JNIUtil.h"
#include "MiniTextureCache.h"
#include <android/bitmap.h>

namespace util
{

struct JBitmapAndroid
{
	jweak     m_obj;
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
    : m_privateBitmap(NULL)
    , m_image(NULL)
    , m_pixels(NULL)
{
	KLOG("BitmapAndroid::BitmapAndroid()");
	//JNIEnv* env = getJNIEnv();
	JNIEnv* env = yanbo::JNIUtil::getEnv();
	KLOG("BitmapAndroid::BitmapAndroid()0");
	//jclass clazz = env->FindClass("com/mini/app/core/BoyiaBitmap");
	jclass clazz = yanbo::JNIUtil::getJavaClassID("com/boyia/app/core/BoyiaBitmap");

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
    if (!m_privateBitmap)
    {
    	m_privateBitmap = new JBitmapAndroid;
    }

    KLOG("BitmapAndroid::init1");
	//jclass clazz = env->GetObjectClass(obj);
	m_privateBitmap->m_obj = env->NewGlobalRef(obj);
	m_privateBitmap->m_loadImage = GetJMethod(env, clazz, "loadImage", "(Ljava/lang/String;JII)V");
	m_privateBitmap->m_getBitmap = GetJMethod(env, clazz, "getBitmap", "()Landroid/graphics/Bitmap;");
	m_privateBitmap->m_drawText  = GetJMethod(env, clazz, "drawText", "(Ljava/lang/String;IIIIIII)V");
	m_privateBitmap->m_recycle  = GetJMethod(env, clazz, "recycle", "()V");

	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(obj);
}

ImageAndroid::~ImageAndroid()
{
	if (m_privateBitmap != NULL)
	{
		JNIEnv* env = yanbo::JNIUtil::getEnv();
		env->CallVoidMethod(
				m_privateBitmap->m_obj,
				m_privateBitmap->m_recycle
		);

		env->DeleteGlobalRef(m_privateBitmap->m_obj);
		delete m_privateBitmap;
	}

	m_privateBitmap = NULL;
}



LVoid ImageAndroid::load(const String& path, LVoid* image)
{
	KLOG("BitmapAndroid::load");
	m_image = (yanbo::HtmlView*) image;
	yanbo::ImageView* item = (yanbo::ImageView*) m_image;

	JNIEnv* env = yanbo::JNIUtil::getEnv();
	AutoJObject javaObject = m_privateBitmap->object(env);
    if (!javaObject.get())
        return;

    jstring strPath = strToJstring(env, (const char*)path.GetBuffer());
    KFORMATLOG("Image::load path=%s imageItem width=%d height=%d", (const char*)path.GetBuffer(), item->getWidth(), item->getHeight());
    env->CallVoidMethod(javaObject.get(), m_privateBitmap->m_loadImage,
    		strPath, (jlong)this, item->getWidth(), item->getHeight());

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
		LGraphicsContext::TextAlign align,
		const LFont& font,
		const LRgb& penColor,
		const LRgb& brushColor)
{
	JNIEnv* env = yanbo::JNIUtil::getEnv();
	AutoJObject javaObject = m_privateBitmap->object(env);
    if (!javaObject.get())
        return;

	LUint value = LColor::rgb(penColor.m_red, penColor.m_green, penColor.m_blue, penColor.m_alpha);

	KFORMATLOG("BitmapAndroid drawText color=%x text=%s", value, (const char*)text.GetBuffer());
	KFORMATLOG("BitmapAndroid drawText text=%s width=%d height=%d", (const char*)text.GetBuffer(), rect.GetWidth(), rect.GetHeight());
	jstring strText = strToJstring(env, (const char*)text.GetBuffer());
    env->CallVoidMethod(
    		javaObject.get(),
    		m_privateBitmap->m_drawText,
    		strText,
    		rect.GetWidth(),
    		rect.GetHeight(),
    		(jint)align,
    		font.getFontSize(),
    		(jint)LColor::rgb(penColor.m_red, penColor.m_green, penColor.m_blue, penColor.m_alpha),
    		(jint)font.getFontStyle(),
    		(jint)LColor::rgb(brushColor.m_red, brushColor.m_green, brushColor.m_blue, brushColor.m_alpha)
    );
    env->DeleteLocalRef(strText);
}

LVoid ImageAndroid::setLoaded(LBool loaded)
{
	LImage::setLoaded(loaded);
	if (m_image && loaded)
	{
		unlockPixels();
		yanbo::UIView::getInstance()->getLoader()->repaint(m_image);
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
	if (!javaBitmap.get())
	{
		 return;
	}

	AndroidBitmapInfo info;
	memset(&info, 0, sizeof(info));
	AndroidBitmap_getInfo(javaBitmap.env(), javaBitmap.get(), &info);

	if (info.width <= 0 || info.height <= 0 ||
			(info.format != ANDROID_BITMAP_FORMAT_RGB_565
				&& info.format != ANDROID_BITMAP_FORMAT_RGBA_8888))
	{
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

}
